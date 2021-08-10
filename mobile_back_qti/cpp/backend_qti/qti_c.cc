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

#include <fstream>
#include <unordered_map>

#include "allocator.h"
#include "cpuctrl.h"
#include "mlperf_helper.h"
#include "qti_settings.h"
#include "qti_backend_helper.h"
#include "tensorflow/core/platform/logging.h"
#include "tflite_c.h"

static QTIBackendHelper *backend_data_g = nullptr;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// Should return true if current hardware is supported.
bool mlperf_backend_matches_hardware(const char **not_allowed_message,
                                     const char **settings,
                                     const mlperf_device_info_t *device_info) {
  if (device_info && device_info->model && device_info->manufacturer) {
    LOG(INFO) << "QTI HW supported check: model: " << device_info->model
              << ", manufacturer: " << device_info->manufacturer;
  }

  std::ifstream in_file;

  *not_allowed_message = nullptr;
  bool isQSoC = CpuCtrl::isSnapDragon(device_info->manufacturer);
  LOG(INFO) << "Is QTI SOC: " << isQSoC;

  uint32_t soc_id = CpuCtrl::getSocId();
  if (isQSoC && soc_id == 0) {
    // it's a QTI SOC, but can't access soc_id
    *not_allowed_message = "Unsupported app";
    *settings = empty_settings.c_str();
    return true;
  }

  // Check if this SoC is supported

  if (isQSoC && (soc_id == SDM865 || soc_id == SDM888)) {
    // it's a QTI SOC, and the chipset is supported
    *not_allowed_message = nullptr;
    if (soc_id == SDM865) {
      *settings = qti_settings_sdm865.c_str();
    } else {
      *settings = qti_settings_sdm888.c_str();
    }
    return true;
  } else if (isQSoC) {
    // it's a QTI SOC, but the chipset is not yet supported
    *not_allowed_message = "Unsupported QTI SoC";
    *settings = empty_settings.c_str();
    return true;
  }

  // It's not a QTI SOC, so set pbData to NULL
  *settings = nullptr;
  return false;
}

// Create a new backend and return the pointer to it.
mlperf_backend_ptr_t mlperf_backend_create(
    const char *model_path, mlperf_backend_configuration_t *configs,
    const char *native_lib_path) {
  if (backend_data_g) {
    LOG(FATAL) << "Only one backend instance can be active at a time";
  }
  LOG(INFO) << "CONFIGS count = " << configs->count;
  for (int i = 0; i < configs->count; ++i) {
    LOG(INFO) << "configs->[" << configs->keys[i]
              << "] = " << configs->values[i];
  }
  backend_data_g = new QTIBackendHelper();
  QTIBackendHelper *backend_data = backend_data_g;

  process_config(configs, backend_data);

  if (backend_data->bgLoad_) {
    CpuCtrl::startLoad();
  }

  if (backend_data->is_tflite_) {
    // use highlatency cores for mobileBERT
    CpuCtrl::highLatency();
    backend_data->tflite_backend = tflite_backend_create(model_path, configs);
    backend_data->get_buffer_ = std_get_buffer;
    backend_data->release_buffer_ = std_release_buffer;
    return backend_data;
  }

  // use lowLatency cores for all snpe models
  CpuCtrl::lowLatency();

  // Use ION buffers for vision tasks
  backend_data->get_buffer_ = get_ion_buffer;
  backend_data->release_buffer_ = release_ion_buffer;

  std::stringstream adsp_lib_path;
  adsp_lib_path << native_lib_path << ";";
  adsp_lib_path << "/system/lib/rfsa/adsp;/system/vendor/lib/rfsa/adsp;/dsp";
  LOG(INFO) << "lib_path: " << adsp_lib_path.str();
  setenv("ADSP_LIBRARY_PATH", adsp_lib_path.str().c_str(), 1 /*override*/);

  // set runtime config
  backend_data->set_runtime_config();
  // Use PSNPE or SNPE
  if (backend_data->useSnpe_) {
    backend_data->use_snpe(model_path);
  } else {
    backend_data->use_psnpe(model_path);
  }

  backend_data->get_data_formats();
  backend_data->map_inputs();
  backend_data->map_outputs();

  LOG(INFO) << "SNPE build completed successfully";

  return backend_data;
}

// Return the name of this backend.
const char *mlperf_backend_name(mlperf_backend_ptr_t backend_ptr) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  return backend_data->name_;
}

// Destroy the backend pointer and its data.
void mlperf_backend_delete(mlperf_backend_ptr_t backend_ptr) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  LOG(INFO) << "Deleting Backend";
  if (backend_data->bgLoad_) {
    CpuCtrl::stopLoad();
  }
  CpuCtrl::normalLatency();
  if (backend_data->is_tflite_) {
    tflite_backend_delete(backend_data->tflite_backend);
  }
  delete backend_data;
  backend_data_g = nullptr;
}

