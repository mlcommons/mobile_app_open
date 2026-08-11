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

#ifndef MOBILE_APP_OPEN_SDEXECUTOR_H
#define MOBILE_APP_OPEN_SDEXECUTOR_H

#include <string.h>

#include "Executor.h"
#include "backend_utils.h"
#include "soc_utility.h"

#ifdef STABLEDIFFUSION_FLAG
#include "StableDiffusionShared/include/QnnApiHelpers.hpp"
#endif

class SdExecutor : public Executor {
 public:
  const char* name_ = "QTI-SD";

#ifdef STABLEDIFFUSION_FLAG
  QnnApiHelpers* sd_pipeline;
#else
  void* sd_pipeline;
#endif

  // Stable Diffusion specific parameters
  int num_steps = 20;
  int seed = 633994880;
  float guidance_scale = 7.5f;

  // Path variables
  std::string native_lib_path;
  std::string data_folder_path;

  /* fixed input and output data formats */
  std::vector<mlperf_data_t> inputFormat_;
  std::vector<mlperf_data_t> outputFormat_;
  QTIBufferType inputBufferType_ = QTIBufferType::UINT_8;
  QTIBufferType outputBufferType_ = QTIBufferType::FLOAT_32;

  // Key functions
  void create(const char* model_path,
              const char* native_model_path = "") override;
  mlperf_status_t execute(ft_callback callback = nullptr,
                          void* context = nullptr) override;
  mlperf_status_t set_input(int32_t batchIndex, int32_t inputIndex,
                            void* data) override;
  mlperf_status_t get_output(uint32_t batchIndex, int32_t outputIndex,
                             void** data) override;
  void* getBuffer(size_t n) override;
  void deregister(void* p) override;

  // Get functions
  const char* get_name_() const override;
  bool getUseIonBuffers_() const override;
  std::vector<mlperf_data_t> getInputFormat_() const override;
  std::vector<mlperf_data_t> getOutputFormat_() const override;

  // setting configs
  void setConfigs(const mlperf_backend_configuration_t* configs) override;

  // Constructor and destructor
  SdExecutor() = default;
  ~SdExecutor() override {
#ifdef STABLEDIFFUSION_FLAG
    if (sd_pipeline) {
      delete sd_pipeline;
      sd_pipeline = nullptr;
    }
#endif
  }
};
#endif  // MOBILE_APP_OPEN_SDEXECUTOR_H