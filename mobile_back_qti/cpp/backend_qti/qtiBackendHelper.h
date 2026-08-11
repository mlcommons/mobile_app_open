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

#ifndef MOBILE_APP_OPEN_QTIBACKENDHELPER_H
#define MOBILE_APP_OPEN_QTIBACKENDHELPER_H

#include "BackendFactory.h"
#include "Executor.h"
#include "flutter/cpp/c/backend_c.h"

typedef enum {
  QTI_BACKEND_TYPE_UNKNOWN = 0,
  QTI_BACKEND_TYPE_TFLITE,
  QTI_BACKEND_TYPE_SNPE,
  QTI_BACKEND_TYPE_PSNPE,
  QTI_BACKEND_TYPE_QNN,
  QTI_BACKEND_TYPE_QAIRT,
  QTI_BACKEND_TYPE_STABLE_DIFFUSION,
  QTI_BACKEND_TYPE_GENIE,
} qti_backend_type_t;

class qtiBackendHelper {
 public:
  qtiBackendHelper() {}
  ~qtiBackendHelper() {}

  qti_backend_type_t backend_type_ = QTI_BACKEND_TYPE_UNKNOWN;

  const char* acceleratorName_;
  bool bgLoad_ = false;
  int queryCount_;
  uint32_t loadOffTime_ = 2;
  uint32_t loadOnTime_ = 100;
  std::unique_ptr<Executor> m_executor;

  // Key functions
  void create(const char* model_path, const char* native_lib_path = "");
  void setBackend();
  void flush();
  mlperf_status_t set_input(int32_t, int32_t, void*);
  mlperf_status_t execute(ft_callback callback = nullptr,
                          void* context = nullptr);
  void* getBuffer(size_t n);
  mlperf_status_t get_output(uint32_t, int32_t, void**);
  void deregister(void*);

  // Get functions
  const char* get_name_() const;
  bool getUseIonBuffers_() const;
  std::vector<mlperf_data_t> getInputFormat_() const;
  std::vector<mlperf_data_t> getOutputFormat_() const;

  void setExecutor(std::unique_ptr<Executor> execute_handler);

  void setConfigs(const mlperf_backend_configuration_t* configs);
};

#endif  // MOBILE_APP_OPEN_QTIBACKENDHELPER_H
