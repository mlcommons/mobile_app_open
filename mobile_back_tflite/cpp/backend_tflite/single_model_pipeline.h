/* Copyright 2024 The MLPerf Authors. All Rights Reserved.
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

#ifndef TFLITE_SINGLE_MODEL_PIPELINE_H
#define TFLITE_SINGLE_MODEL_PIPELINE_H

#include "flutter/cpp/c/type.h"
#include "tensorflow/lite/c/c_api.h"

// A simple pipeline which runs a single model.
class SingleModelPipeline {
 public:
  // Destroy the backend pointer and its data.
  void backend_delete(mlperf_backend_ptr_t backend_ptr);

  // Create a new backend and return the pointer to it.
  mlperf_backend_ptr_t backend_create(const char *model_path,
                                      mlperf_backend_configuration_t *configs,
                                      const char *native_lib_path);

  // Vendor name who create this backend.
  const char *backend_vendor_name(mlperf_backend_ptr_t backend_ptr);

  // Return the name of the accelerator.
  const char *backend_accelerator_name(mlperf_backend_ptr_t backend_ptr);

  // Return the name of this backend.
  const char *backend_name(mlperf_backend_ptr_t backend_ptr);

  // Run the inference for a sample.
  mlperf_status_t backend_issue_query(mlperf_backend_ptr_t backend_ptr);

  // Flush the staged queries immediately.
  mlperf_status_t backend_flush_queries(mlperf_backend_ptr_t backend_ptr);

  // Return the number of inputs of the model.
  int32_t backend_get_input_count(mlperf_backend_ptr_t backend_ptr);

  // Return the type of the ith input.
  mlperf_data_t backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                       int32_t i);

  // Set the data for ith input.
  mlperf_status_t backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                    int32_t batch_index, int32_t i, void *data);

  // Return the number of outputs for the model.
  int32_t backend_get_output_count(mlperf_backend_ptr_t backend_ptr);

  // Return the type of the ith output.
  mlperf_data_t backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                        int32_t i);

  // Get the data from ith output.
  mlperf_status_t backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                     uint32_t batchIndex, int32_t i,
                                     void **data);

  void backend_convert_inputs(mlperf_backend_ptr_t backend_ptr, int bytes,
                              int width, int height, uint8_t *data);

  void *backend_get_buffer(size_t n);

  void backend_release_buffer(void *p);
};

#endif  // TFLITE_SINGLE_MODEL_PIPELINE_H