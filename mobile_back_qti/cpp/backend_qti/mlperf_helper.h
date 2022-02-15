/* Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.

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
#ifndef MLPERFHELPER_H
#define MLPERFHELPER_H

#include "flutter/cpp/c/backend_c.h"
#include "flutter/cpp/c/type.h"
#include "qti_backend_helper.h"
#include "tensorflow/core/platform/logging.h"

static void process_config(const mlperf_backend_configuration_t *configs,
                           QTIBackendHelper *backend_data) {
  backend_data->isTflite_ = false;
  backend_data->batchSize_ = 1;
  backend_data->useSnpe_ = false;
  backend_data->perfProfile_ = zdl::DlSystem::PerformanceProfile_t::BURST;
  backend_data->loadOffTime_ = 2;
  backend_data->loadOnTime_ = 100;
  backend_data->useIonBuffers_ = true;

  std::string &delegate = backend_data->delegate_;
  delegate = configs->accelerator;
  if ((delegate == "gpu_f16") || (delegate == "nnapi_qti-dsp") ||
      (delegate == "nnapi_qti-gpu")) {
    backend_data->isTflite_ = true;
  } else if (strncmp(configs->accelerator, "snpe", 4) == 0) {
    backend_data->useSnpe_ = true;
  } else if (strncmp(configs->accelerator, "psnpe", 5) == 0) {
    backend_data->useSnpe_ = false;
  } else {
    LOG(FATAL) << "Error: Unsupported delegate " << delegate;
  }

  // Batch size is zero if not specified
  backend_data->batchSize_ =
      (configs->batch_size == 0) ? 1 : configs->batch_size;

  // Handle custom settings
  std::string perfProfile = "burst";
  for (int i = 0; i < configs->count; ++i) {
    if (strcmp(configs->keys[i], "scenario") == 0) {
      backend_data->scenario_ = configs->values[i];
    } else if (strcmp(configs->keys[i], "snpe_output_layers") == 0) {
      backend_data->snpeOutputLayers_ = configs->values[i];
    } else if (strcmp(configs->keys[i], "bg_load") == 0) {
      backend_data->bgLoad_ = true;
    } else if (strcmp(configs->keys[i], "load_off_time") == 0) {
      backend_data->loadOffTime_ = atoi(configs->values[i]);
    } else if (strcmp(configs->keys[i], "load_on_time") == 0) {
      backend_data->loadOnTime_ = atoi(configs->values[i]);
    } else if (strcmp(configs->keys[i], "input_buffer_type") == 0) {
      if (std::strcmp(configs->values[i], "float_32") == 0) {
        backend_data->inputBufferType_ =
            QTIBackendHelper::QTIBufferType::FLOAT_32;
      } else {
        backend_data->inputBufferType_ =
            QTIBackendHelper::QTIBufferType::UINT_8;
      }
    } else if (strcmp(configs->keys[i], "output_buffer_type") == 0) {
      if (std::strcmp(configs->values[i], "float_32") == 0) {
        backend_data->outputBufferType_ =
            QTIBackendHelper::QTIBufferType::FLOAT_32;
      } else {
        backend_data->outputBufferType_ =
            QTIBackendHelper::QTIBufferType::UINT_8;
      }
    } else if (strcmp(configs->keys[i], "use_ion_buffer") == 0) {
      if (std::strcmp(configs->values[i], "true") == 0) {
        backend_data->useIonBuffers_ = true;
      } else {
        backend_data->useIonBuffers_ = false;
      }
    } else if (strcmp(configs->keys[i], "perf_profile") == 0) {
      perfProfile = configs->values[i];
      if ((std::strcmp(configs->values[i], "default") == 0) ||
          (std::strcmp(configs->values[i], "balanced") == 0)) {
        backend_data->perfProfile_ =
            zdl::DlSystem::PerformanceProfile_t::BALANCED;
      } else if (std::strcmp(configs->values[i], "high_performance") == 0) {
        backend_data->perfProfile_ =
            zdl::DlSystem::PerformanceProfile_t::HIGH_PERFORMANCE;
      } else if (std::strcmp(configs->values[i], "power_saver") == 0) {
        backend_data->perfProfile_ =
            zdl::DlSystem::PerformanceProfile_t::POWER_SAVER;
      } else if (std::strcmp(configs->values[i], "system_settings") == 0) {
        backend_data->perfProfile_ =
            zdl::DlSystem::PerformanceProfile_t::SYSTEM_SETTINGS;
      } else if (std::strcmp(configs->values[i],
                             "sustained_high_performance") == 0) {
        backend_data->perfProfile_ =
            zdl::DlSystem::PerformanceProfile_t::SUSTAINED_HIGH_PERFORMANCE;
      } else if (std::strcmp(configs->values[i], "burst") == 0) {
        backend_data->perfProfile_ = zdl::DlSystem::PerformanceProfile_t::BURST;
      } else if (std::strcmp(configs->values[i], "low_power_saver") == 0) {
        backend_data->perfProfile_ =
            zdl::DlSystem::PerformanceProfile_t::LOW_POWER_SAVER;
      } else if (std::strcmp(configs->values[i], "high_power_saver") == 0) {
        backend_data->perfProfile_ =
            zdl::DlSystem::PerformanceProfile_t::HIGH_POWER_SAVER;
      } else if (std::strcmp(configs->values[i], "low_balanced") == 0) {
        backend_data->perfProfile_ =
            zdl::DlSystem::PerformanceProfile_t::LOW_BALANCED;
      } else {
        LOG(INFO) << "Unrecognized performance profile: " << perfProfile;
        backend_data->perfProfile_ = zdl::DlSystem::PerformanceProfile_t::BURST;
        perfProfile = "burst";
      }
    }
  }

  LOG(INFO) << "Config: delegate: " << delegate
            << " | scenario: " << backend_data->scenario_
            << " | output: " << backend_data->snpeOutputLayers_
            << " | isTfLite: " << backend_data->isTflite_
            << " | batchSize: " << backend_data->batchSize_
            << " | useSNPE: " << backend_data->useSnpe_
            << " | bgLoad: " << backend_data->bgLoad_
            << " | loadOffTime: " << backend_data->loadOffTime_
            << " | loadOnTime_: " << backend_data->loadOnTime_
            << " | inputBufferType: " << backend_data->inputBufferType_
            << " | outputBufferType: " << backend_data->outputBufferType_
            << " | perfProfile: " << perfProfile
            << " | useIonBuffer: " << backend_data->useIonBuffers_;
}

#endif
