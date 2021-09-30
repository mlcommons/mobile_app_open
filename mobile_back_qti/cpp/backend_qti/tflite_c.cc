/* Copyright (c) 2020-2021 Qualcomm Innovation Center, Inc. All rights reserved.

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tensorflow/core/platform/logging.h"
#include "tensorflow/lite/c/c_api.h"
#include "tensorflow/lite/c/common.h"
#if __ANDROID__
#include "tensorflow/lite/delegates/gpu/delegate.h"
#include "tensorflow/lite/delegates/nnapi/nnapi_delegate.h"
#endif
#include "tflite_c.h"

void tflite_backend_delete(mlperf_backend_ptr_t backend_ptr);

struct TFLiteBackendData {
  const char* name = "TFLite-C";
  TfLiteModel* model{nullptr};
  TfLiteInterpreterOptions* options{nullptr};
  TfLiteInterpreter* interpreter{nullptr};
  TfLiteDelegate* delegate{nullptr};
};

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

// Create a new backend and return the pointer to it.
mlperf_backend_ptr_t tflite_backend_create(
    const char* model_path, mlperf_backend_configuration_t* configs) {
  TFLiteBackendData* backend_data = new TFLiteBackendData();

  // Load the model.
  backend_data->model = TfLiteModelCreateFromFile(model_path);
  if (!backend_data->model) {
    LOG(FATAL) << "Failed to load model: " << model_path;
    tflite_backend_delete(backend_data);
    return nullptr;
  }

  // Create interpreter options.
  backend_data->options = TfLiteInterpreterOptionsCreate();
  std::string delegateStr = configs->accelerator;
#if __ANDROID__
  if (delegateStr == "gpu_f16") {
    TfLiteGpuDelegateOptionsV2 options = TfLiteGpuDelegateOptionsV2Default();
    options.inference_preference =
        TFLITE_GPU_INFERENCE_PREFERENCE_SUSTAINED_SPEED;
    options.inference_priority1 = TFLITE_GPU_INFERENCE_PRIORITY_MIN_LATENCY;
    options.max_delegated_partitions = 1;
    backend_data->delegate = TfLiteGpuDelegateV2Create(&options);
  } else if (delegateStr.find("nnapi") == 0) {
    auto options = tflite::StatefulNnApiDelegate::Options();
    options.allow_fp16 = false;
    options.disallow_nnapi_cpu = true;
    std::string accelerator_name =
        (delegateStr.find("-") != std::string::npos)
            ? delegateStr.substr(delegateStr.find('-') + 1)
            : std::string();
    options.execution_preference = tflite::StatefulNnApiDelegate::Options::
        ExecutionPreference::kFastSingleAnswer;
    if (!accelerator_name.empty()) {
      options.accelerator_name = accelerator_name.c_str();
    }
    backend_data->delegate = new tflite::StatefulNnApiDelegate(options);
  }
  if (backend_data->delegate != nullptr) {
    TfLiteInterpreterOptionsAddDelegate(backend_data->options,
                                        backend_data->delegate);
  }
#endif
  for (int i = 0; i < configs->count; ++i) {
    if (strcmp(configs->keys[i], "num_threads") == 0) {
      TfLiteInterpreterOptionsSetNumThreads(backend_data->options,
                                            atoi(configs->values[i]));
    }
  }

  // Create the interpreter.
  backend_data->interpreter =
      TfLiteInterpreterCreate(backend_data->model, backend_data->options);
  if (!backend_data->interpreter) {
    LOG(FATAL) << "Failed to create the interpreter";
    tflite_backend_delete(backend_data);
    return nullptr;
  }

  // Allocate tensors.
  if (TfLiteInterpreterAllocateTensors(backend_data->interpreter) !=
      kTfLiteOk) {
    LOG(FATAL) << "Failed to create the interpreter";
    tflite_backend_delete(backend_data);
    return nullptr;
  }

  return backend_data;
}

// Destroy the backend pointer and its data.
void tflite_backend_delete(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  delete backend_data->delegate;
  TfLiteModelDelete(backend_data->model);
  TfLiteInterpreterOptionsDelete(backend_data->options);
  TfLiteInterpreterDelete(backend_data->interpreter);
  delete backend_data;
}

// Run the inference for a sample.
mlperf_status_t tflite_backend_issue_query(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  if (TfLiteInterpreterInvoke(backend_data->interpreter) != kTfLiteOk) {
    printf("Failed to run the inference");
    return MLPERF_FAILURE;
  }
  return MLPERF_SUCCESS;
}

// Flush the staged queries immediately.
mlperf_status_t tflite_backend_flush_queries(mlperf_backend_ptr_t backend_ptr) {
  return MLPERF_SUCCESS;
}

// Return the number of inputs of the model.
int32_t tflite_backend_get_input_count(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  return TfLiteInterpreterGetInputTensorCount(backend_data->interpreter);
}

// Return the type of the ith input.
mlperf_data_t tflite_backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                            int32_t i) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  const TfLiteTensor* tensor =
      TfLiteInterpreterGetInputTensor(backend_data->interpreter, i);
  mlperf_data_t type;
  type.type = TfType2Type(TfLiteTensorType(tensor));
  type.size = TFLiteNumElements(tensor);
  return type;
}
// Set the data for ith input.
mlperf_status_t tflite_backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                         int32_t batchIndex, int32_t i,
                                         void* data) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  TfLiteTensor* tensor =
      TfLiteInterpreterGetInputTensor(backend_data->interpreter, i);
  tensor->data.raw = (char*)data;
  return MLPERF_SUCCESS;
}

// Return the number of outputs fro the model.
int32_t tflite_backend_get_output_count(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  return TfLiteInterpreterGetOutputTensorCount(backend_data->interpreter);
}
// Return the type of ith output.
mlperf_data_t tflite_backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                             int32_t i) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  const TfLiteTensor* tensor =
      TfLiteInterpreterGetOutputTensor(backend_data->interpreter, i);
  mlperf_data_t type;
  type.type = TfType2Type(TfLiteTensorType(tensor));
  type.size = TFLiteNumElements(tensor);
  return type;
}

// Get the data from ith output.
mlperf_status_t tflite_backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                          uint32_t batchIndex, int32_t i,
                                          void** data) {
  TFLiteBackendData* backend_data = (TFLiteBackendData*)backend_ptr;
  const TfLiteTensor* output_tensor =
      TfLiteInterpreterGetOutputTensor(backend_data->interpreter, i);
  switch (output_tensor->type) {
    case kTfLiteFloat32:
      *data = output_tensor->data.f;
      break;
    case kTfLiteUInt8:
      *data = output_tensor->data.uint8;
      break;
    case kTfLiteInt8:
      *data = output_tensor->data.int8;
      break;
    case kTfLiteFloat16:
      *data = output_tensor->data.f16;
      break;
    case kTfLiteInt32:
      *data = output_tensor->data.i32;
      break;
    case kTfLiteInt64:
      *data = output_tensor->data.i64;
      break;
    default:
      printf("Data type not yet supported");
      return MLPERF_FAILURE;
  }
  return MLPERF_SUCCESS;
}
