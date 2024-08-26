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
#include "tflite_settings_android.h"
#include "tflite_settings_apple.h"
#include "tflite_settings_windows.h"

#if __ANDROID__
#include <sys/system_properties.h>
#endif

#if MTK_TFLITE_NEURON_BACKEND
#include "neuron/neuron_backend.h"
#include "neuron/neuron_builder.h"
#include "neuron/tflite_settings_mtk.h"
#endif

#if MTK_TFLITE_NEURON_BACKEND
std::string GetPlatformName();
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

// TFLite is the standard backend for all hardware.
bool mlperf_backend_matches_hardware(const char **not_allowed_message,
                                     const char **settings,
                                     const mlperf_device_info_t *device_info) {
  *not_allowed_message = nullptr;
#if MTK_TFLITE_NEURON_BACKEND && defined(__ANDROID__)
  std::string device = GetPlatformName();
  *settings = tflite_settings_mtk.c_str();
  if (device == "mt6989") {
    *settings = tflite_settings_mtk_mt6989.c_str();
  }
  return neuron_tflite_backend(not_allowed_message, device_info);
#elif __APPLE__
  tflite_settings_apple = tflite_settings_apple_main;
  std::string model = device_info->model;
  int modelNumber = 0;

  // iphone model is written as "iPhone14,4" or similar
  // check that model value has 'iPhone' prefix
  std::string prefix = "iPhone";
  if (!model.compare(0, prefix.size(), prefix)) {
    modelNumber = std::stoi(model.substr(prefix.size()));
  }
  switch (modelNumber) {
    case 11:
      // iPhone X series
      tflite_settings_apple += tflite_settings_apple_iphoneX;
      break;
    case 12:
      // iPhone 11 series
      tflite_settings_apple += tflite_settings_apple_iphone11;
      break;
    case 13:
      // iPhone 12 series
      tflite_settings_apple += tflite_settings_apple_iphone12;
      break;
    default:
      // use iPhone 12 settings for unknown devices
      tflite_settings_apple += tflite_settings_apple_iphone12;
      break;
  }
  *settings = tflite_settings_apple.c_str();
#elif defined(_WIN64) || defined(_WIN32)
  *settings = tflite_settings_windows.c_str();
#else
  *settings = tflite_settings_android.c_str();
#endif
  LOG(INFO) << "TFLite backend matches hardware";
  return true;
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
mlperf_status_t mlperf_backend_issue_query(mlperf_backend_ptr_t backend_ptr) {
  return pipeline->backend_issue_query(backend_ptr);
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
