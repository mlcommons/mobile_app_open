/* Copyright 2020 The MLPerf Authors. All Rights Reserved.

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
#ifndef MLPERF_C_BACKEND_C_H_
#define MLPERF_C_BACKEND_C_H_

#include <stdint.h>

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// Should return true if current hardware is supported.
bool mlperf_backend_matches_hardware(const char** not_allowed_message,
                                     const char** settings,
                                     const mlperf_device_info_t* device_info,
                                     const char* native_lib_path);

// Create a new backend and return the pointer to it.
mlperf_backend_ptr_t mlperf_backend_create(
    const char* model_path, mlperf_backend_configuration_t* configs,
    const char* native_lib_path);

// Vendor name who create this backend.
const char* mlperf_backend_vendor_name(mlperf_backend_ptr_t backend_ptr);

// Return the name of this backend.
const char* mlperf_backend_name(mlperf_backend_ptr_t backend_ptr);

// Destroy the backend pointer and its data.
void mlperf_backend_delete(mlperf_backend_ptr_t backend_ptr);

// Run the inference for a sample.
mlperf_status_t mlperf_backend_issue_query(mlperf_backend_ptr_t backend_ptr);
// Flush the staged queries immediately.
mlperf_status_t mlperf_backend_flush_queries(mlperf_backend_ptr_t backend_ptr);

// Return the number of inputs of the model.
int32_t mlperf_backend_get_input_count(mlperf_backend_ptr_t backend_ptr);
// Return the type of the ith input.
mlperf_data_t mlperf_backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                            int32_t i);
// Set the data for ith input, of batchIndex'th batch
mlperf_status_t mlperf_backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                         int32_t batchIndex, int32_t i,
                                         void* data);

// Return the number of outputs from the model.
int32_t mlperf_backend_get_output_count(mlperf_backend_ptr_t backend_ptr);
// Return the type of ith output.
mlperf_data_t mlperf_backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                             int32_t i);
// Get the data from ith output.
mlperf_status_t mlperf_backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                          uint32_t batchIndex, int32_t i,
                                          void** data);

// Optional functions
void mlperf_backend_convert_inputs(mlperf_backend_ptr_t backend_ptr, int bytes,
                                   int width, int height, uint8_t* data);

void* mlperf_backend_get_buffer(size_t size);

void mlperf_backend_release_buffer(void* p);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // MLPERF_C_BACKEND_C_H_
