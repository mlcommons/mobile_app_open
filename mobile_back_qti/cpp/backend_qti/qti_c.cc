/* Copyright (c) 2020-2025 Qualcomm Innovation Center, Inc. All rights reserved.

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

#include <fstream>
#include <unordered_map>

#include "Executor.h"
#include "allocator.h"
#include "cpuctrl.h"
#include "mlperf_helper.h"
#include "qtiBackendHelper.h"
#include "qti_settings.h"
#include "soc_utility.h"
#include "tensorflow/core/platform/logging.h"

#ifdef DEBUG_FLAG
#include <chrono>
using namespace std::chrono;
#endif

static qtiBackendHelper *backend_data_ = nullptr;

bool useIonBuffer_g;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

bool set_system_paths(const char *native_lib_path) {
#ifdef __ANDROID__
  std::stringstream adsp_lib_path;
  adsp_lib_path << native_lib_path << ";";
  adsp_lib_path << "/system/lib/rfsa/adsp;/system/vendor/lib/rfsa/adsp;/dsp";
  LOG(INFO) << "adsp_lib_path: " << adsp_lib_path.str();
  setenv("ADSP_LIBRARY_PATH", adsp_lib_path.str().c_str(), 1 /*override*/);
  std::stringstream ld_lib_path;
  ld_lib_path << native_lib_path << ";";
  ld_lib_path << "/system/vendor/lib64";
  LOG(INFO) << "ld_lib_path: " << ld_lib_path.str();
  setenv("LD_LIBRARY_PATH", ld_lib_path.str().c_str(), 1 /*override*/);
#endif

  return false;
}

// Should return true if current hardware is supported.
bool mlperf_backend_matches_hardware(const char **not_allowed_message,
                                     const char **settings,
                                     const mlperf_device_info_t *device_info) {
  if (device_info && device_info->model && device_info->manufacturer) {
    LOG(INFO) << "QTI HW supported check: model: " << device_info->model
              << ", manufacturer: " << device_info->manufacturer;
  }

  std::ifstream in_file;
  set_system_paths(device_info->native_lib_path);

  *not_allowed_message = nullptr;
  bool isQSoC = Socs::isSnapDragon(device_info->manufacturer);
  LOG(INFO) << "Is QTI SOC: " << isQSoC;
  if (isQSoC) {
    return Socs::soc_settings(settings, not_allowed_message);
  }

  // It's not a QTI SOC, so set pbData to NULL
  *settings = nullptr;
  return false;
}

// Create a new backend and return the pointer to it.
mlperf_backend_ptr_t mlperf_backend_create(
    const char *model_path, mlperf_backend_configuration_t *configs,
    const char *native_lib_path) {
  if (backend_data_) {
    LOG(FATAL) << "Only one backend instance can be active at a time";
  }
  LOG(INFO) << "CONFIGS count = " << configs->count;
  for (int i = 0; i < configs->count; ++i) {
    LOG(INFO) << "configs->[" << configs->keys[i]
              << "] = " << configs->values[i];
  }

  backend_data_ = new qtiBackendHelper();
  qtiBackendHelper *backend_data = backend_data_;

  process_config_framework(configs, backend_data);
  backend_data->setBackend();
  backend_data->setConfigs(configs);

  useIonBuffer_g = backend_data->getUseIonBuffers_();

  CpuCtrl::init();

  if (backend_data->bgLoad_) {
    CpuCtrl::startLoad(backend_data->loadOffTime_, backend_data->loadOnTime_);
  }

  // use high latency cores for SD8EliteG5 and low latency cores for other
  // devices
  if (Socs::get_soc_name() == "SD8EliteG5") {
    CpuCtrl::highLatency();
  } else {
    CpuCtrl::lowLatency();
  }
  set_system_paths(native_lib_path);

  backend_data->create(model_path, native_lib_path);
  backend_data->queryCount_ = 0;

  return backend_data;
}

// Return the name of the accelerator.
const char *mlperf_backend_accelerator_name(mlperf_backend_ptr_t backend_ptr) {
  qtiBackendHelper *backend_data = (qtiBackendHelper *)backend_ptr;
  return backend_data->acceleratorName_;
}

