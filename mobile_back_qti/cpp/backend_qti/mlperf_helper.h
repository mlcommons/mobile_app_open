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

#include "android/cpp/c/backend_c.h"
#include "android/cpp/c/type.h"
#include "qti_backend_helper.h"
#include "tensorflow/core/platform/logging.h"

static void process_config(const mlperf_backend_configuration_t *configs,
                           QTIBackendHelper *backend_data) {
  backend_data->is_tflite_ = false;
  backend_data->batchSize_ = 1;
  backend_data->useSnpe_ = false;

  std::string &delegate = backend_data->delegate_;
  delegate = configs->accelerator;
  if ((delegate == "gpu_f16") || (delegate == "nnapi-qti-dsp") ||
      (delegate == "nnapi-qti-gpu")) {
    backend_data->is_tflite_ = true;
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
  for (int i = 0; i < configs->count; ++i) {
    if (strcmp(configs->keys[i], "scenario") == 0) {
      backend_data->scenario_ = configs->values[i];
    } else if (strcmp(configs->keys[i], "snpeOutputLayers") == 0) {
      backend_data->snpe_output_layers_ = configs->values[i];
    } else if (strcmp(configs->keys[i], "bgLoad") == 0) {
      backend_data->bgLoad_ = true;
    }
  }

  LOG(INFO) << "Config: delegate: " << delegate
            << " scenario: " << backend_data->scenario_
            << " output: " << backend_data->snpe_output_layers_
            << " isTfLite: " << backend_data->is_tflite_
            << " batchSize: " << backend_data->batchSize_
            << " useSNPE: " << backend_data->useSnpe_
            << " bgLoad: " << backend_data->bgLoad_;
}

#endif
