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

#ifndef MOBILE_APP_OPEN_SNPEEXECUTOR_H
#define MOBILE_APP_OPEN_SNPEEXECUTOR_H

#include <string.h>

#include <unordered_map>

#include "DlSystem/DlEnums.hpp"
#include "Executor.h"
#include "backend_utils.h"
#include "soc_utility.h"

class snpe_handler {
 public:
  snpe_handler() {}
  Snpe_SNPE_Handle_t snpeHandle;
  ~snpe_handler() { Snpe_SNPE_Delete(snpeHandle); }
};

class SnpeExecutor : public Executor {
 public:
  std::unordered_map<std::string, DlSystem::DspPerfPowerMode_t> powerModeMap = {
      {"SNPE_DSP_PERF_INFRASTRUCTURE_POWERMODE_ADJUST_UP_DOWN",
       DlSystem::DspPerfPowerMode_t::
           DSP_PERF_INFRASTRUCTURE_POWERMODE_ADJUST_UP_DOWN},
      {"SNPE_DSP_PERF_INFRASTRUCTURE_POWERMODE_ADJUST_ONLY_UP",
       DlSystem::DspPerfPowerMode_t::
           DSP_PERF_INFRASTRUCTURE_POWERMODE_ADJUST_ONLY_UP},
      {"SNPE_DSP_PERF_INFRASTRUCTURE_POWERMODE_POWER_SAVER_MODE",
       DlSystem::DspPerfPowerMode_t::
           DSP_PERF_INFRASTRUCTURE_POWERMODE_POWER_SAVER_MODE},
      {"SNPE_DSP_PERF_INFRASTRUCTURE_POWERMODE_POWER_SAVER_AGGRESSIVE_MODE",
       DlSystem::DspPerfPowerMode_t::
           DSP_PERF_INFRASTRUCTURE_POWERMODE_POWER_SAVER_AGGRESSIVE_MODE},
      {"SNPE_DSP_PERF_INFRASTRUCTURE_POWERMODE_PERFORMANCE_MODE",
       DlSystem::DspPerfPowerMode_t::
           DSP_PERF_INFRASTRUCTURE_POWERMODE_PERFORMANCE_MODE},
      {"SNPE_DSP_PERF_INFRASTRUCTURE_POWERMODE_DUTY_CYCLE_MODE",
       DlSystem::DspPerfPowerMode_t::
           DSP_PERF_INFRASTRUCTURE_POWERMODE_DUTY_CYCLE_MODE},
      {"SNPE_DSP_PERF_INFRASTRUCTURE_POWERMODE_UNKNOWN",
       DlSystem::DspPerfPowerMode_t::
           DSP_PERF_INFRASTRUCTURE_POWERMODE_UNKNOWN}};

  std::unordered_map<std::string, DlSystem::DspPerfVoltageCorner_t>
      voltageCornerMap = {
          {"SNPE_DCVS_VOLTAGE_CORNER_DISABLE",
           DlSystem::DspPerfVoltageCorner_t::DCVS_VOLTAGE_CORNER_DISABLE},
          {"SNPE_DCVS_VOLTAGE_VCORNER_MIN_VOLTAGE_CORNER",
           DlSystem::DspPerfVoltageCorner_t::
               DCVS_VOLTAGE_VCORNER_MIN_VOLTAGE_CORNER},
          {"SNPE_DCVS_VOLTAGE_VCORNER_SVS2",
           DlSystem::DspPerfVoltageCorner_t::DCVS_VOLTAGE_VCORNER_SVS2},
          {"SNPE_DCVS_VOLTAGE_VCORNER_SVS",
           DlSystem::DspPerfVoltageCorner_t::DCVS_VOLTAGE_VCORNER_SVS},
          {"SNPE_DCVS_VOLTAGE_VCORNER_SVS_PLUS",
           DlSystem::DspPerfVoltageCorner_t::DCVS_VOLTAGE_VCORNER_SVS_PLUS},
          {"SNPE_DCVS_VOLTAGE_VCORNER_NOM",
           DlSystem::DspPerfVoltageCorner_t::DCVS_VOLTAGE_VCORNER_NOM},
          {"SNPE_DCVS_VOLTAGE_VCORNER_NOM_PLUS",
           DlSystem::DspPerfVoltageCorner_t::DCVS_VOLTAGE_VCORNER_NOM_PLUS},
          {"SNPE_DCVS_VOLTAGE_VCORNER_TURBO",
           DlSystem::DspPerfVoltageCorner_t::DCVS_VOLTAGE_VCORNER_TURBO},
          {"SNPE_DCVS_VOLTAGE_VCORNER_TURBO_PLUS",
           DlSystem::DspPerfVoltageCorner_t::DCVS_VOLTAGE_VCORNER_TURBO_PLUS},
          {"SNPE_DCVS_VOLTAGE_VCORNER_MAX_VOLTAGE_CORNER",
           DlSystem::DspPerfVoltageCorner_t::
               DCVS_VOLTAGE_VCORNER_MAX_VOLTAGE_CORNER},
          {"SNPE_DCVS_VOLTAGE_VCORNER_TURBO_L1",
           DlSystem::DspPerfVoltageCorner_t::DCVS_VOLTAGE_VCORNER_TURBO_L1},
          {"SNPE_DCVS_VOLTAGE_VCORNER_TURBO_L2",
           DlSystem::DspPerfVoltageCorner_t::DCVS_VOLTAGE_VCORNER_TURBO_L2},
          {"SNPE_DCVS_VOLTAGE_VCORNER_TURBO_L3",
           DlSystem::DspPerfVoltageCorner_t::DCVS_VOLTAGE_VCORNER_TURBO_L3},
          {"SNPE_DCVS_VOLTAGE_VCORNER_UNKNOWN",
           DlSystem::DspPerfVoltageCorner_t::DCVS_VOLTAGE_VCORNER_UNKNOWN}};

