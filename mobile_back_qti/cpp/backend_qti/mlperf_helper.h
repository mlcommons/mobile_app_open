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
#ifndef MLPERFHELPER_H
#define MLPERFHELPER_H

#include "flutter/cpp/c/backend_c.h"
#include "flutter/cpp/c/type.h"
#include "qtiBackendHelper.h"
#include "tensorflow/core/platform/logging.h"

static void process_config_framework(
    const mlperf_backend_configuration_t *configs,
    qtiBackendHelper *backend_data) {
  /* this function is meant to determine only the framework type.
   * As soon as it determines it, it returns from this function */

  for (int i = 0; i < configs->count; ++i) {
    if (strcmp(configs->keys[i], "pipeline") == 0) {
      if (std::strcmp(configs->values[i], "StableDiffusionPipeline") == 0) {
        backend_data->backend_type_ = QTI_BACKEND_TYPE_STABLE_DIFFUSION;
        return;
      }
      if (std::strcmp(configs->values[i], "GeniePipeline") == 0) {
        backend_data->backend_type_ = QTI_BACKEND_TYPE_GENIE;
        return;
      }
    }
  }
  std::string delegate = configs->accelerator;
  if ((delegate == "gpu_f16") || (delegate == "nnapi_qti-dsp") ||
      (delegate == "nnapi_qti-gpu")) {
    backend_data->backend_type_ = QTI_BACKEND_TYPE_TFLITE;
  } else if (strncmp(configs->accelerator, "snpe", 4) == 0) {
    backend_data->backend_type_ = QTI_BACKEND_TYPE_SNPE;
  } else if (strncmp(configs->accelerator, "psnpe", 5) == 0) {
    backend_data->backend_type_ = QTI_BACKEND_TYPE_PSNPE;
  } else {
    LOG(FATAL) << "Error: Unsupported delegate " << delegate;
  }
}

#endif
