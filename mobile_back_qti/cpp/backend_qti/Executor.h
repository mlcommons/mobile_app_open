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

#ifndef MOBILE_APP_OPEN_EXECUTOR_H
#define MOBILE_APP_OPEN_EXECUTOR_H

#include <memory>
#include <string>
#include <unordered_map>

#include "DlSystem/StringList.h"
#include "SNPE/PSNPE.h"
#include "SNPE/SNPE.h"
#include "allocator.h"
#include "cpuctrl.h"
#include "flutter/cpp/c/type.h"
#include "soc_utility.h"

class Executor {
 public:
  enum QTIBufferType { FLOAT_32 = 0, UINT_8 = 1, INT_32 = 2 };
  Executor(){};
  virtual ~Executor(){};

  virtual void create(const char* model_path,
                      const char* native_model_path = "") = 0;
  virtual void flush() {
  }  // called after each query; override for per-query reset
  virtual mlperf_status_t execute(ft_callback callback = nullptr,
                                  void* context = nullptr) = 0;
  virtual mlperf_status_t set_input(int32_t, int32_t, void*) = 0;
  virtual mlperf_status_t get_output(uint32_t, int32_t, void**) = 0;
  virtual void* getBuffer(size_t) = 0;
  virtual void deregister(void*) = 0;

  // Get Functions
  virtual const char* get_name_() const = 0;
  virtual bool getUseIonBuffers_() const = 0;
  virtual std::vector<mlperf_data_t> getInputFormat_() const = 0;
  virtual std::vector<mlperf_data_t> getOutputFormat_() const = 0;

  virtual void setConfigs(const mlperf_backend_configuration_t* configs) = 0;
};

#endif  // MOBILE_APP_OPEN_EXECUTOR_H
