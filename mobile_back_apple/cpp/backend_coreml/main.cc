/* Copyright 2022 The MLPerf Authors. All Rights Reserved.

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

#import <CoreML/CoreML.h>

#include <cstring>

#include "coreml_util-Swift.h"
#include "flutter/cpp/c/backend_c.h"
#include "flutter/cpp/c/type.h"
#include "flutter/cpp/utils.h"
#include "mobile_back_apple/cpp/backend_coreml/coreml_settings.pbtxt.h"

struct CoreMLBackendData {
  const char *name = "Core ML";
  const char *vendor = "Apple";
  const char *accelerator{nullptr};
  CoreMLExecutor *coreMLExecutor{nullptr};
  bool expectNCHW = false;
};

inline mlperf_data_t::Type MLMultiArrayDataType2MLPerfDataType(
    MLMultiArrayDataType type) {
  switch (type) {
    case MLMultiArrayDataTypeFloat32:
      return mlperf_data_t::Float32;
    case MLMultiArrayDataTypeInt32:
      return mlperf_data_t::Int32;
    default:
      LOG(ERROR) << "MLMultiArrayDataType " << type << " is not supported";
      return mlperf_data_t::Float32;
  }
}

static bool backendExists = false;

template <typename T>
void convert_nhwc_to_nchw(T *data_nhwc, int N, int H, int W, int C) {
  T *data_nchw = new T[N * C * H * W];
  for (int n = 0; n < N; ++n) {
    for (int c = 0; c < C; ++c) {
      for (int h = 0; h < H; ++h) {
        for (int w = 0; w < W; ++w) {
          int index_nchw = ((n * C + c) * H + h) * W + w;
          int index_nhwc = ((n * H + h) * W + w) * C + c;
          data_nchw[index_nchw] = data_nhwc[index_nhwc];
        }
      }
    }
  }
  std::memcpy(data_nhwc, data_nchw, N * H * W * C * sizeof(T));
  delete[] data_nchw;
}

// Return the name of the backend
const char *mlperf_backend_vendor_name(mlperf_backend_ptr_t backend_ptr) {
  return ((CoreMLBackendData *)backend_ptr)->vendor;
}

// Return the name of the accelerator.
const char *mlperf_backend_accelerator_name(mlperf_backend_ptr_t backend_ptr) {
  return ((CoreMLBackendData *)backend_ptr)->accelerator;
}

// Return the name of this backend.
const char *mlperf_backend_name(mlperf_backend_ptr_t backend_ptr) {
  return ((CoreMLBackendData *)backend_ptr)->name;
}

// Should return true if current hardware is supported.
bool mlperf_backend_matches_hardware(const char **not_allowed_message,
                                     const char **settings,
                                     const mlperf_device_info_t *device_info) {
  (void)device_info;
  *not_allowed_message = nullptr;
  *settings = coreml_settings_pbtxt.c_str();
  return true;
}

// Create a new backend and return the pointer to it.
mlperf_backend_ptr_t mlperf_backend_create(
    const char *model_path, mlperf_backend_configuration_t *configs,
    const char *native_lib_path) {
  // Verify only one instance of the backend exists at any time
  if (backendExists) {
    LOG(ERROR) << "Only one backend instance should exist at a time";
    return nullptr;
  }

  CoreMLBackendData *backend_data = new CoreMLBackendData();
  backendExists = true;
  // quick hack for checking if model expects NCHW input.
  if (strcasestr(model_path, "NCHW") != nullptr) {
    backend_data->expectNCHW = true;
    LOG(INFO) << "Will convert inputs from NHWC to NCHW!";
  }

  // Load the model.
  NSError *error;
  CoreMLExecutor *coreMLExecutor =
      [[CoreMLExecutor alloc] initWithModelPath:model_path
                                      batchSize:configs->batch_size
                                acceleratorName:configs->accelerator
                                          error:&error];
  if (!coreMLExecutor || error) {
    LOG(ERROR) << "Cannot create CoreMLExecutor";
    return nullptr;
  }
  backend_data->coreMLExecutor = coreMLExecutor;
  backend_data->accelerator = [coreMLExecutor getAccelerator];
  LOG(INFO) << "Accelerator: " << backend_data->accelerator;

  return backend_data;
}

// Destroy the backend pointer and its data.
void mlperf_backend_delete(mlperf_backend_ptr_t backend_ptr) {
  CoreMLBackendData *backend_data = (CoreMLBackendData *)backend_ptr;
  backend_data->coreMLExecutor = nil;
  backend_data->accelerator = nil;
  delete backend_data;
  backendExists = false;
}

// Run the inference for a sample.
mlperf_status_t mlperf_backend_issue_query(mlperf_backend_ptr_t backend_ptr) {
  CoreMLBackendData *backend_data = (CoreMLBackendData *)backend_ptr;
  if ([backend_data->coreMLExecutor issueQueries]) return MLPERF_SUCCESS;
  return MLPERF_FAILURE;
}

// Flush the staged queries immediately.
mlperf_status_t mlperf_backend_flush_queries(mlperf_backend_ptr_t backend_ptr) {
  CoreMLBackendData *backend_data = (CoreMLBackendData *)backend_ptr;
  if ([backend_data->coreMLExecutor flushQueries]) return MLPERF_SUCCESS;
  return MLPERF_FAILURE;
}

// Return the number of inputs of the model.
int32_t mlperf_backend_get_input_count(mlperf_backend_ptr_t backend_ptr) {
  return [((CoreMLBackendData *)backend_ptr)->coreMLExecutor getInputCount];
}

// Return the type of the ith input.
mlperf_data_t mlperf_backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                            int32_t i) {
  mlperf_data_t data;
  CoreMLBackendData *backend_data = (CoreMLBackendData *)backend_ptr;
  NSNumber *inputTypeNumber = [backend_data->coreMLExecutor getInputTypeAt:i];
  MLMultiArrayDataType coreml_type =
      (MLMultiArrayDataType)inputTypeNumber.integerValue;
  data.type = MLMultiArrayDataType2MLPerfDataType(coreml_type);
  data.size = [backend_data->coreMLExecutor getInputSizeAt:i];
  return data;
}

// Set the data for ith input.
mlperf_status_t mlperf_backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                         int32_t batch_index, int32_t i,
                                         void *data) {
  CoreMLBackendData *backend_data = (CoreMLBackendData *)backend_ptr;
  if ([backend_data->coreMLExecutor setInputData:data
                                              at:i
                                      batchIndex:batch_index])
    return MLPERF_SUCCESS;
  return MLPERF_FAILURE;
}

// Return the number of outputs for the model.
int32_t mlperf_backend_get_output_count(mlperf_backend_ptr_t backend_ptr) {
  return [((CoreMLBackendData *)backend_ptr)->coreMLExecutor getOutputCount];
}

// Return the type of ith output.
mlperf_data_t mlperf_backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                             int32_t i) {
  mlperf_data_t data;
  CoreMLBackendData *backend_data = (CoreMLBackendData *)backend_ptr;

  NSNumber *outputTypeNumber = [backend_data->coreMLExecutor getOutputTypeAt:i];
  MLMultiArrayDataType coreml_type =
      (MLMultiArrayDataType)outputTypeNumber.integerValue;
  data.type = MLMultiArrayDataType2MLPerfDataType(coreml_type);
  data.size = [backend_data->coreMLExecutor getOutputSizeAt:i];
  return data;
}

// Get the data from ith output.
mlperf_status_t mlperf_backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                          uint32_t batch_index, int32_t i,
                                          void **data) {
  CoreMLBackendData *backend_data = (CoreMLBackendData *)backend_ptr;
  if ([backend_data->coreMLExecutor getOutputData:data
                                               at:i
                                       batchIndex:batch_index])
    return MLPERF_SUCCESS;
  return MLPERF_FAILURE;
}

void mlperf_backend_convert_inputs(mlperf_backend_ptr_t backend_ptr, int bytes,
                                   int width, int height, uint8_t *data) {
  CoreMLBackendData *backend_data = (CoreMLBackendData *)backend_ptr;
  if (backend_data->expectNCHW) {
    int N = 1, H = height, W = width, C = 3;
    convert_nhwc_to_nchw(reinterpret_cast<float *>(data), N, H, W, C);
  }
}
