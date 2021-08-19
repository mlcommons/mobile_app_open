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

#ifndef LOG_TAG
#define LOG_TAG "N/A"
#endif

#define __SHARP_X(x) #x
#define __STR(x) __SHARP_X(x)
#define _MLOG(_loglevel, fmt, ...)                                          \
  __android_log_print(_loglevel, "MLPerf",                                  \
                      "[Backend][" LOG_TAG "] %s:" __STR(__LINE__) ": " fmt \
                                                                   "\n",    \
                      __FUNCTION__, ##__VA_ARGS__)
#define MLOGV(fmt, ...) _MLOG(ANDROID_LOG_VERBOSE, fmt, ##__VA_ARGS__)
#define MLOGD(fmt, ...) _MLOG(ANDROID_LOG_DEBUG, fmt, ##__VA_ARGS__)
#define MLOGE(fmt, ...) _MLOG(ANDROID_LOG_ERROR, fmt, ##__VA_ARGS__)

// Should return true if current hardware is supported.
bool mlperf_backend_matches_hardware(const char** not_allowed_message,
                                     const char** settings, mlperf_device_info_t* device_info);

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

void mlperf_backend_convert_inputs(mlperf_backend_ptr_t backend_ptr,
                                   int bytes, int width, int height, uint8_t* data);

struct Backend {
  const std::string name_ = "Samsung";
  std::vector<mlperf_data_t> input_format_;
  std::vector<mlperf_data_t> output_format_;
  std::vector<uint8_t>* outputs_buffer;
  std::vector<uint8_t*>* input_conv;
  std::vector<float>* detected_label_boxes;
  std::vector<float>* detected_label_indices;
  std::vector<float>* detected_label_probabilities;
  std::vector<float>* num_detections;
  std::vector<char>* model_buffer;
  int input_size;
  int output_size;
  std::string model;
  std::string model_path;
  int batch;

  // for mobile bert
  bool isMobileBertModel_ = false;
  std::vector<int32_t> m_inputs_;
  std::vector<int8_t> m_outputs_;
  std::vector<float> out0_;
  std::vector<float> out1_;

  bool created;

  // bool isFirst = false;

  Backend()
      : outputs_buffer(nullptr),
        input_conv(nullptr),
        detected_label_boxes(nullptr),
        detected_label_indices(nullptr),
        detected_label_probabilities(nullptr),
        num_detections(nullptr),
        model_buffer(nullptr),
        input_size(0),
        output_size(0),
        batch(0),
        created(false) {}
};

typedef enum MLPERF_MODEL_TYPE {
  MLPERF_MODEL_TYPE_NONE,
  MLPERF_MODEL_TYPE_IMAGE_CLASSIFICATION,
  MLPERF_MODEL_TYPE_OBJECT_DETECTION,
  MLPERF_MODEL_TYPE_IMAGE_SEGMENTATION,
  MLPERF_MODEL_TYPE_LANGUAGE_PROCESSING,
} MLPERF_MODEL_TYPE;

typedef enum IMAGE_FORMAT {
  // BGR by pixel order
  IMAGE_FORMAT_BGR = 0x1,
  // BGR by channel order
  IMAGE_FORMAT_BGRC,
} IMAGE_FORMAT;

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // MLPERF_C_BACKEND_C_H_
