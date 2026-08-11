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

#include <memory>

#include "Executor.h"
#include "PsnpeExecutor.h"
#include "SnpeExecutor.h"
#ifdef GENIE_FLAG
#include "GenieExecutor.h"
#endif
// #include "QnnExecutor.h"
#include "SdExecutor.h"

enum class Backends { SNPE, PSNPE, QNN, GENIE, SD };

class BackendFactory {
 public:
  static std::unique_ptr<Executor> createBackend(const Backends type) {
    try {
      switch (type) {
        case Backends::SNPE:
          return createSnpeBackend();
        case Backends::PSNPE:
          return createPsnpeBackend();
          // case Backends::QNN:
          //   return createQnnBackend();
#ifdef GENIE_FLAG
        case Backends::GENIE:
          return createGenieBackend();
#endif
        case Backends::SD:
          return createSdBackend();
        default:
          LOG(FATAL) << "Invalid Backend error";
          return nullptr;
      }
    } catch (const std::exception& e) {
      LOG(FATAL) << "Backend initialization error: " << e.what();
      return nullptr;
    }
  }

 private:
  static std::unique_ptr<Executor> createSnpeBackend() {
    LOG(INFO) << "Utilizing snpe object";
    return std::make_unique<SnpeExecutor>();
  }

  static std::unique_ptr<Executor> createPsnpeBackend() {
    LOG(INFO) << "Utilizing psnpe object";
    return std::make_unique<PsnpeExecutor>();
  }
  //
  // static std::unique_ptr<Executor> createQnnBackend()
  // {
  //   LOG(INFO) << "Utilizing qnn object";
  //   return std::make_unique<QnnExecutor>();
  // }
  //
#ifdef GENIE_FLAG
  static std::unique_ptr<Executor> createGenieBackend() {
    LOG(INFO) << "Utilizing Genie Dialog";
    return std::make_unique<GenieExecutor>();
  }
#endif
  static std::unique_ptr<Executor> createSdBackend() {
    LOG(INFO) << "Utilizing SD Pipeline";
    return std::make_unique<SdExecutor>();
  }
};