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

#include "android/cpp/c/backend_c.h"
#include "android/cpp/c/type.h"
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
#elif __APPLE__
#include "tflite_settings_apple.h"
#elif defined(_WIN64) || defined(_WIN32)
// We don't have specialized settings for windows
// but settings for Apple work on Windows orders of magnitude faster than
// settings for Android. Apparently, uint8 operations require some special
// emulation that slows down the benchmark.
#include "tflite_settings_apple.h"
#else
#include "tflite_settings_android.h"
#endif

#include "thread_pool.h"
#include "utils.h"

#if __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#include "tensorflow/lite/delegates/coreml/coreml_delegate.h"
#include "tensorflow/lite/delegates/gpu/metal_delegate.h"
#endif
#endif

struct TFLiteBackendData {
  const char *name = "TFLite";
  const char *vendor = "Google";
  TfLiteModel *model{nullptr};
  std::vector<TfLiteInterpreterOptions *> options{};
  std::vector<TfLiteInterpreter *> interpreter{};
  int32_t shards_num = 1;
  uint32_t real_batch_size = 1;
  std::unique_ptr<Threadpool> executer;
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
      printf("TfLiteType %d not supported\n", type);
      return mlperf_data_t::Float32;
  }
}

size_t TFLiteNumElements(const TfLiteTensor *tensor) {
  size_t result = 1;
  for (int i = 0; i < TfLiteTensorNumDims(tensor); ++i) {
    result *= TfLiteTensorDim(tensor, i);
  }
  return result;
}

