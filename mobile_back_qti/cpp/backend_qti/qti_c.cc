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
#include "qti_backend_helper.h"
#include "qti_settings.h"
#include "tensorflow/core/platform/logging.h"
#include "tflite_c.h"

#define xverstr(a) verstr(a)
#define verstr(a) #a

#ifndef SNPE_VERSION_STRING
#define SNPE_VERSION_STRING "default"
#endif

static QTIBackendHelper *backend_data_ = nullptr;

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
  if (isQSoC) {
    switch (soc_id) {
      // it's a QTI SOC, and the chipset is supported
      *not_allowed_message = nullptr;
      case SDM865:
        *settings = qti_settings_sdm865.c_str();
        break;
      case SDM888:
        *settings = qti_settings_sdm888.c_str();
        break;
      case SDM778:
        *settings = qti_settings_sdm778.c_str();
        break;
	  case SD8G1:
        *settings = qti_settings_sd8g1.c_str();
	  	break;
      default:
        // it's a QTI SOC, but the chipset is not yet supported
        *not_allowed_message = "Unsupported QTI SoC";
        *settings = empty_settings.c_str();
        break;
    }
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
  if (backend_data_) {
    LOG(FATAL) << "Only one backend instance can be active at a time";
  }
  LOG(INFO) << "CONFIGS count = " << configs->count;
  for (int i = 0; i < configs->count; ++i) {
    LOG(INFO) << "configs->[" << configs->keys[i]
              << "] = " << configs->values[i];
  }
  backend_data_ = new QTIBackendHelper();
  QTIBackendHelper *backend_data = backend_data_;

  process_config(configs, backend_data);

  if (backend_data->bgLoad_) {
    CpuCtrl::startLoad(backend_data->loadOffTime_, backend_data->loadOnTime_);
  }

  if (backend_data->isTflite_) {
    CpuCtrl::highLatency();
    backend_data->tfliteBackend_ = tflite_backend_create(model_path, configs);
    backend_data->getBuffer_ = std_get_buffer;
    backend_data->releaseBuffer_ = std_release_buffer;
    return backend_data;
  }

  // use lowLatency cores for all snpe models
  CpuCtrl::lowLatency();

  // Set buffers to be used for snpe
  if (backend_data->useIonBuffers_) {
    backend_data->getBuffer_ = get_ion_buffer;
    backend_data->releaseBuffer_ = release_ion_buffer;
  } else {
    backend_data->getBuffer_ = std_get_buffer;
    backend_data->releaseBuffer_ = std_release_buffer;
  }

  std::stringstream adsp_lib_path;
  adsp_lib_path << native_lib_path << ";";
  adsp_lib_path << "/system/lib/rfsa/adsp;/system/vendor/lib/rfsa/adsp;/dsp";
  LOG(INFO) << "lib_path: " << adsp_lib_path.str();
  setenv("ADSP_LIBRARY_PATH", adsp_lib_path.str().c_str(), 1 /*override*/);
  std::string snpe_version = xverstr(SNPE_VERSION_STRING);
  if (snpe_version.compare("default") != 0){
    int dotPosition = snpe_version.find_last_of(".");
    snpe_version = snpe_version.substr(dotPosition+1);
  }

  if (snpe_version.compare(backend_data->get_snpe_version()) != 0) {
    LOG(FATAL) << "Snpe libs modified. expected: " << snpe_version << " found: " << backend_data->get_snpe_version();
  }
  LOG(INFO) << "snpe_version: " <<  snpe_version;

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

// TODO: Return the name of the accelerator.
const char *mlperf_backend_accelerator_name(mlperf_backend_ptr_t backend_ptr) {
  return "ACCELERATOR_NAME";
}

// Return the name of this backend.
const char *mlperf_backend_name(mlperf_backend_ptr_t backend_ptr) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  return backend_data->name_;
}

// Return the vendor name of this backend
const char *mlperf_backend_vendor_name(mlperf_backend_ptr_t backend_ptr) {
  return "QTI";
}

// Destroy the backend pointer and its data.
void mlperf_backend_delete(mlperf_backend_ptr_t backend_ptr) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  LOG(INFO) << "Deleting Backend";
  if (backend_data->bgLoad_) {
    CpuCtrl::stopLoad();
  }
  CpuCtrl::normalLatency();
  if (backend_data->isTflite_) {
    tflite_backend_delete(backend_data->tfliteBackend_);
  }
  delete backend_data;
  backend_data_ = nullptr;
}

// Run the inference for a sample.
mlperf_status_t mlperf_backend_issue_query(mlperf_backend_ptr_t backend_ptr) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  if (backend_data->isTflite_) {
    return tflite_backend_issue_query(backend_data->tfliteBackend_);
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
  if (backend_data->isTflite_) {
    return tflite_backend_flush_queries(backend_data->tfliteBackend_);
  }
  return MLPERF_SUCCESS;
}

