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

#ifndef MOBILE_APP_OPEN_BACKEND_UTILS_H
#define MOBILE_APP_OPEN_BACKEND_UTILS_H

#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "DiagLog/IDiagLog.h"
#include "DlContainer/DlContainer.h"
#include "DlSystem/DlEnums.h"
#include "DlSystem/DlError.h"
#include "DlSystem/IBufferAttributes.h"
#include "DlSystem/IUserBuffer.h"
#include "DlSystem/PlatformConfig.hpp"
#include "DlSystem/StringList.h"
#include "DlSystem/TensorMap.h"
#include "DlSystem/TensorShape.h"
#include "DlSystem/TensorShapeMap.h"
#include "DlSystem/UserBufferMap.h"
#include "SNPE/PSNPE.h"
#include "SNPE/SNPE.h"
#include "SNPE/SNPEBuilder.h"
#include "SNPE/SNPEUtil.h"
#include "SNPE/UserBufferList.h"
#include "absl/strings/ascii.h"
#include "allocator.h"
#include "cpuctrl.h"
#include "flutter/cpp/c/backend_c.h"
#include "flutter/cpp/c/type.h"
#include "tensorflow/core/platform/logging.h"

enum class snpe_runtimes_ {
  SNPE_DSP = 0,
  SNPE_GPU = 1,
  SNPE_CPU = 2,
  SNPE_GPU_FP16 = 3
};

void get_accelerator_instances_utils(int &numDSP, int &numGPU, int &numCPU,
                                     int &numGPU_FP16);
bool IsRuntimeAvailable_utils(const snpe_runtimes_ delegate);
Snpe_StringList_Handle_t ResolveCommaSeparatedList_utils(std::string &line);
static void split_utils(std::vector<std::string> &split_string,
                        const std::string &tokenized_string,
                        const char separator);
Snpe_Runtime_t Str2Delegate_utils(const snpe_runtimes_ delegate);
size_t calcSizeFromDims_utils(const size_t rank, const size_t *dims);
Snpe_TensorShape_Handle_t calcStrides_utils(
    Snpe_TensorShape_Handle_t dimsHandle, size_t elementSize);

// set functions
void setScenario_utils(std::string scenario_);
void setDelegate_utils(std::string delegate_);
#endif  // MOBILE_APP_OPEN_BACKEND_UTILS_H