#if defined(__ANDROID__) && defined(MTK_TFLITE_NEURON_BACKEND)
static bool neuron_tflite_backend(const char **not_allowed_message,
                                  const mlperf_device_info_t *device_info) {
  bool neuron_capable = false;

  bool neuron_adapter = false;
  void *libneuron_adapter;
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
      char ro_device_family[PROP_VALUE_MAX + 1];
      __system_property_get("ro.build.device_family", ro_device_family);
      if (strcmp(ro_device_family, "OPMT6885") &&
          atoi(ro_system_build_version_sdk) >= 30) {
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
bool mlperf_backend_matches_hardware(const char **not_allowed_message,
                                     const char **settings,
                                     const mlperf_device_info_t *device_info) {
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
    char *ptr;
    ptr = strstr(ro_build_characteristics, "emulator");
    if (ptr) return true;
  }
  return false;
}
#endif

// Create a new backend and return the pointer to it.
mlperf_backend_ptr_t mlperf_backend_create(
    const char *model_path, mlperf_backend_configuration_t *configs,
    const char *native_lib_path) {
  // Verify only one instance of the backend exists at any time
  if (backendExists) {
    printf("Error: Only one backend instance should exist at a time\n");
    return nullptr;
  }

  TFLiteBackendData *backend_data = new TFLiteBackendData();

  backendExists = true;

  // Load the model.
  backend_data->model = TfLiteModelCreateFromFile(model_path);
  if (!backend_data->model) {
    printf("Failed to load model: %s", model_path);
    mlperf_backend_delete(backend_data);
    return nullptr;
  }

  if (configs->batch_size > 1) {
    // If we use batching, make shards_num 2
    //   if it is not specified in settings.
    // If we don't use batching, we will use shards_num=1,
    //   which is the default value.
    backend_data->shards_num = 2;

    // TODO convert this to a member var
    for (int i = 0; i < configs->count; ++i) {
      if (strcmp(configs->keys[i], "shards_num") == 0) {
        backend_data->shards_num = atoi(configs->values[i]);
      }
    }

    if ((configs->batch_size % backend_data->shards_num) != 0) {
      printf("Batch size is not dividable by shards_num: %d %% %d != 0\n",
             configs->batch_size, backend_data->shards_num);
      mlperf_backend_delete(backend_data);
      return nullptr;
    }

    backend_data->real_batch_size =
        configs->batch_size / backend_data->shards_num;
  }

  backend_data->executer =
      std::unique_ptr<Threadpool>(new Threadpool(backend_data->shards_num));

  // Create interpreter options function.
  auto create_option = [&](TfLiteInterpreterOptions *&option_ptr) -> void {
    option_ptr = TfLiteInterpreterOptionsCreate();
    TfLiteDelegate *delegate = nullptr;

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
        options.inference_priority1 = TFLITE_GPU_INFERENCE_PRIORITY_MIN_LATENCY;
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
      //
      // Here we use the batch size of the task to check if this
      // is for offline/batch scenario, if batch size is not
      // larger than 1, assume it's offline and use NNAPI delegate
      // to handle it.
      //
      // When batch_size > 1 and divisible by shards_num, we have
      //   backend_data->real_batch_size =
      //     configs->batch_size / backend_data->shards_num;
      // which is not what we want. E.g., for shards_num = 2 and
      // batch_size = 2, then real_batch_size = 1. We still want to
      // use NNAPI delegate for this case.
      if (configs->batch_size > 1) {
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
#endif  // MTK_TFLITE_NEURON_BACKEND
    }
#endif  // __ANDROID__
#if TARGET_OS_SIMULATOR
#elif TARGET_OS_IPHONE
    if (strcmp(configs->accelerator, "metal") == 0) {
      TFLGpuDelegateOptions opts{
          .allow_precision_loss = false,
          .wait_type = TFLGpuDelegateWaitType::TFLGpuDelegateWaitTypePassive,
          .enable_quantization = true,
      };
      delegate = TFLGpuDelegateCreate(&opts);
      std::cout << "Enabling Metal delegate " << delegate << "\n";
    } else if (strcmp(configs->accelerator, "coreml") == 0) {
      TfLiteCoreMlDelegateOptions opts{
          .enabled_devices = TfLiteCoreMlDelegateAllDevices,
          .coreml_version = 3,
          .max_delegated_partitions = 0,
          .min_nodes_per_partition = 2,
      };
      delegate = TfLiteCoreMlDelegateCreate(&opts);
      std::cout << "Enabling CoreML delegate " << delegate << "\n";
    }
#endif
    if (delegate != nullptr) {
      TfLiteInterpreterOptionsAddDelegate(option_ptr, delegate);
    }
  };

  backend_data->options.resize(backend_data->shards_num);
  backend_data->interpreter.resize(backend_data->shards_num);

  for (int k = 0; k < backend_data->shards_num; k++) {
    create_option(backend_data->options[k]);

    backend_data->interpreter[k] =
        TfLiteInterpreterCreate(backend_data->model, backend_data->options[k]);
    if (!backend_data->interpreter[k]) {
      printf("Fallback to a vanilla interpreter\n");
      backend_data->interpreter[k] = TfLiteInterpreterCreate(
          backend_data->model, TfLiteInterpreterOptionsCreate());
      if (!backend_data->interpreter[k]) {
        printf("Failed to create the interpreter\n");
        mlperf_backend_delete(backend_data);
        return nullptr;
      }
    }
  }

  const int32_t input_tensor_count =
      TfLiteInterpreterGetInputTensorCount(backend_data->interpreter[0]);

  for (int shard_index = 0; shard_index < backend_data->shards_num;
       shard_index++) {
    TfLiteInterpreter *&shard = backend_data->interpreter[shard_index];

    for (int input_index = 0; input_index < input_tensor_count; input_index++) {
      TfLiteTensor *tensor =
          TfLiteInterpreterGetInputTensor(shard, input_index);

      backend_data->original_tensor_size = tensor->bytes;

      if (backend_data->real_batch_size != tensor->dims->data[0]) {
        std::vector<int32_t> dims;
        dims.resize(tensor->dims->size);
        dims[0] = backend_data->real_batch_size;
        for (int i = 1; i < tensor->dims->size; i++) {
          dims[i] = tensor->dims->data[i];
        }
        if (TfLiteInterpreterResizeInputTensor(shard, input_index, dims.data(),
                                               tensor->dims->size) !=
            kTfLiteOk) {
          printf("Failed to resize input\n");
          mlperf_backend_delete(backend_data);
          return nullptr;
        }
      }
    }

    if (TfLiteInterpreterAllocateTensors(shard) != kTfLiteOk) {
      printf("Failed to allocate tensors\n");
      mlperf_backend_delete(backend_data);
      return nullptr;
    }
  }

  return backend_data;
}

// Vendor name who create this backend.
const char *mlperf_backend_vendor_name(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
  return backend_data->vendor;
}

// Return the name of this backend.
const char *mlperf_backend_name(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
  return backend_data->name;
}

// Destroy the backend pointer and its data.
void mlperf_backend_delete(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
  TfLiteModelDelete(backend_data->model);
  for (int i = 0; i < backend_data->shards_num; i++) {
    TfLiteInterpreterOptionsDelete(backend_data->options[i]);
    TfLiteInterpreterDelete(backend_data->interpreter[i]);
  }
  delete backend_data;
  backendExists = false;
}

// Run the inference for a sample.
mlperf_status_t mlperf_backend_issue_query(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
  auto task = [&backend_data](int index) -> TfLiteStatus {
    return TfLiteInterpreterInvoke(backend_data->interpreter[index]);
  };

  std::vector<std::future<TfLiteStatus>> f;
  f.resize(backend_data->shards_num);
  // dispatch workers for shards
  for (int k = 1; k < backend_data->shards_num; k++) {
    f[k] = backend_data->executer->submit(task, k);
  }
  // main thread for the first shard
  if (task(0) != kTfLiteOk) {
    printf("Failed to run the inference\n");
    return MLPERF_FAILURE;
  }
  // sync and get result of workers
  for (int k = 1; k < backend_data->shards_num; k++) {
    if (f[k].get() != kTfLiteOk) {
      printf("Failed to run the inference\n");
      return MLPERF_FAILURE;
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
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
  return TfLiteInterpreterGetInputTensorCount(backend_data->interpreter[0]);
}

// Return the type of the ith input.
mlperf_data_t mlperf_backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                            int32_t i) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
  const TfLiteTensor *tensor =
      TfLiteInterpreterGetInputTensor(backend_data->interpreter[0], i);
  mlperf_data_t type;
  type.type = TfType2Type(TfLiteTensorType(tensor));
  type.size = TFLiteNumElements(tensor);
  type.size /= backend_data->real_batch_size;
  return type;
}

// Set the data for ith input.
mlperf_status_t mlperf_backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                         int32_t batch_index, int32_t i,
                                         void *data) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
