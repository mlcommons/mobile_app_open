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

#include "PsnpeExecutor.h"

mlperf_status_t PsnpeExecutor::execute(ft_callback /*callback*/,
                                       void* /*context*/) {
  if (useIonBuffers_ && !isIonRegistered_) {
    if (Snpe_PSNPE_RegisterUserMemoryMappedBuffers(
            psnpe_->psnpeHandle, userMemoryMappedBufferMapHandle_) ==
        SNPE_SUCCESS) {
      LOG(INFO) << "Ion Buffer Registration Successful";
      isIonRegistered_ = true;
    } else {
      LOG(FATAL) << "Not able to do registration";
    }
  }
  if (Snpe_PSNPE_Execute(psnpe_->psnpeHandle, inputMapListHandle_,
                         outputMapListHandle_) != SNPE_SUCCESS) {
    return MLPERF_FAILURE;
  }
  return MLPERF_SUCCESS;
}

void PsnpeExecutor::Init(const char* model_path) {
  uint32_t numInits = get_num_inits();
  LOG(INFO) << "Using PSNPE. numInits: " << numInits;

// Enable debug logs
#ifdef DEBUG_FLAG
  if (Snpe_Util_InitializeLogging(SNPE_LOG_LEVEL_VERBOSE)) {
    LOG(INFO) << "Debug logs successful";
  } else {
    LOG(INFO) << "Debug logs can not be intialized";
  }
#endif

  bool psnpe_buildStatus = false;
  // Init cache generated on device giving better performance.
  // So build the DLC twice to use the init cache generated in the first run
  for (int i = 0; i < numInits; i++) {
    // Open the DL container that contains the network to execute.
    Snpe_DlContainer_Handle_t containerHandle =
        Snpe_DlContainer_Open(model_path);
    // destroys previous snpe instance and creates a new one.
    psnpe_.reset(new psnpe_handler());
    // Loads the container after destroying the previous instance
    if (!containerHandle) {
      LOG(FATAL) << "Container is not available " << model_path;
    }

    Snpe_BuildConfig_Handle_t buildConfigHandle = Snpe_BuildConfig_Create();
    Snpe_BuildConfig_SetContainer(buildConfigHandle, containerHandle);
    Snpe_BuildConfig_SetRuntimeConfigList(buildConfigHandle,
                                          runtimeConfigsListHandle);
    Snpe_BuildConfig_SetInputOutputTransmissionMode(
        buildConfigHandle,
        static_cast<Snpe_PSNPE_InputOutputTransmissionMode_t>(
            SNPE_PSNPE_INPUTOUTPUTTRANSMISSIONMODE_SYNC));

    Snpe_StringList_Handle_t outputLayers =
        ResolveCommaSeparatedList_utils(snpeOutputLayers_);

    Snpe_StringList_Handle_t outputTensors =
        ResolveCommaSeparatedList_utils(snpeOutputTensors_);

    Snpe_SNPEBuilder_Handle_t snpeBuilderHandle =
        Snpe_SNPEBuilder_Create(containerHandle);
    dummyInputRuntimeListHandle = Snpe_RuntimeList_Create();
    Snpe_RuntimeList_Add(dummyInputRuntimeListHandle, SNPE_RUNTIME_CPU);
    setupPerfHandle();
    Snpe_SNPEBuilder_SetPerformanceProfile(snpeBuilderHandle, perfProfile_);
    Snpe_SNPEBuilder_SetExecutionPriorityHint(snpeBuilderHandle,
                                              SNPE_EXECUTION_PRIORITY_HIGH);
    Snpe_SNPEBuilder_SetRuntimeProcessorOrder(snpeBuilderHandle,
                                              dummyInputRuntimeListHandle);
    Snpe_SNPEBuilder_SetOutputLayers(snpeBuilderHandle, outputLayers);
    Snpe_SNPEBuilder_SetOutputTensors(snpeBuilderHandle, outputTensors);

    if (Snpe_StringList_Size(outputLayers) > 0)
      Snpe_BuildConfig_SetOutputBufferNames(buildConfigHandle, outputLayers);

    std::string platformOptionStr = "";
    if (useCpuInt8_) {
      platformOptionStr = "enableCpuFxpMode:ON";
    }
    if (Socs::get_use_dsp_features()) {
      // use unsignedPD feature for untrusted app.
      platformOptionStr += "unsignedPD:ON";
    }
    if (Socs::soc_check_feature(useIonBuffers_, platformOptionStr)) {
      Snpe_BuildConfig_SetEnableInitCache(buildConfigHandle, true);
    }
    Snpe_BuildConfig_SetPlatformOptions(buildConfigHandle,
                                        platformOptionStr.c_str());

    Snpe_PlatformConfig_Handle_t platformConfigHandle =
        Snpe_PlatformConfig_Create();
    bool setSuccess = Snpe_PlatformConfig_SetPlatformOptions(
        platformConfigHandle, platformOptionStr.c_str());
    bool isValid = Snpe_PlatformConfig_IsOptionsValid(platformConfigHandle);
    if (!isValid) {
      LOG(INFO) << "platformconfig option is invalid";
    }
    Snpe_SNPEBuilder_SetPlatformConfig(snpeBuilderHandle, platformConfigHandle);
    snpe_->snpeHandle = Snpe_SNPEBuilder_Build(snpeBuilderHandle);

    psnpe_buildStatus = (Snpe_PSNPE_Build(psnpe_->psnpeHandle,
                                          buildConfigHandle) == SNPE_SUCCESS);
    // Saves the container if there is a modification in any of the record.
    if (numInits > 1) Snpe_DlContainer_Save(containerHandle, model_path);

    Snpe_DlContainer_Delete(containerHandle);
    Snpe_BuildConfig_Delete(buildConfigHandle);
    Snpe_StringList_Delete(outputLayers);
    Snpe_SNPEBuilder_Delete(snpeBuilderHandle);
    Snpe_PlatformConfig_Delete(platformConfigHandle);
  }

  if (!psnpe_buildStatus) {
    LOG(FATAL) << "Error in init of psnpe_ " << psnpe_buildStatus;
  }

  if (profilingLevel_ != SNPE_PROFILING_LEVEL_OFF) {
    auto diagLogHandle = Snpe_SNPE_GetDiagLogInterface_Ref(snpe_->snpeHandle);
    if (!diagLogHandle) LOG(INFO) << "Get diagLogHandle failed";
    auto optionsHandle = Snpe_IDiagLog_GetOptions(diagLogHandle);
    std::string OutputDir = ".\diaglogs";
#ifdef __ANDROID__
    OutputDir =
        "/sdcard/Android/data/org.mlcommons.android.mlperfbench/files/diaglogs";
#endif
    Snpe_Options_SetLogFileDirectory(optionsHandle, OutputDir.c_str());

    if (Snpe_IDiagLog_SetOptions(diagLogHandle, optionsHandle) != SNPE_SUCCESS)
      LOG(INFO) << "Failed to set DiagLog options";

    if (Snpe_IDiagLog_Start(diagLogHandle) != SNPE_SUCCESS)
      LOG(INFO) << "Failed to start logger ";
  }
}

