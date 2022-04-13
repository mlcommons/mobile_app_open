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

#ifndef QTIBACKENDHELPER_H
#define QTIBACKENDHELPER_H

#include <vector>

#include "DlSystem/IUserBufferFactory.hpp"
#include "SNPE/PSNPE.hpp"
#include "SNPE/SNPE.hpp"
#include "android/cpp/c/backend_c.h"
#include "android/cpp/c/type.h"

#include "allocator.h"

#define SIGNED_PD 0
#define UNSIGNED_PD 1
#define DEFAULT -1

class QTIBackendHelper {
 private:
  zdl::DlSystem::RuntimeList inputRuntimeList;
  zdl::DlSystem::RuntimeList dummyInputRuntimeList;
  zdl::PSNPE::RuntimeConfigList runtimeConfigsList;

  inline int get_num_inits();
  void get_accelerator_instances(int &numDSP, int &numAIP, int &numGPU,
                                 int &numCPU);

 public:
  enum QTIBufferType { FLOAT_32 = 0, UINT_8 = 1 };
  using GetBufferFn = std::add_pointer<void *(size_t)>::type;
  using ReleaseBufferFn = std::add_pointer<void(void *)>::type;

  const char *name_ = "snpe";
  std::string snpeOutputLayers_;
  std::vector<mlperf_data_t> inputFormat_;
  std::vector<mlperf_data_t> outputFormat_;
  std::unique_ptr<zdl::PSNPE::PSNPE> psnpe_;
  std::unique_ptr<zdl::SNPE::SNPE> snpe_;
  zdl::PSNPE::UserBufferList inputMap_, outputMap_;
  std::vector<
      std::unordered_map<std::string, std::vector<uint8_t, Allocator<uint8_t>>>>
      bufs_;
  std::string scenario_;
  zdl::DlSystem::StringList networkInputTensorNames_;
  zdl::DlSystem::StringList networkOutputTensorNames_;
  zdl::DlSystem::PerformanceProfile_t perfProfile_;

  bool isTflite_;
  bool useSnpe_;
  mlperf_backend_ptr_t tfliteBackend_;
  int batchSize_;
  int inputBatch_;
  int outputBatchBufsize_;
  GetBufferFn getBuffer_;
  ReleaseBufferFn releaseBuffer_;
  bool bgLoad_;
  std::string delegate_;
  QTIBufferType inputBufferType_ = UINT_8;
  QTIBufferType outputBufferType_ = FLOAT_32;
  bool useDspFeatures = false;
  uint32_t loadOffTime_ = 2;
  uint32_t loadOnTime_ = 100;
  bool useIonBuffers_ = true;

  /* exposed functions */
  void use_psnpe(const char *model_path);
  void use_snpe(const char *model_path);
  void map_inputs();
  void map_outputs();
  void get_data_formats();
  void set_runtime_config();
  std::string get_snpe_version();
};

#endif
