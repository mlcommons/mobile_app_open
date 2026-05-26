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

#ifndef MOBILE_APP_OPEN_GENIEEXECUTOR_H
#define MOBILE_APP_OPEN_GENIEEXECUTOR_H

#include <string>
#include <vector>

#include "Executor.h"
#include "backend_utils.h"
#include "soc_utility.h"

#ifdef GENIE_FLAG
#include "Genie/GenieCommon.h"
#include "Genie/GenieDialog.h"
#endif

class GenieExecutor : public Executor {
 public:
  const char* name_ = "QTI-Genie";

#ifdef GENIE_FLAG
  GenieDialog_Handle_t dialogHandle_ = nullptr;
#endif

  // I/O state — static output buffer matches NativeQNN g_output_data_ pattern
  static std::vector<uint32_t> outputTokens_;
  const std::vector<int>* inputTokens_ =
      nullptr;  // points into dataset sample, valid for query duration

  // Config (set via setConfigs before create is called)
  std::string modelFolder_;
  std::string deviceType_ = "HTP";
  uint32_t maxNewTokens_ = 512;
  uint32_t contextSize_ = 4096;

  // Fixed I/O formats
  std::vector<mlperf_data_t> inputFormat_;
  std::vector<mlperf_data_t> outputFormat_;

  // Executor interface
  void create(const char* model_path,
              const char* native_lib_path = "") override;
  void flush() override;
  mlperf_status_t execute(ft_callback callback = nullptr,
                          void* context = nullptr) override;
  mlperf_status_t set_input(int32_t batchIndex, int32_t inputIndex,
                            void* data) override;
  mlperf_status_t get_output(uint32_t batchIndex, int32_t outputIndex,
                             void** data) override;
  void* getBuffer(size_t n) override;
  void deregister(void* p) override;

  const char* get_name_() const override;
  bool getUseIonBuffers_() const override;
  std::vector<mlperf_data_t> getInputFormat_() const override;
  std::vector<mlperf_data_t> getOutputFormat_() const override;
  void setConfigs(const mlperf_backend_configuration_t* configs) override;

  // Static token callback — matches NativeQNN tokenToTokenCallback pattern
#ifdef GENIE_FLAG
  static void tokenToTokenCallback(const uint32_t* token, uint32_t tokensLength,
                                   GenieDialog_SentenceCode_t sentenceCode,
                                   const void* userData);
#endif

  // First-token callback state (static so accessible from static callback)
  static ft_callback ftCallback_;
  static void* ftContext_;
  static bool ftFired_;

  GenieExecutor() = default;
  ~GenieExecutor() override;

 private:
  std::string buildConfigJson(const std::string& modelFolder,
                              const std::string& deviceType, uint32_t maxTokens,
                              uint32_t contextSize);
};

#endif  // MOBILE_APP_OPEN_GENIEEXECUTOR_H