void* PsnpeExecutor::getBuffer(size_t n) {
  void* batchedDataPtr = nullptr;

  if (useIonBuffers_) {
    const char* name = Snpe_StringList_At(networkInputTensorNamesHandle_, 0);

    size_t totalBlocks = ChunkAllocator::GetSize(n) + (inputBatch_ - 1);

    batchedDataPtr = get_buffer(n, totalBlocks);
    uint64_t Offset = ChunkAllocator::GetOffset(batchedDataPtr);

    if (SnpeExecutor::count % inputBatch_ == 0) {
      Snpe_UserMemoryMap_AddFdOffset(
          userMemoryMappedBufferMapHandle_, name,
          ChunkAllocator::GetBatchPtr(batchedDataPtr), n * totalBlocks, fd,
          Offset);
    }
    SnpeExecutor::count++;
  } else {
    batchedDataPtr = get_buffer(n, inputBatch_);
  }

  return batchedDataPtr;
}

void PsnpeExecutor::deregister(void* p) {
  if (useIonBuffers_ && isIonRegistered_) {
    Snpe_StringList_Handle_t userBufferNames =
        Snpe_UserMemoryMap_GetUserBufferNames(userMemoryMappedBufferMapHandle_);

    if (Snpe_PSNPE_DeregisterUserMemoryMappedBuffers(
            psnpe_->psnpeHandle, userBufferNames) != SNPE_SUCCESS)
      LOG(INFO) << "Deregistration Failed !";

    auto input_buffer_name = Snpe_StringList_At(userBufferNames, 0);
    Snpe_UserMemoryMap_Remove(userMemoryMappedBufferMapHandle_,
                              input_buffer_name);
    Snpe_StringList_Delete(userBufferNames);
    isIonRegistered_ = false;
  }
  release_buffer(p);
}

const char* PsnpeExecutor::get_name_() const { return name_; }