// Return the number of inputs of the model.
int32_t mlperf_backend_get_input_count(mlperf_backend_ptr_t backend_ptr) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  if (backend_data->isTflite_) {
    return tflite_backend_get_input_count(backend_data->tfliteBackend_);
  }
  return backend_data->inputFormat_.size();
}

// Return the type of the ith input.
mlperf_data_t mlperf_backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                            int32_t i) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  if (backend_data->isTflite_) {
    return tflite_backend_get_input_type(backend_data->tfliteBackend_, i);
  }
  return backend_data->inputFormat_[i];
}

// Set the data for ith input.
mlperf_status_t mlperf_backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                         int32_t batchIndex, int32_t i,
                                         void *data) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  if (backend_data->isTflite_) {
    return tflite_backend_set_input(backend_data->tfliteBackend_, batchIndex, i,
                                    data);
  }
  // The inputs are in contiguous batches of backend_data->inputBatch_ inputs
  // If the value is a pad value for batched case then use the pointer to
  // its contiguous block
  void *batchedDataPtr = (backend_data->inputBatch_ > 1)
                             ? ChunkAllocator::GetBatchPtr(data)
                             : data;
  // if (batchedDataPtr != data) {
  //   LOG(INFO) << "TESTING: Using " << batchedDataPtr << " instead of " <<
  //   data;
  // }
  backend_data->inputMap_[batchIndex / backend_data->inputBatch_]
      .getUserBuffer(backend_data->networkInputTensorNames_.at(i))
      ->setBufferAddress(batchedDataPtr);
  return MLPERF_SUCCESS;
}

// Return the number of outputs for the model.
int32_t mlperf_backend_get_output_count(mlperf_backend_ptr_t backend_ptr) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  if (backend_data->isTflite_) {
    return tflite_backend_get_output_count(backend_data->tfliteBackend_);
  }
  return backend_data->outputFormat_.size();
}
// Return the type of ith output.
mlperf_data_t mlperf_backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                             int32_t i) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  if (backend_data->isTflite_) {
    return tflite_backend_get_output_type(backend_data->tfliteBackend_, i);
  }
  return backend_data->outputFormat_[i];
}

// Get the data from ith output.
mlperf_status_t mlperf_backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                          uint32_t batchIndex,
                                          int32_t outputIndex, void **data) {
  QTIBackendHelper *backend_data = (QTIBackendHelper *)backend_ptr;
  if (backend_data->isTflite_) {
    return tflite_backend_get_output(backend_data->tfliteBackend_, batchIndex,
                                     outputIndex, data);
  }
  if (backend_data->snpeOutputLayers_ ==
      "Postprocessor/BatchMultiClassNonMaxSuppression") {
    // Reorder snpeOutputLayers_ for coco process_output
    std::unordered_map<int, std::string> mapIndexLayer;
    mapIndexLayer[0] = "boxes";
    mapIndexLayer[1] = "classes";
    mapIndexLayer[2] = "scores";
    mapIndexLayer[3] = "num_detections";
    const char *outputLayerName;

    for (int idx = 0; idx < backend_data->networkOutputTensorNames_.size();
         idx++) {
      if (strstr(backend_data->networkOutputTensorNames_.at(idx),
                 mapIndexLayer[outputIndex].c_str())) {
        // layer name found
        outputLayerName = backend_data->networkOutputTensorNames_.at(idx);
        break;
      }
    }

    *data = backend_data->bufs_[batchIndex].at(outputLayerName).data();
    return MLPERF_SUCCESS;
  } else if (backend_data->snpeOutputLayers_ == "transpose") {
    *data = backend_data->bufs_[int(batchIndex / backend_data->inputBatch_)]
                .at(backend_data->networkOutputTensorNames_.at(0))
                .data() +
            (1 - outputIndex) * 384 * sizeof(float);
    return MLPERF_SUCCESS;
  }
  size_t size = sizeof(float);
  if (backend_data->outputBufferType_ ==
      QTIBackendHelper::QTIBufferType::UINT_8) {
    size = sizeof(uint8_t);
  }

  *data =
      backend_data->bufs_[int(batchIndex / backend_data->inputBatch_)]
          .at(backend_data->networkOutputTensorNames_.at(outputIndex))
          .data() +
      (batchIndex % backend_data->inputBatch_) *
          int(backend_data->outputBatchBufsize_ / backend_data->inputBatch_) *
          size;
  return MLPERF_SUCCESS;
}

void *mlperf_backend_get_buffer(size_t n) {
  return backend_data_->getBuffer_(n);
}

void mlperf_backend_release_buffer(void *p) {
  backend_data_->releaseBuffer_(p);
}

#ifdef __cplusplus
}
#endif  // __cplusplus