#if defined(MTK_TFLITE_NEURON_BACKEND) && defined(__ANDROID__)
  if (use_gpu)
    perf_handle =
        acquirePerformanceLock(perf_handle, FAST_SINGLE_ANSWER_MODE, 2000);
#endif
  const int shard_index = batch_index / backend_data->real_batch_size;
  TfLiteTensor *tensor = TfLiteInterpreterGetInputTensor(
      backend_data->interpreter[shard_index], i);
  const int data_offset = backend_data->original_tensor_size *
                          (batch_index % backend_data->real_batch_size);
  memcpy(tensor->data.raw + data_offset, data,
         backend_data->original_tensor_size);

  return MLPERF_SUCCESS;
}

// Return the number of outputs for the model.
int32_t mlperf_backend_get_output_count(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
  return TfLiteInterpreterGetOutputTensorCount(backend_data->interpreter[0]);
}

// Return the type of ith output.
mlperf_data_t mlperf_backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                             int32_t i) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
  const TfLiteTensor *tensor =
      TfLiteInterpreterGetOutputTensor(backend_data->interpreter[0], i);
  mlperf_data_t type;
  type.type = TfType2Type(TfLiteTensorType(tensor));
  type.size = TFLiteNumElements(tensor);
  type.size /= backend_data->real_batch_size;
  return type;
}

// Get the data from ith output.
mlperf_status_t mlperf_backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                          uint32_t batch_index, int32_t i,
                                          void **data) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
  const int shard_index = batch_index / backend_data->real_batch_size;

  const TfLiteTensor *output_tensor = TfLiteInterpreterGetOutputTensor(
      backend_data->interpreter[shard_index], i);
  batch_index %= backend_data->real_batch_size;

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
      printf("Data type not yet supported\n");
      return MLPERF_FAILURE;
  }
  return MLPERF_SUCCESS;
}
