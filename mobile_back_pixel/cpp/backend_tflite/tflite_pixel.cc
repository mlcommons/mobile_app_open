/* Copyright 2021 The MLPerf Authors. All Rights Reserved.

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

#include "flutter/cpp/c/backend_c.h"
#include "flutter/cpp/c/type.h"
#include "tensorflow/lite/c/c_api.h"
#include "tensorflow/lite/c/c_api_experimental.h"
#include "tensorflow/lite/c/common.h"
#if __ANDROID__
#include <sys/system_properties.h>

#include "tensorflow/core/platform/logging.h"
#include "tensorflow/lite/delegates/gpu/delegate.h"
#include "tensorflow/lite/delegates/nnapi/nnapi_delegate.h"
#endif
#include "resize_argmax_op.h"
#include "tflite_settings_pixel.h"
#include "thread_pool.h"

#define N_OFFLINE_INTEPRETERS 8

struct TFLiteBackendData {
  const char* name = "TFLite";
  const char* vendor = "Google";
  TfLiteModel* model{nullptr};
  TfLiteInterpreterOptions* options[N_OFFLINE_INTEPRETERS] = {};
  TfLiteInterpreter* interpreter[N_OFFLINE_INTEPRETERS] = {};
  TfLiteInterpreter* interpreter8[N_OFFLINE_INTEPRETERS] = {};
  uint32_t batch_size = 64;
  int32_t input_tensor_count;
  void** acc_data[N_OFFLINE_INTEPRETERS] = {};
  std::unique_ptr<Threadpool> executer;
  bool use_shard = false;
  bool has_temp_data = false;
  int32_t original_tensor_size = 0;

  std::future<TfLiteStatus> status[N_OFFLINE_INTEPRETERS];
};

static bool backendExists = false;

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

// TFLite is the standard backend for all hardwares.
bool mlperf_backend_matches_hardware(const char** not_allowed_message,
                                     const char** settings,
                                     const mlperf_device_info_t* device_info) {
  *not_allowed_message = nullptr;
  *settings = tflite_settings.c_str();

  if (device_info && device_info->model && device_info->manufacturer) {
    LOG(INFO) << "Pixel HW supported check: model: " << device_info->model
              << ", manufacturer: " << device_info->manufacturer;

    if (strcmp(device_info->manufacturer, "Google") == 0 &&
        strstr(device_info->model, "Pixel 6") != NULL) {
      printf("Pixel backend matches hardware");
      return true;
    }
  }

  return false;
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
      std::unique_ptr<Threadpool>(new Threadpool(N_OFFLINE_INTEPRETERS));

  // Create interpreter options.
  // Create interpreter options function.
  auto create_option = [&](TfLiteInterpreterOptions*& option_ptr) -> void {
    option_ptr = TfLiteInterpreterOptionsCreate();
    TfLiteInterpreterOptionsAddCustomOp(option_ptr, "ResizeArgmax",
                                        Register_ResizeArgmax(), 1, 999);
    TfLiteDelegate* delegate = nullptr;

    for (int i = 0; i < configs->count; ++i) {
      if (strcmp(configs->keys[i], "num_threads") == 0) {
        TfLiteInterpreterOptionsSetNumThreads(option_ptr,
                                              atoi(configs->values[i]));
      }
#if __ANDROID__
      if (!is_emulator() && ((strcmp(configs->accelerator, "gpu_f16") == 0) ||
                             (strcmp(configs->accelerator, "gpu") == 0))) {
        auto options = TfLiteGpuDelegateOptionsV2Default();
        if (strcmp(configs->accelerator, "gpu_f16") == 0)
          options.inference_priority1 =
              TFLITE_GPU_INFERENCE_PRIORITY_MIN_LATENCY;
        delegate = TfLiteGpuDelegateV2Create(&options);
      } else if (strcmp(configs->accelerator, "nnapi") == 0) {
        auto options = tflite::StatefulNnApiDelegate::Options();
        options.allow_fp16 = true;
        options.disallow_nnapi_cpu = true;
        options.accelerator_name = "google-edgetpu";
        options.use_burst_computation = true;
        delegate = new tflite::StatefulNnApiDelegate(options);
      }
      if (delegate != nullptr) {
        TfLiteInterpreterOptionsAddDelegate(option_ptr, delegate);
      }
#endif
    }
  };

  for (int k = 0; k < N_OFFLINE_INTEPRETERS; k++) {
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

    // Create the interpreter.
    backend_data->interpreter8[k] =
        TfLiteInterpreterCreate(backend_data->model, backend_data->options[k]);
    if (!backend_data->interpreter8[k]) {
      // create a vanilla interpreter
      backend_data->interpreter8[k] = TfLiteInterpreterCreate(
          backend_data->model, TfLiteInterpreterOptionsCreate());
      if (!backend_data->interpreter8[k]) {
        printf("Failed to create the interpreter");
        mlperf_backend_delete(backend_data);
        return nullptr;
      }
    }
  }

  backend_data->input_tensor_count =
      TfLiteInterpreterGetInputTensorCount(backend_data->interpreter[0]);
  for (int i = 0; i < N_OFFLINE_INTEPRETERS; i++) {
    backend_data->acc_data[i] =
        (void**)malloc(sizeof(void*) * backend_data->input_tensor_count);
    memset(backend_data->acc_data[i], 0,
           sizeof(void*) * backend_data->input_tensor_count);
  }

  for (int k = 0; k < N_OFFLINE_INTEPRETERS; k++) {
    for (int i = 0; i < backend_data->input_tensor_count; ++i) {
      TfLiteTensor* tensor =
          TfLiteInterpreterGetInputTensor(backend_data->interpreter[k], i);
      int32_t* dims = (int32_t*)malloc(sizeof(int32_t) * tensor->dims->size);
      dims[0] = 1;
      for (int i = 1; i < tensor->dims->size; i++) {
        dims[i] = tensor->dims->data[i];
      }
      TfLiteInterpreterResizeInputTensor(backend_data->interpreter[k], i, dims,
                                         tensor->dims->size);
      free(dims);
    }
    if (kTfLiteOk !=
        TfLiteInterpreterAllocateTensors(backend_data->interpreter[k])) {
      printf("Failed to allocate tensors");
      return nullptr;
    }
  }

  for (int k = 0; k < N_OFFLINE_INTEPRETERS; k++) {
    for (int i = 0; i < backend_data->input_tensor_count; ++i) {
      TfLiteTensor* tensor =
          TfLiteInterpreterGetInputTensor(backend_data->interpreter8[k], i);
      int32_t* dims = (int32_t*)malloc(sizeof(int32_t) * tensor->dims->size);
      dims[0] = 8;
      for (int i = 1; i < tensor->dims->size; i++) {
        dims[i] = tensor->dims->data[i];
      }
      TfLiteInterpreterResizeInputTensor(backend_data->interpreter8[k], i, dims,
                                         tensor->dims->size);
      free(dims);
    }
    if (kTfLiteOk !=
        TfLiteInterpreterAllocateTensors(backend_data->interpreter8[k])) {
      printf("Failed to allocate tensors");
      break;
    }
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
  for (int i = 0; i < N_OFFLINE_INTEPRETERS; i++) {
    if (backend_data->use_shard) {
      free(backend_data->acc_data[i]);
    }
    backend_data->acc_data[i] = nullptr;
  }
  TfLiteModelDelete(backend_data->model);
  for (int i = 0; i < N_OFFLINE_INTEPRETERS; i++) {
    TfLiteInterpreterOptionsDelete(backend_data->options[i]);
    TfLiteInterpreterDelete(backend_data->interpreter[i]);
    TfLiteInterpreterDelete(backend_data->interpreter8[i]);
  }
  delete backend_data;
  backendExists = false;
}

// Run the inference for a sample.
mlperf_status_t mlperf_backend_issue_query(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;

  // main thread for batch_size == 1
  if (!backend_data->use_shard) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(6, &cpuset);
    CPU_SET(7, &cpuset);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
    if (TfLiteInterpreterInvoke(backend_data->interpreter[0]) != kTfLiteOk) {
      printf("Failed to run the inference");
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
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(6, &cpuset);
  CPU_SET(7, &cpuset);
  sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);

  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  const int real_batch_size = backend_data->batch_size / N_OFFLINE_INTEPRETERS;
  const int shard = batch_index / (real_batch_size);
  if (shard == 0 && batch_index == 0 && backend_data->use_shard == false) {
    backend_data->use_shard = false;
  } else {
    backend_data->use_shard = true;
  }
  int real_batch_index = batch_index % real_batch_size;

  TfLiteTensor* tensor = nullptr;
  if (backend_data->use_shard) {
    tensor =
        TfLiteInterpreterGetInputTensor(backend_data->interpreter8[shard], i);
  } else {
    tensor =
        TfLiteInterpreterGetInputTensor(backend_data->interpreter[shard], i);
  }
  if (real_batch_index == 0 && backend_data->use_shard == false) {
    if (backend_data->original_tensor_size == 0) {
      backend_data->original_tensor_size = tensor->bytes;
      memcpy((char*)tensor->data.raw, (char*)data,
             backend_data->original_tensor_size);
      backend_data->has_temp_data = true;
    } else {
      tensor->data.raw = (char*)data;
    }
  }

  if (backend_data->use_shard) {
    if (backend_data->acc_data[shard][i] == nullptr) {
      backend_data->acc_data[shard][i] =
          malloc(real_batch_size * backend_data->original_tensor_size);
    }
    if (backend_data->has_temp_data) {
      TfLiteTensor* tensor =
          TfLiteInterpreterGetInputTensor(backend_data->interpreter[shard], i);
      memcpy((char*)backend_data->acc_data[shard][i], (char*)tensor->data.raw,
             backend_data->original_tensor_size);
      backend_data->has_temp_data = false;
    }
    memcpy(((char*)backend_data->acc_data[shard][i] +
            ((batch_index % real_batch_size) *
             backend_data->original_tensor_size)),
           data, backend_data->original_tensor_size);
    if (real_batch_index == (real_batch_size - 1)) {
      tensor->data.raw = (char*)backend_data->acc_data[shard][i];
    }
  }

  // Allocate tensors.
  if (((batch_index + 1) % real_batch_size) == 0 &&
      i == (backend_data->input_tensor_count - 1)) {
    auto task = [](TFLiteBackendData* backend_data, int index) -> TfLiteStatus {
      if (backend_data->use_shard) {
        return TfLiteInterpreterInvoke(backend_data->interpreter8[index]);
      } else {
        return TfLiteInterpreterInvoke(backend_data->interpreter[index]);
      }
    };

    // dispatch workers
    if (backend_data->use_shard) {
      backend_data->status[shard] =
          backend_data->executer->submit(task, backend_data, shard);
    }
  }

  return MLPERF_SUCCESS;
}

// Return the number of outputs for the model.
int32_t mlperf_backend_get_output_count(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  if (backend_data->use_shard) {
    return TfLiteInterpreterGetOutputTensorCount(backend_data->interpreter8[0]);
  } else {
    return TfLiteInterpreterGetOutputTensorCount(backend_data->interpreter[0]);
  }
}

// Return the type of ith output.
mlperf_data_t mlperf_backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                             int32_t i) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  const TfLiteTensor* tensor;
  if (backend_data->use_shard) {
    tensor = TfLiteInterpreterGetOutputTensor(backend_data->interpreter8[0], i);
  } else {
    tensor = TfLiteInterpreterGetOutputTensor(backend_data->interpreter[0], i);
  }
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
      (backend_data->use_shard)
          ? backend_data->batch_size / N_OFFLINE_INTEPRETERS
          : 1;
  const int shard = batch_index / (real_batch_size);

  if (backend_data->use_shard) {
    if (backend_data->status[shard].valid()) {
      if (backend_data->status[shard].get() != kTfLiteOk) {
        printf("Failed to get output: %d", shard);
        return MLPERF_FAILURE;
      }
    }
  }

  const TfLiteTensor* output_tensor;
  if (backend_data->use_shard) {
    output_tensor =
        TfLiteInterpreterGetOutputTensor(backend_data->interpreter8[shard], i);
  } else {
    output_tensor =
        TfLiteInterpreterGetOutputTensor(backend_data->interpreter[shard], i);
  }
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