  std::unordered_map<std::string, DlSystem::DspHmx_ClkPerfMode_t>
      hmxClkPerfModeMap = {{"SNPE_HMX_CLK_PERF_HIGH",
                            DlSystem::DspHmx_ClkPerfMode_t::HMX_CLK_PERF_HIGH},
                           {"SNPE_HMX_CLK_PERF_LOW",
                            DlSystem::DspHmx_ClkPerfMode_t::HMX_CLK_PERF_LOW}};

  std::unordered_map<std::string, DlSystem::DspHmx_ExpVoltageCorner_t>
      hmxVoltageCornerMap = {
          {"SNPE_DCVS_EXP_VCORNER_DISABLE",
           DlSystem::DspHmx_ExpVoltageCorner_t::DCVS_EXP_VCORNER_DISABLE},
          {"SNPE_DCVS_EXP_VCORNER_MIN",
           DlSystem::DspHmx_ExpVoltageCorner_t::DCVS_EXP_VCORNER_MIN},
          {"SNPE_DCVS_EXP_VCORNER_LOW_SVS_D2",
           DlSystem::DspHmx_ExpVoltageCorner_t::DCVS_EXP_VCORNER_LOW_SVS_D2},
          {"SNPE_DCVS_EXP_VCORNER_LOW_SVS_D1",
           DlSystem::DspHmx_ExpVoltageCorner_t::DCVS_EXP_VCORNER_LOW_SVS_D1},
          {"SNPE_DCVS_EXP_VCORNER_LOW_SVS",
           DlSystem::DspHmx_ExpVoltageCorner_t::DCVS_EXP_VCORNER_LOW_SVS},
          {"SNPE_DCVS_EXP_VCORNER_SVS",
           DlSystem::DspHmx_ExpVoltageCorner_t::DCVS_EXP_VCORNER_SVS},
          {"SNPE_DCVS_EXP_VCORNER_SVS_L1",
           DlSystem::DspHmx_ExpVoltageCorner_t::DCVS_EXP_VCORNER_SVS_L1},
          {"SNPE_DCVS_EXP_VCORNER_NOM",
           DlSystem::DspHmx_ExpVoltageCorner_t::DCVS_EXP_VCORNER_NOM},
          {"SNPE_DCVS_EXP_VCORNER_NOM_L1",
           DlSystem::DspHmx_ExpVoltageCorner_t::DCVS_EXP_VCORNER_NOM_L1},
          {"SNPE_DCVS_EXP_VCORNER_TUR",
           DlSystem::DspHmx_ExpVoltageCorner_t::DCVS_EXP_VCORNER_TUR},
          {"SNPE_DCVS_EXP_VCORNER_TUR_L1",
           DlSystem::DspHmx_ExpVoltageCorner_t::DCVS_EXP_VCORNER_TUR_L1},
          {"SNPE_DCVS_EXP_VCORNER_TUR_L2",
           DlSystem::DspHmx_ExpVoltageCorner_t::DCVS_EXP_VCORNER_TUR_L2},
          {"SNPE_DCVS_EXP_VCORNER_TUR_L3",
           DlSystem::DspHmx_ExpVoltageCorner_t::DCVS_EXP_VCORNER_TUR_L3},
          {"SNPE_DCVS_EXP_VCORNER_MAX",
           DlSystem::DspHmx_ExpVoltageCorner_t::DCVS_EXP_VCORNER_MAX}};