// Run the inference for a sample.
mlperf_status_t mlperf_backend_issue_query(mlperf_backend_ptr_t backend_ptr) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  if (backend_data->is_tflite_) {
    return tflite_backend_issue_query(backend_data->tflite_backend);
  }
  if (!backend_data->useSnpe_) {
    if (!backend_data->psnpe_->execute(backend_data->inputMap_,
                                       backend_data->outputMap_)) {
      return MLPERF_FAILURE;
    }
  } else {
    if (!backend_data->snpe_->execute(backend_data->inputMap_[0],
                                      backend_data->outputMap_[0])) {
      return MLPERF_FAILURE;
    }
  }
  return MLPERF_SUCCESS;
}

// Flush the staged queries immediately.
mlperf_status_t mlperf_backend_flush_queries(mlperf_backend_ptr_t backend_ptr) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  if (backend_data->is_tflite_) {
    return tflite_backend_flush_queries(backend_data->tflite_backend);
  }
  return MLPERF_SUCCESS;
}

// Return the number of inputs of the model.
int32_t mlperf_backend_get_input_count(mlperf_backend_ptr_t backend_ptr) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  if (backend_data->is_tflite_) {
    return tflite_backend_get_input_count(backend_data->tflite_backend);
  }
  return backend_data->input_format_.size();
}

// Return the type of the ith input.
mlperf_data_t mlperf_backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                            int32_t i) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  if (backend_data->is_tflite_) {
    return tflite_backend_get_input_type(backend_data->tflite_backend, i);
  }
  return backend_data->input_format_[i];
}

// Set the data for ith input.
mlperf_status_t mlperf_backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                         int32_t batchIndex, int32_t i,
                                         void *data) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  if (backend_data->is_tflite_) {
    return tflite_backend_set_input(backend_data->tflite_backend, batchIndex, i,
                                    data);
  }
  // The inputs are in contiguous batches of backend_data->input_batch_ inputs
  if (batchIndex % backend_data->input_batch_ == 0)
    for (auto &name : backend_data->networkInputTensorNames_) {
      // If the value is a pad value for batched case then use the pointer to
      // its contiguous block
      void *batchedDataPtr = (backend_data->input_batch_ > 1)
                                 ? ChunkAllocator::GetBatchPtr(data)
                                 : data;
      if (batchedDataPtr != data) {
        LOG(INFO) << "TESTING: Using " << batchedDataPtr << " instead of "
                  << data;
      }
      backend_data->inputMap_[batchIndex / backend_data->input_batch_]
          .getUserBuffer(name)
          ->setBufferAddress(batchedDataPtr);
    }
  return MLPERF_SUCCESS;
}

// Return the number of outputs for the model.
int32_t mlperf_backend_get_output_count(mlperf_backend_ptr_t backend_ptr) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  if (backend_data->is_tflite_) {
    return tflite_backend_get_output_count(backend_data->tflite_backend);
  }
  return backend_data->output_format_.size();
}
// Return the type of ith output.
mlperf_data_t mlperf_backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                             int32_t i) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  if (backend_data->is_tflite_) {
    return tflite_backend_get_output_type(backend_data->tflite_backend, i);
  }
  return backend_data->output_format_[i];
}

// Get the data from ith output.
mlperf_status_t mlperf_backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                          uint32_t batchIndex, int32_t i,
                                          void **data) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  if (backend_data->is_tflite_) {
    return tflite_backend_get_output(backend_data->tflite_backend, batchIndex,
                                     i, data);
  }
  if (backend_data->snpe_output_layers_ ==
      "Postprocessor/BatchMultiClassNonMaxSuppression") {
    /* Reorder snpe_output_layers for coco process_output */
    std::unordered_map<int, int> mdet;
    mdet[0] = 0;
    mdet[1] = 1;
    mdet[2] = 3;
    mdet[3] = 2;
    *data = backend_data->bufs_[batchIndex]
                .at(backend_data->networkOutputTensorNames_.at(mdet.at(i)))
                .data();
    return MLPERF_SUCCESS;
  }

  *data = backend_data->bufs_[int(batchIndex / backend_data->input_batch_)]
              .at(backend_data->networkOutputTensorNames_.at(i))
              .data() +
          (batchIndex % backend_data->input_batch_) *
              int(backend_data->output_batch_bufsize_ /
                  backend_data->input_batch_) *
              sizeof(float);
  return MLPERF_SUCCESS;
}

void *mlperf_backend_get_buffer(size_t n) {
  return backend_data_g->get_buffer_(n);
}

void mlperf_backend_release_buffer(void *p) {
  backend_data_g->release_buffer_(p);
}

#ifdef __cplusplus
}
#endif  // __cplusplus
