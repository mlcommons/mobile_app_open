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
#include <unistd.h>

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

#define N_OFFLINE_INTERPRETERS 8

struct TFLiteBackendData {
  const char* name = "TFLite-pixel";
  const char* vendor = "Google";
  TfLiteModel* model{nullptr};
  std::vector<TfLiteInterpreterOptions*> options{};
  std::vector<TfLiteInterpreter*> interpreter{};
  int32_t shards_num = 1;
  uint32_t real_batch_size = 1;
  std::unique_ptr<Threadpool> executer;
  int32_t original_tensor_size = 0;
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
      printf("TfLiteType %d not supported\n", type);
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
                                     const mlperf_device_info_t* device_info,
                                     const char* native_lib_path) {
  *not_allowed_message = nullptr;
  *settings = tflite_settings.c_str();

  if (device_info && device_info->model && device_info->manufacturer) {
    LOG(INFO) << "Pixel HW supported check: model: " << device_info->model
              << ", manufacturer: " << device_info->manufacturer;

    if (strcmp(device_info->manufacturer, "Google") == 0 &&
        access("/dev/edgetpu", F_OK) == 0) {
      LOG(INFO) << "Pixel backend matches hardware";
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
    printf("Error: Only one backend instance should exist at a time\n");
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

  if (configs->batch_size > 1) {
    backend_data->shards_num = N_OFFLINE_INTERPRETERS;

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
    }

#if __ANDROID__
    if (!is_emulator() && ((strcmp(configs->accelerator, "gpu_f16") == 0) ||
                           (strcmp(configs->accelerator, "gpu") == 0))) {
      auto options = TfLiteGpuDelegateOptionsV2Default();
      if (strcmp(configs->accelerator, "gpu_f16") == 0)
        options.inference_priority1 = TFLITE_GPU_INFERENCE_PRIORITY_MIN_LATENCY;
      delegate = TfLiteGpuDelegateV2Create(&options);
    } else if (strcmp(configs->accelerator, "tpu") == 0) {
      auto options = tflite::StatefulNnApiDelegate::Options();
      options.allow_fp16 = true;
      options.disallow_nnapi_cpu = true;
      options.accelerator_name = "google-edgetpu";
      delegate = new tflite::StatefulNnApiDelegate(options);
    }
    if (delegate != nullptr) {
      TfLiteInterpreterOptionsAddDelegate(option_ptr, delegate);
    }
#endif
  };

  backend_data->options.resize(backend_data->shards_num);
  backend_data->interpreter.resize(backend_data->shards_num);

  for (int k = 0; k < backend_data->shards_num; k++) {
    // Create Backend Option
    create_option(backend_data->options[k]);

    // Create the interpreter.
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
    TfLiteInterpreter*& shard = backend_data->interpreter[shard_index];

    for (int input_index = 0; input_index < input_tensor_count; input_index++) {
      TfLiteTensor* tensor =
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
const char* mlperf_backend_vendor_name(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  return backend_data->vendor;
}

// TODO: Return the name of the accelerator.
const char* mlperf_backend_accelerator_name(mlperf_backend_ptr_t backend_ptr) {
  return "ACCELERATOR_NAME";
}

// Return the name of this backend.
const char* mlperf_backend_name(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  return backend_data->name;
}

// Destroy the backend pointer and its data.
void mlperf_backend_delete(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
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
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
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
  type.size /= backend_data->real_batch_size;
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

  const int shard_index = batch_index / backend_data->real_batch_size;
  TfLiteTensor* tensor = TfLiteInterpreterGetInputTensor(
      backend_data->interpreter[shard_index], i);
  const int data_offset = backend_data->original_tensor_size *
                          (batch_index % backend_data->real_batch_size);
  memcpy(tensor->data.raw + data_offset, data,
         backend_data->original_tensor_size);

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
  type.size /= backend_data->real_batch_size;
  return type;
}

// Get the data from ith output.
mlperf_status_t mlperf_backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                          uint32_t batch_index, int32_t i,
                                          void** data) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  const int shard_index = batch_index / backend_data->real_batch_size;

  const TfLiteTensor* output_tensor = TfLiteInterpreterGetOutputTensor(
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
