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
#include "single_model_pipeline.h"
#include "stable_diffusion_pipeline.h"
#include "tensorflow/core/platform/logging.h"
#include "tflite_settings_pixel.h"

#if __ANDROID__
#include <sys/system_properties.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

std::unique_ptr<Pipeline> pipeline;

void init_pipeline(const char *pipeline_type) {
  bool sd_pipeline = (strcmp(pipeline_type, "StableDiffusionPipeline") == 0);
  if (sd_pipeline) {
    LOG(INFO) << "Initializing StableDiffusionPipeline";
    pipeline = std::make_unique<StableDiffusionPipeline>();
  } else {
    LOG(INFO) << "Initializing SingleModelPipeline";
    pipeline = std::make_unique<SingleModelPipeline>();
  }
}

void reset_pipeline() { pipeline.reset(); }

// TFLite is the standard backend for all hardware.
bool mlperf_backend_matches_hardware(const char **not_allowed_message,
                                     const char **settings,
                                     const mlperf_device_info_t *device_info) {
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

// Create a new backend and return the pointer to it.
mlperf_backend_ptr_t mlperf_backend_create(
    const char *model_path, mlperf_backend_configuration_t *configs,
    const char *native_lib_path) {
  const char *pipeline_type = "";
  for (int i = 0; i < configs->count; ++i) {
    if (strcmp(configs->keys[i], "pipeline") == 0) {
      pipeline_type = configs->values[i];
      break;
    }
  }
  init_pipeline(pipeline_type);
  return pipeline->backend_create(model_path, configs, native_lib_path);
}

// Vendor name who create this backend.
const char *mlperf_backend_vendor_name(mlperf_backend_ptr_t backend_ptr) {
  return pipeline->backend_vendor_name(backend_ptr);
}

// TODO: Return the name of the accelerator.
const char *mlperf_backend_accelerator_name(mlperf_backend_ptr_t backend_ptr) {
  return pipeline->backend_accelerator_name(backend_ptr);
}

// Return the name of this backend.
const char *mlperf_backend_name(mlperf_backend_ptr_t backend_ptr) {
  return pipeline->backend_name(backend_ptr);
}

// Destroy the backend pointer and its data.
void mlperf_backend_delete(mlperf_backend_ptr_t backend_ptr) {
  pipeline->backend_delete(backend_ptr);
  reset_pipeline();
}

// Run the inference for a sample.
mlperf_status_t mlperf_backend_issue_query(mlperf_backend_ptr_t backend_ptr,
                                           ft_callback callback,
                                           void *context) {
  return pipeline->backend_issue_query(backend_ptr, callback, context);
}

// Flush the staged queries immediately.
mlperf_status_t mlperf_backend_flush_queries(mlperf_backend_ptr_t backend_ptr) {
  return pipeline->backend_flush_queries(backend_ptr);
}

// Return the number of inputs of the model.
int32_t mlperf_backend_get_input_count(mlperf_backend_ptr_t backend_ptr) {
  return pipeline->backend_get_input_count(backend_ptr);
}

// Return the type of the ith input.
mlperf_data_t mlperf_backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                            int32_t i) {
  return pipeline->backend_get_input_type(backend_ptr, i);
}

// Set the data for ith input.
mlperf_status_t mlperf_backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                         int32_t batch_index, int32_t i,
                                         void *data) {
  return pipeline->backend_set_input(backend_ptr, batch_index, i, data);
}

// Return the number of outputs for the model.
int32_t mlperf_backend_get_output_count(mlperf_backend_ptr_t backend_ptr) {
  return pipeline->backend_get_output_count(backend_ptr);
}

// Return the type of ith output.
mlperf_data_t mlperf_backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                             int32_t i) {
  return pipeline->backend_get_output_type(backend_ptr, i);
}

// Get the data from ith output.
mlperf_status_t mlperf_backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                          uint32_t batch_index, int32_t i,
                                          void **data) {
  return pipeline->backend_get_output(backend_ptr, batch_index, i, data);
}

void mlperf_backend_convert_inputs(mlperf_backend_ptr_t backend_ptr, int bytes,
                                   int width, int height, uint8_t *data) {
  return pipeline->backend_convert_inputs(backend_ptr, bytes, width, height,
                                          data);
}

void *mlperf_backend_get_buffer(size_t n) {
  return pipeline->backend_get_buffer(n);
}

void mlperf_backend_release_buffer(void *p) {
  return pipeline->backend_release_buffer(p);
}

#ifdef __cplusplus
}
#endif  // __cplusplus
