/* Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.

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

#include <unordered_map>
#include <vector>

#include "SNPE/PSNPE.h"
#include "SNPE/SNPE.h"
#include "allocator.h"
#include "flutter/cpp/c/backend_c.h"
#include "flutter/cpp/c/type.h"

#define SIGNED_PD 0
#define UNSIGNED_PD 1
#define DEFAULT -1

class snpe_handler {
 public:
  Snpe_SNPE_Handle_t snpeHandle;

  ~snpe_handler() { Snpe_SNPE_Delete(snpeHandle); }
};

class psnpe_handler {
 public:
  Snpe_PSNPE_Handle_t psnpeHandle;

  psnpe_handler() { psnpeHandle = Snpe_PSNPE_Create(); }

  ~psnpe_handler() { Snpe_PSNPE_Delete(psnpeHandle); }
};

class QTIBackendHelper {
 private:
  Snpe_RuntimeList_Handle_t inputRuntimeListHandle;
  Snpe_RuntimeList_Handle_t dummyInputRuntimeListHandle;
  Snpe_RuntimeConfigList_Handle_t runtimeConfigsListHandle;

  inline int get_num_inits();
  void get_accelerator_instances(int &numDSP, int &numAIP, int &numGPU,
                                 int &numCPU);

 public:
  enum QTIBufferType { FLOAT_32 = 0, UINT_8 = 1, INT_32 = 2 };
  using GetBufferFn = std::add_pointer<void *(size_t, int)>::type;
  using ReleaseBufferFn = std::add_pointer<void(void *)>::type;
  const char *name_ = "snpe";
  const char *acceleratorName_;
  std::string snpeOutputLayers_;
  std::vector<mlperf_data_t> inputFormat_;
  std::vector<mlperf_data_t> outputFormat_;
  std::unique_ptr<psnpe_handler> psnpe_;
  std::unique_ptr<snpe_handler> snpe_;
  Snpe_UserBufferList_Handle_t inputMapListHandle_, outputMapListHandle_;
  std::vector<
      std::unordered_map<std::string, std::vector<uint8_t, Allocator<uint8_t>>>>
      bufs_;
  std::string scenario_;
  std::unordered_map<int, std::string> odLayerMap;
  Snpe_StringList_Handle_t networkInputTensorNamesHandle_;
  Snpe_StringList_Handle_t networkOutputTensorNamesHandle_;
  Snpe_PerformanceProfile_t perfProfile_;

  bool isTflite_;
  bool useSnpe_;
  mlperf_backend_ptr_t tfliteBackend_;
  int batchSize_;
  int queryCount_;
  int inputBatch_;
  int outputBatchBufsize_;
  GetBufferFn getBuffer_;
  ReleaseBufferFn releaseBuffer_;
  bool bgLoad_;
  std::string delegate_;
  QTIBufferType inputBufferType_ = UINT_8;
  QTIBufferType outputBufferType_ = FLOAT_32;
  uint32_t loadOffTime_ = 2;
  uint32_t loadOnTime_ = 100;
  bool useIonBuffers_ = true;

  /* exposed functions */
  void use_psnpe(const char *model_path);
  void use_snpe(const char *model_path);
  mlperf_status_t execute();
  void map_inputs();
  void map_outputs();
  void get_data_formats();
  void set_runtime_config();
  std::string get_snpe_version();

  QTIBackendHelper()
      : inputRuntimeListHandle(Snpe_RuntimeList_Create()),
        dummyInputRuntimeListHandle(Snpe_RuntimeList_Create()),
        runtimeConfigsListHandle(Snpe_RuntimeConfigList_Create()),
        networkInputTensorNamesHandle_(Snpe_StringList_Create()),
        networkOutputTensorNamesHandle_(Snpe_StringList_Create()),
        inputMapListHandle_(Snpe_UserBufferList_Create()),
        outputMapListHandle_(Snpe_UserBufferList_Create()),
        snpe_(new snpe_handler()),
        psnpe_(new psnpe_handler()) {
    odLayerMap[0] = "detection_boxes:0";
    odLayerMap[1] = "Postprocessor/BatchMultiClassNonMaxSuppression_classes";
    odLayerMap[2] = "detection_scores:0";
    odLayerMap[3] =
        "Postprocessor/BatchMultiClassNonMaxSuppression_num_detections";
  }

  ~QTIBackendHelper() {
    Snpe_RuntimeList_Delete(inputRuntimeListHandle);
    Snpe_RuntimeList_Delete(dummyInputRuntimeListHandle);
    Snpe_StringList_Delete(networkInputTensorNamesHandle_);
    Snpe_StringList_Delete(networkOutputTensorNamesHandle_);
    Snpe_UserBufferList_Delete(inputMapListHandle_);
    Snpe_UserBufferList_Delete(outputMapListHandle_);
  }
};

#endif
