/* Copyright 2020-2021 The MLPerf Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(MTK_TFLITE_NEURON_BACKEND) && defined(__ANDROID__)
#include <dlfcn.h>

#include "neuron/APUWareUtilsApi.h"
#endif

#include "cpp/c/backend_c.h"
#include "cpp/c/type.h"
#include "tensorflow/lite/c/c_api.h"
#include "tensorflow/lite/c/common.h"
#if __ANDROID__
#include <sys/system_properties.h>

#if MTK_TFLITE_NEURON_BACKEND
#include "neuron/neuron_delegate.h"
#endif
#include "tensorflow/lite/delegates/gpu/delegate.h"
#include "tensorflow/lite/delegates/nnapi/nnapi_delegate.h"
#endif
#include "tensorflow/core/platform/logging.h"
#if MTK_TFLITE_NEURON_BACKEND
#include "neuron/tflite_settings_mtk.h"
#else
#include "tflite_settings.h"
#endif
#include "thread_pool.h"
#include "utils.h"

static const int32_t shard_num = 2;

struct TFLiteBackendData {
  const char* name = "TFLite";
  const char* vendor = "Google";
  TfLiteModel* model{nullptr};
  TfLiteInterpreterOptions* options[shard_num] = {};
  TfLiteInterpreter* interpreter[shard_num] = {};
  uint32_t batch_size = 1;
  int32_t input_tensor_count;
  void** acc_data[shard_num] = {};
  std::unique_ptr<Threadpool> executer;
  bool use_batches = false;
  int32_t original_tensor_size = 0;
};

static bool backendExists = false;

#if defined(MTK_TFLITE_NEURON_BACKEND) && defined(__ANDROID__)
static int perf_handle = 0;
static bool use_gpu = false;
#endif

inline mlperf_data_t::Type TfType2Type(TfLiteType type) {
  switch (type) {
    case kTfLiteFloat32:
      return mlperf_data_t::Float32;
    case kTfLiteUInt8:
      return mlperf_data_t::Uint8;
    case kTfLiteInt8:
      return mlperf_data_t::Int8;
    case kTfLiteFloat16:
      return mlperf_data_t::Float16;
    case kTfLiteInt32:
      return mlperf_data_t::Int32;
    case kTfLiteInt64:
      return mlperf_data_t::Int64;
    default:
      printf("TfLiteType %d not supported", type);
      return mlperf_data_t::Float32;
  }
}

size_t TFLiteNumElements(const TfLiteTensor* tensor) {
  size_t result = 1;
  for (int i = 0; i < TfLiteTensorNumDims(tensor); ++i) {
    result *= TfLiteTensorDim(tensor, i);
  }
  return result;
}

#if defined(__ANDROID__) && defined(MTK_TFLITE_NEURON_BACKEND)
static bool neuron_tflite_backend(const char** not_allowed_message,
                                  const mlperf_device_info_t* device_info) {
  bool neuron_capable = false;

  bool neuron_adapter = false;
  void* libneuron_adapter;
  libneuron_adapter =
      dlopen("libneuron_adapter.mtk.so", RTLD_LAZY | RTLD_LOCAL);
  if (libneuron_adapter == nullptr) {
    // Try to dlopen Neuron universal SDK
    libneuron_adapter =
        dlopen("libneuronusdk_adapter.mtk.so", RTLD_LAZY | RTLD_LOCAL);
    if (libneuron_adapter != nullptr) neuron_adapter = true;
  } else {
    neuron_adapter = true;
  }

  if (neuron_adapter) {
    char ro_system_build_version_sdk[PROP_VALUE_MAX + 1];
    if (__system_property_get("ro.system.build.version.sdk",
                              ro_system_build_version_sdk)) {
      if (atoi(ro_system_build_version_sdk) >= 30) {
        neuron_capable = true;
      }
    }
  }

  if (neuron_capable) {
    LOG(INFO) << "mtk_neuron_backend backend matches hardware";
  } else {
    LOG(INFO)
        << "mtk_neuron_backend backend, Soc Not supported. Trying next backend";
  }
  return neuron_capable;
}
#endif

// TFLite is the standard backend for all hardwares.
bool mlperf_backend_matches_hardware(const char** not_allowed_message,
                                     const char** settings,
                                     const mlperf_device_info_t* device_info) {
  *not_allowed_message = nullptr;
  *settings = tflite_settings.c_str();
#if MTK_TFLITE_NEURON_BACKEND && defined(__ANDROID__)
  return neuron_tflite_backend(not_allowed_message, device_info);
#else
  LOG(INFO) << "TFLite backend matches hardware";
  return true;
#endif
}

#if __ANDROID__
bool is_emulator() {
  char ro_build_characteristics[PROP_VALUE_MAX + 1];
  if (__system_property_get("ro.build.characteristics",
                            ro_build_characteristics)) {
    char* ptr;
    ptr = strstr(ro_build_characteristics, "emulator");
    if (ptr) return true;
  }
  return false;
}
#endif

// Create a new backend and return the pointer to it.
mlperf_backend_ptr_t mlperf_backend_create(
    const char* model_path, mlperf_backend_configuration_t* configs,
    const char* native_lib_path) {
  // Verify only one instance of the backend exists at any time
  if (backendExists) {
    printf("Error: Only one backend instance should exist at a time");
    return nullptr;
  }

  TFLiteBackendData* backend_data = new TFLiteBackendData();

  backendExists = true;

  // Load the model.
  backend_data->model = TfLiteModelCreateFromFile(model_path);
  if (!backend_data->model) {
    printf("Failed to load model: %s", model_path);
    mlperf_backend_delete(backend_data);
    return nullptr;
  }

  backend_data->executer =
      std::unique_ptr<Threadpool>(new Threadpool(shard_num));

  // Create interpreter options function.
  auto create_option = [&](TfLiteInterpreterOptions*& option_ptr) -> void {
    option_ptr = TfLiteInterpreterOptionsCreate();
    TfLiteDelegate* delegate = nullptr;
    if (configs->batch_size > 1) {
      backend_data->use_batches = true;
      backend_data->batch_size = configs->batch_size;
    }

    // TODO convert this to a member var
    for (int i = 0; i < configs->count; ++i) {
      if (strcmp(configs->keys[i], "num_threads") == 0) {
        TfLiteInterpreterOptionsSetNumThreads(option_ptr,
                                              atoi(configs->values[i]));
      }
    }

#if __ANDROID__
    if (!is_emulator() && ((strcmp(configs->accelerator, "gpu_f16") == 0) ||
                           (strcmp(configs->accelerator, "gpu") == 0))) {
      auto options = TfLiteGpuDelegateOptionsV2Default();
      if (strcmp(configs->accelerator, "gpu_f16") == 0)
        options.inference_priority1 =
            TFLITE_GPU_INFERENCE_PRIORITY_MIN_LATENCY;
      delegate = TfLiteGpuDelegateV2Create(&options);
#if MTK_TFLITE_NEURON_BACKEND
      use_gpu = true;
#endif
    } else if (strcmp(configs->accelerator, "nnapi") == 0) {
      auto options = tflite::StatefulNnApiDelegate::Options();
      options.allow_fp16 = true;
      options.disallow_nnapi_cpu = true;
      delegate = new tflite::StatefulNnApiDelegate(options);
#if MTK_TFLITE_NEURON_BACKEND
    } else if (strcmp(configs->accelerator, "neuron") == 0) {
      // The kOptimizationBatchProcessor doesn't work yet.
      // Use NNAPI instead.
      if (backend_data->use_batches == true) {
        auto options = tflite::StatefulNnApiDelegate::Options();
        options.allow_fp16 = true;
        options.disallow_nnapi_cpu = true;
        delegate = new tflite::StatefulNnApiDelegate(options);
      } else {
        auto options = TfLiteNeuronDelegateOptionsDefault();
        options.optimization_hint =
            kOptimizationLowLatency | kOptimizationDeepFusion;
        options.allow_fp16 = true;
        delegate = TfLiteNeuronDelegateCreate(&options);
      }
#endif // MTK_TFLITE_NEURON_BACKEND
    }

    if (delegate != nullptr) {
      TfLiteInterpreterOptionsAddDelegate(option_ptr, delegate);
    }
#endif // __ANDROID__
  };

  const int dispatch_max = shard_num;
  for (int k = 0; k < dispatch_max; k++) {
    // Create Backend Option
    create_option(backend_data->options[k]);

    // Create the interpreter.
    backend_data->interpreter[k] =
        TfLiteInterpreterCreate(backend_data->model, backend_data->options[k]);
    if (!backend_data->interpreter[k]) {
      // create a vanilla interpreter
      backend_data->interpreter[k] = TfLiteInterpreterCreate(
          backend_data->model, TfLiteInterpreterOptionsCreate());
      if (!backend_data->interpreter[k]) {
        printf("Failed to create the interpreter");
        mlperf_backend_delete(backend_data);
        return nullptr;
      }
    }
  }

  backend_data->input_tensor_count =
      TfLiteInterpreterGetInputTensorCount(backend_data->interpreter[0]);

  for (int i = 0; i < dispatch_max; i++) {
    backend_data->acc_data[i] =
        (void**)malloc(sizeof(void*) * backend_data->input_tensor_count);
  }
  return backend_data;
}

// Vendor name who create this backend.
const char* mlperf_backend_vendor_name(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  return backend_data->vendor;
}

// Return the name of this backend.
const char* mlperf_backend_name(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  return backend_data->name;
}

// Destroy the backend pointer and its data.
void mlperf_backend_delete(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  const int dispatch_max = shard_num;
  for (int i = 0; i < dispatch_max; i++) free(backend_data->acc_data[i]);
  TfLiteModelDelete(backend_data->model);
  for (int i = 0; i < dispatch_max; i++) {
    TfLiteInterpreterOptionsDelete(backend_data->options[i]);
    TfLiteInterpreterDelete(backend_data->interpreter[i]);
  }
  delete backend_data;
  backendExists = false;
}

// Run the inference for a sample.
mlperf_status_t mlperf_backend_issue_query(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  auto task = [&backend_data](int index) -> TfLiteStatus {
    return TfLiteInterpreterInvoke(backend_data->interpreter[index]);
  };

  const int dispatch_max = (backend_data->use_batches) ? shard_num : 1;
  std::future<TfLiteStatus> f[shard_num];
  // dispatch workers
  for (int k = 1; k < dispatch_max; k++)
    f[k] = backend_data->executer->submit(task, k);
  // main thread for batch_size == 1
  if (TfLiteInterpreterInvoke(backend_data->interpreter[0]) != kTfLiteOk) {
    printf("Failed to run the inference");
    return MLPERF_FAILURE;
  }
  // sync and get result of workers
  for (int k = 1; k < dispatch_max; k++) {
    if (f[k].get() != kTfLiteOk) {
      printf("Failed to run the inference");
      return MLPERF_FAILURE;
    }
  }
  for (int k = 0; k < dispatch_max; k++) {
    if (backend_data->acc_data[k] != nullptr) {
      for (int i = 0; i < backend_data->input_tensor_count; i++) {
        if (backend_data->acc_data[k][i] != nullptr) {
          free(backend_data->acc_data[k][i]);
        }
        backend_data->acc_data[k][i] = nullptr;
      }
    }
  }
  return MLPERF_SUCCESS;
}

// Flush the staged queries immediately.
mlperf_status_t mlperf_backend_flush_queries(mlperf_backend_ptr_t backend_ptr) {
  return MLPERF_SUCCESS;
}

// Return the number of inputs of the model.
int32_t mlperf_backend_get_input_count(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  return TfLiteInterpreterGetInputTensorCount(backend_data->interpreter[0]);
}

// Return the type of the ith input.
mlperf_data_t mlperf_backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                            int32_t i) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  const TfLiteTensor* tensor =
      TfLiteInterpreterGetInputTensor(backend_data->interpreter[0], i);
  mlperf_data_t type;
  type.type = TfType2Type(TfLiteTensorType(tensor));
  type.size = TFLiteNumElements(tensor);
  return type;
}
// Set the data for ith input.
mlperf_status_t mlperf_backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                         int32_t batch_index, int32_t i,
                                         void* data) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
#if defined(MTK_TFLITE_NEURON_BACKEND) && defined(__ANDROID__)
  if (use_gpu)
    perf_handle =
      acquirePerformanceLock(perf_handle, FAST_SINGLE_ANSWER_MODE, 2000);
#endif
  const int real_batch_size =
      (backend_data->use_batches) ? (backend_data->batch_size / shard_num) : 1;
  const int shard = batch_index / (real_batch_size);
  TfLiteTensor* tensor =
      TfLiteInterpreterGetInputTensor(backend_data->interpreter[shard], i);
  if (batch_index % real_batch_size == 0) {
    // Allocate tensors.
    if (TfLiteInterpreterAllocateTensors(backend_data->interpreter[shard]) !=
        kTfLiteOk) {
      printf("Failed to create the interpreter");
      mlperf_backend_delete(backend_data);
      return MLPERF_FAILURE;
    }

    auto new_tensor =
        TfLiteInterpreterGetInputTensor(backend_data->interpreter[shard], i);
    if (!(new_tensor == tensor)) tensor = new_tensor;
    tensor->data.raw = (char*)data;
    if (backend_data->original_tensor_size == 0)
      backend_data->original_tensor_size = tensor->bytes;

    backend_data->acc_data[shard][i] =
        (char*)malloc(backend_data->original_tensor_size);
  } else if (batch_index % real_batch_size == 1) {
    backend_data->acc_data[shard][i] =
        (char*)realloc(backend_data->acc_data[shard][i],
                       real_batch_size * backend_data->original_tensor_size);
  }
  memcpy(
      ((char*)backend_data->acc_data[shard][i] +
       ((batch_index % real_batch_size) * backend_data->original_tensor_size)),
      data, backend_data->original_tensor_size);

  if (batch_index % real_batch_size == (real_batch_size - 1)) {
    if (tensor->dims->data[0] != real_batch_size) {
      int32_t* dims = (int32_t*)malloc(sizeof(int32_t) * tensor->dims->size);
      dims[0] = real_batch_size;
      for (int i = 1; i < tensor->dims->size; i++) {
        dims[i] = tensor->dims->data[i];
      }
      TfLiteInterpreterResizeInputTensor(backend_data->interpreter[shard], i,
                                         dims, tensor->dims->size);
      free(dims);
    }
    tensor->data.raw = (char*)backend_data->acc_data[shard][i];

    // Allocate tensors.
  }
  if (batch_index == (backend_data->batch_size - 1)) {
    auto task = [&backend_data](int index) -> TfLiteStatus {
      return TfLiteInterpreterAllocateTensors(backend_data->interpreter[index]);
    };
    std::future<TfLiteStatus> f[shard_num];

    const int dispatch_max = (backend_data->use_batches) ? shard_num : 1;
    // dispatch workers
    for (int k = 1; k < dispatch_max; k++)
      f[k] = backend_data->executer->submit(task, k);
    // main thread for batch_size == 1
    if (TfLiteInterpreterAllocateTensors(backend_data->interpreter[0]) !=
        kTfLiteOk) {
      printf("Failed to run the inference");
      return MLPERF_FAILURE;
    }
    // sync and get result of workers
    for (int k = 1; k < dispatch_max; k++) {
      if (f[k].get() != kTfLiteOk) {
        printf("Failed to run the inference");
        return MLPERF_FAILURE;
      }
    }
  }
  return MLPERF_SUCCESS;
}

// Return the number of outputs for the model.
int32_t mlperf_backend_get_output_count(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  return TfLiteInterpreterGetOutputTensorCount(backend_data->interpreter[0]);
}

// Return the type of ith output.
mlperf_data_t mlperf_backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                             int32_t i) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  const TfLiteTensor* tensor =
      TfLiteInterpreterGetOutputTensor(backend_data->interpreter[0], i);
  mlperf_data_t type;
  type.type = TfType2Type(TfLiteTensorType(tensor));
  type.size = TFLiteNumElements(tensor);
  return type;
}

// Get the data from ith output.
mlperf_status_t mlperf_backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                          uint32_t batch_index, int32_t i,
                                          void** data) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  const int real_batch_size =
      (backend_data->use_batches) ? (backend_data->batch_size / shard_num) : 1;
  const int shard = batch_index / (real_batch_size);

  const TfLiteTensor* output_tensor =
      TfLiteInterpreterGetOutputTensor(backend_data->interpreter[shard], i);
  batch_index %= (real_batch_size);

  int non_batch_size = 1;
  for (int i = 1; i < output_tensor->dims->size; i++) {
    non_batch_size *= output_tensor->dims->data[i];
  }

  switch (output_tensor->type) {
    case kTfLiteFloat32:
      *data = (output_tensor->data.f + (batch_index * non_batch_size));
      break;
    case kTfLiteUInt8:
      *data = (output_tensor->data.uint8 + (batch_index * non_batch_size));
      break;
    case kTfLiteInt8:
      *data = (output_tensor->data.int8 + (batch_index * non_batch_size));
      break;
    case kTfLiteFloat16:
      *data = (output_tensor->data.f16 + (batch_index * non_batch_size));
      break;
    case kTfLiteInt32:
      *data = (output_tensor->data.i32 + (batch_index * non_batch_size));
      break;
    case kTfLiteInt64:
      *data = (output_tensor->data.i64 + (batch_index * non_batch_size));
      break;
    default:
      printf("Data type not yet supported");
      return MLPERF_FAILURE;
  }
  return MLPERF_SUCCESS;
}