  const char* name_ = "snpe";
  static int count, flag;
  Snpe_RuntimeList_Handle_t inputRuntimeListHandle;
  Snpe_RuntimeList_Handle_t dummyInputRuntimeListHandle;
  Snpe_RuntimeConfigList_Handle_t runtimeConfigsListHandle;
  Snpe_UserBufferList_Handle_t inputMapListHandle_, outputMapListHandle_;
  Snpe_UserMemoryMap_Handle_t userMemoryMappedBufferMapHandle_;
  Snpe_StringList_Handle_t networkInputTensorNamesHandle_;
  Snpe_StringList_Handle_t networkOutputTensorNamesHandle_;
  Snpe_SNPEPerfProfile_Handle_t customPerfProfile_ = nullptr;
  std::string snpeOutputLayers_;
  std::string snpeOutputTensors_;
  std::unordered_map<int, std::string> odLayerMap;
  Snpe_PerformanceProfile_t perfProfile_ = SNPE_PERFORMANCE_PROFILE_BURST;
  Snpe_ProfilingLevel_t profilingLevel_;
  int32_t fd = -1;
  bool useIonBuffers_ = true;
  bool useCpuInt8_ = false;
  bool isIonRegistered_;
  int batchSize_;
  int inputBatch_;
  int outputBatchBufsize_;
  int inputBatchBufsize_;
  std::vector<mlperf_data_t> inputFormat_;
  std::vector<mlperf_data_t> outputFormat_;
  QTIBufferType inputBufferType_ = QTIBufferType::UINT_8;
  QTIBufferType outputBufferType_ = QTIBufferType::FLOAT_32;
  std::unique_ptr<snpe_handler> snpe_;

 public:
  std::vector<
      std::unordered_map<std::string, std::vector<uint8_t, Allocator<uint8_t>>>>
      bufs_;
  std::unordered_map<std::string, std::string> customPerfProfileMap_;

  // Key functions
  void create(const char* model_path,
              const char* native_model_path = "") override;
  virtual void Init(const char* model_path);
  mlperf_status_t execute(ft_callback callback = nullptr,
                          void* context = nullptr) override;
  mlperf_status_t set_input(int32_t, int32_t, void*) override;
  mlperf_status_t get_output(uint32_t, int32_t, void**) override;
  void* getBuffer(size_t) override;
  void deregister(void*) override;

  void map_inputs();
  void map_outputs();
  void set_runtime_config();
  void get_data_formats();
  static std::string get_sdk_version();
  bool setupPerfHandle();

  // Get functions
  inline int get_num_inits() { return Socs::soc_num_inits(); }
  const char* get_name_() const override;
  bool getUseIonBuffers_() const override;
  std::vector<mlperf_data_t> getInputFormat_() const override;
  std::vector<mlperf_data_t> getOutputFormat_() const override;

  // setting configs
  void setConfigs(const mlperf_backend_configuration_t* configs) override;

  SnpeExecutor()
      : inputRuntimeListHandle(Snpe_RuntimeList_Create()),
        dummyInputRuntimeListHandle(Snpe_RuntimeList_Create()),
        runtimeConfigsListHandle(Snpe_RuntimeConfigList_Create()),
        networkInputTensorNamesHandle_(Snpe_StringList_Create()),
        networkOutputTensorNamesHandle_(Snpe_StringList_Create()),
        inputMapListHandle_(Snpe_UserBufferList_Create()),
        outputMapListHandle_(Snpe_UserBufferList_Create()),
        snpe_(new snpe_handler()) {
    odLayerMap[0] = "detection_boxes:0";
    odLayerMap[1] = "Postprocessor/BatchMultiClassNonMaxSuppression_classes";
    odLayerMap[2] = "detection_scores:0";
    odLayerMap[3] =
        "Postprocessor/BatchMultiClassNonMaxSuppression_num_detections";
    userMemoryMappedBufferMapHandle_ = Snpe_UserMemoryMap_Create();
    isIonRegistered_ = false;
  };

  ~SnpeExecutor() {
    Snpe_RuntimeList_Delete(inputRuntimeListHandle);
    Snpe_RuntimeList_Delete(dummyInputRuntimeListHandle);
    Snpe_RuntimeConfigList_Delete(runtimeConfigsListHandle);
    Snpe_StringList_Delete(networkInputTensorNamesHandle_);
    Snpe_StringList_Delete(networkOutputTensorNamesHandle_);
    Snpe_UserBufferList_Delete(inputMapListHandle_);
    Snpe_UserBufferList_Delete(outputMapListHandle_);
    Snpe_UserMemoryMap_Delete(userMemoryMappedBufferMapHandle_);
    if (customPerfProfile_) Snpe_SNPEPerfProfile_Delete(customPerfProfile_);
  };
};
#endif  // MOBILE_APP_OPEN_SNPEEXECUTOR_H