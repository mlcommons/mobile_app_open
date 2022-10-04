/* Copyright (c) 2020-2022 Qualcomm Innovation Center, Inc. All rights reserved.

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
#pragma once

#include "flutter/cpp/c/backend_c.h"
#include "flutter/cpp/c/type.h"

// Create a new backend and return the pointer to it.
mlperf_backend_ptr_t tflite_backend_create(
    const char* model_path, mlperf_backend_configuration_t* configs);
// Destroy the backend pointer and its data.
void tflite_backend_delete(mlperf_backend_ptr_t backend_ptr);
// Run the inference for a sample.
mlperf_status_t tflite_backend_issue_query(mlperf_backend_ptr_t backend_ptr);
// Flush the staged queries immediately.
mlperf_status_t tflite_backend_flush_queries(mlperf_backend_ptr_t backend_ptr);
// Return the number of inputs of the model.
int32_t tflite_backend_get_input_count(mlperf_backend_ptr_t backend_ptr);
// Return the type of the ith input.
mlperf_data_t tflite_backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                            int32_t i);
// Set the data for ith input.
mlperf_status_t tflite_backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                         int32_t batchIndex, int32_t i,
                                         void* data);
// Return the number of outputs fro the model.
int32_t tflite_backend_get_output_count(mlperf_backend_ptr_t backend_ptr);
// Return the type of ith output.
mlperf_data_t tflite_backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                             int32_t i);
// Get the data from ith output.
mlperf_status_t tflite_backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                          uint32_t batchIndex, int32_t i,
                                          void** data);
