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

#ifndef MOBILE_APP_OPEN_PSNPEEXECUTOR_H
#define MOBILE_APP_OPEN_PSNPEEXECUTOR_H

#include "SNPE/PSNPE.h"
#include "SnpeExecutor.h"
#include "allocator.h"
#include "backend_utils.h"

class psnpe_handler {
 public:
  Snpe_PSNPE_Handle_t psnpeHandle;

  psnpe_handler() { psnpeHandle = Snpe_PSNPE_Create(); }

  ~psnpe_handler() { Snpe_PSNPE_Delete(psnpeHandle); }
};

class PsnpeExecutor : public SnpeExecutor {
 public:
  const char* name_ = "psnpe";
  std::unique_ptr<psnpe_handler> psnpe_;

  void Init(const char* model_path) override;
  mlperf_status_t execute(ft_callback callback = nullptr,
                          void* context = nullptr) override;
  void* getBuffer(size_t) override;
  void deregister(void*) override;

  const char* get_name_() const override;
  Snpe_SNPE_Handle_t getHandle() const;

  PsnpeExecutor() : psnpe_(new psnpe_handler()){};

  ~PsnpeExecutor() override = default;
};

#endif  // MOBILE_APP_OPEN_PSNPEEXECUTOR_H