// Return the name of this backend.
const char *mlperf_backend_name(mlperf_backend_ptr_t backend_ptr) {
  qtiBackendHelper *backend_data = (qtiBackendHelper *)backend_ptr;
  return backend_data->get_name_();
}

// Return the vendor name of this backend
const char *mlperf_backend_vendor_name(mlperf_backend_ptr_t backend_ptr) {
  return "QTI";
}

// Destroy the backend pointer and its data.
void mlperf_backend_delete(mlperf_backend_ptr_t backend_ptr) {
  qtiBackendHelper *backend_data = (qtiBackendHelper *)backend_ptr;
  LOG(INFO) << "Deleting Backend";
  if (backend_data->bgLoad_) {
    CpuCtrl::stopLoad();
  }
  CpuCtrl::normalLatency();
  delete backend_data;
  backend_data_ = nullptr;
}

// Run the inference for a sample.
mlperf_status_t mlperf_backend_issue_query(mlperf_backend_ptr_t backend_ptr,
                                           ft_callback callback,
                                           void *context) {
  mlperf_status_t ret = MLPERF_FAILURE;
  qtiBackendHelper *backend_data = (qtiBackendHelper *)backend_ptr;
#ifdef DEBUG_FLAG
  LOG(INFO) << "Query cnt: " << backend_data->queryCount_;
  auto start = high_resolution_clock::now();
#endif

  ret = backend_data->execute(callback, context);

#ifdef DEBUG_FLAG
  auto end = high_resolution_clock::now();
  auto duration = duration_cast<microseconds>(end - start);
  LOG(INFO) << "Inference Time(ms): " << duration.count();
#endif

  backend_data->queryCount_ = backend_data->queryCount_ + 1;
  return ret;
}

// Flush the staged queries immediately.
mlperf_status_t mlperf_backend_flush_queries(mlperf_backend_ptr_t backend_ptr) {
  qtiBackendHelper *backend_data = (qtiBackendHelper *)backend_ptr;
  backend_data->flush();
  return MLPERF_SUCCESS;
}

// Return the number of inputs of the model.
int32_t mlperf_backend_get_input_count(mlperf_backend_ptr_t backend_ptr) {
  qtiBackendHelper *backend_data = (qtiBackendHelper *)backend_ptr;
  return backend_data->getInputFormat_().size();
}

// Return the type of the ith input.
mlperf_data_t mlperf_backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                            int32_t i) {
  qtiBackendHelper *backend_data = (qtiBackendHelper *)backend_ptr;
  return backend_data->getInputFormat_()[i];
}

// Set the data for ith input.
mlperf_status_t mlperf_backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                         int32_t batchIndex, int32_t i,
                                         void *data) {
  qtiBackendHelper *backend_data = (qtiBackendHelper *)backend_ptr;
  return backend_data->set_input(batchIndex, i, data);
}

// Return the number of outputs for the model.
int32_t mlperf_backend_get_output_count(mlperf_backend_ptr_t backend_ptr) {
  qtiBackendHelper *backend_data = (qtiBackendHelper *)backend_ptr;
  return backend_data->getOutputFormat_().size();
}
// Return the type of ith output.
mlperf_data_t mlperf_backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                             int32_t i) {
  qtiBackendHelper *backend_data = (qtiBackendHelper *)backend_ptr;
  return backend_data->getOutputFormat_()[i];
}

// Get the data from ith output.
mlperf_status_t mlperf_backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                          uint32_t batchIndex,
                                          int32_t outputIndex, void **data) {
  qtiBackendHelper *backend_data = (qtiBackendHelper *)backend_ptr;
  return backend_data->get_output(batchIndex, outputIndex, data);
}

void *mlperf_backend_get_buffer(size_t n) {
  if (backend_data_ == nullptr)
    LOG(FATAL) << "Cannot proceed. Backend ptr is NULL";

  return backend_data_->getBuffer(n);
}

void mlperf_backend_release_buffer(void *p) { backend_data_->deregister(p); }

#ifdef __cplusplus
}
#endif  // __cplusplus
