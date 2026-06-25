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

#include "SnpeExecutor.h"

#define xverstr(a) verstr(a)
#define verstr(a) #a

#ifndef SNPE_VERSION_STRING
#define SNPE_VERSION_STRING "default"
#endif

int SnpeExecutor::count = 0, SnpeExecutor::flag = 0;

std::string SnpeExecutor::get_sdk_version() {
  Snpe_DlVersion_Handle_t version = Snpe_Util_GetLibraryVersion();
  return Snpe_DlVersion_GetBuild(version);
}

void SnpeExecutor::create(const char *model_path,
                          const char *native_model_path) {
  // reset the flags
  SnpeExecutor::count = 0;
  SnpeExecutor::flag = 0;
  std::string snpe_version = xverstr(SNPE_VERSION_STRING);
  if (snpe_version.compare("default") != 0) {
    int dotPosition = snpe_version.find_last_of(".");
    snpe_version = snpe_version.substr(dotPosition + 1);
  }

  if (get_sdk_version().find_first_of(snpe_version) != 0) {
    LOG(FATAL) << "Snpe libs modified. expected: " << snpe_version
               << " found: " << get_sdk_version();
  }
  LOG(INFO) << "snpe_version: " << snpe_version;

  set_runtime_config();
  Init(model_path);
  get_data_formats();
  map_inputs();
  map_outputs();

  LOG(INFO) << "SNPE build completed successfully";
}

void SnpeExecutor::set_runtime_config() {
  int numDSP = 0, numGPU = 0, numCPU = 0, numGPU_FP16 = 0;
  get_accelerator_instances_utils(numDSP, numGPU, numCPU, numGPU_FP16);
  Snpe_Runtime_t runtime;
  for (int i = 0; i < numDSP; i++) {
    if (i == 0) {
      runtime = Str2Delegate_utils(snpe_runtimes_::SNPE_DSP);
    }
    auto runtimeConfigHandle = Snpe_RuntimeConfig_Create();

    Snpe_RuntimeConfig_SetRuntime(runtimeConfigHandle, runtime);
    Snpe_RuntimeConfig_SetPerformanceProfile(runtimeConfigHandle, perfProfile_);
    Snpe_RuntimeConfigList_PushBack(runtimeConfigsListHandle,
                                    runtimeConfigHandle);
    Snpe_RuntimeList_Add(inputRuntimeListHandle, runtime);
    setupPerfHandle();
    Snpe_RuntimeConfig_Delete(runtimeConfigHandle);
  }

  for (int i = 0; i < numGPU; i++) {
    if (i == 0) {
      runtime = Str2Delegate_utils(snpe_runtimes_::SNPE_GPU);
    }
    auto runtimeConfigHandle = Snpe_RuntimeConfig_Create();
    Snpe_RuntimeConfig_SetRuntime(runtimeConfigHandle, runtime);
    Snpe_RuntimeConfig_SetPerformanceProfile(runtimeConfigHandle, perfProfile_);
    Snpe_RuntimeConfigList_PushBack(runtimeConfigsListHandle,
                                    runtimeConfigHandle);
    Snpe_RuntimeList_Add(inputRuntimeListHandle, runtime);
    setupPerfHandle();
    Snpe_RuntimeConfig_Delete(runtimeConfigHandle);
  }

  for (int i = 0; i < numCPU; i++) {
    if (i == 0) {
      runtime = Str2Delegate_utils(snpe_runtimes_::SNPE_CPU);
    }
    auto runtimeConfigHandle = Snpe_RuntimeConfig_Create();
    Snpe_RuntimeConfig_SetRuntime(runtimeConfigHandle, runtime);
    Snpe_RuntimeConfig_SetPerformanceProfile(runtimeConfigHandle, perfProfile_);
    Snpe_RuntimeConfigList_PushBack(runtimeConfigsListHandle,
                                    runtimeConfigHandle);
    Snpe_RuntimeList_Add(inputRuntimeListHandle, runtime);
    setupPerfHandle();
    Snpe_RuntimeConfig_Delete(runtimeConfigHandle);
  }

  for (int i = 0; i < numGPU_FP16; i++) {
    if (i == 0) {
      runtime = Str2Delegate_utils(snpe_runtimes_::SNPE_GPU_FP16);
    }
    auto runtimeConfigHandle = Snpe_RuntimeConfig_Create();
    Snpe_RuntimeConfig_SetRuntime(runtimeConfigHandle, runtime);
    Snpe_RuntimeConfig_SetPerformanceProfile(runtimeConfigHandle, perfProfile_);
    Snpe_RuntimeConfigList_PushBack(runtimeConfigsListHandle,
                                    runtimeConfigHandle);
    Snpe_RuntimeList_Add(inputRuntimeListHandle, runtime);
    setupPerfHandle();

    Snpe_RuntimeConfig_Delete(runtimeConfigHandle);
  }
}

void SnpeExecutor::Init(const char *model_path) {
  uint32_t numInits = get_num_inits();
  LOG(INFO) << "using SNPE. numInits: " << numInits;

// Enable debug logs
#ifdef DEBUG_FLAG
  if (Snpe_Util_InitializeLogging(SNPE_LOG_LEVEL_VERBOSE)) {
    LOG(INFO) << "Debug logs successful";
  } else {
    LOG(INFO) << "Debug logs can not be intialized";
  }
#endif

  // Use SNPE
  for (int i = 0; i < numInits; i++) {
    // Open the DL container that contains the network to execute.
    Snpe_DlContainer_Handle_t containerHandle =
        Snpe_DlContainer_Open(model_path);
    // Loads the container after destroying the previous instance
    if (!containerHandle) {
      LOG(FATAL) << "Container is not available " << model_path;
    }

    Snpe_SNPEBuilder_Handle_t snpeBuilderHandle =
        Snpe_SNPEBuilder_Create(containerHandle);
    Snpe_SNPEBuilder_SetCpuFixedPointMode(snpeBuilderHandle, useCpuInt8_);
    Snpe_StringList_Handle_t outputLayers =
        ResolveCommaSeparatedList_utils(snpeOutputLayers_);
    Snpe_StringList_Handle_t outputTensors =
        ResolveCommaSeparatedList_utils(snpeOutputTensors_);
    setupPerfHandle();
    Snpe_SNPEBuilder_SetPerformanceProfile(snpeBuilderHandle, perfProfile_);
    Snpe_SNPEBuilder_SetProfilingLevel(snpeBuilderHandle, profilingLevel_);
    Snpe_SNPEBuilder_SetExecutionPriorityHint(snpeBuilderHandle,
                                              SNPE_EXECUTION_PRIORITY_HIGH);
    Snpe_SNPEBuilder_SetRuntimeProcessorOrder(snpeBuilderHandle,
                                              inputRuntimeListHandle);
    Snpe_SNPEBuilder_SetUseUserSuppliedBuffers(snpeBuilderHandle, true);
    Snpe_SNPEBuilder_SetOutputLayers(snpeBuilderHandle, outputLayers);
    Snpe_SNPEBuilder_SetOutputTensors(snpeBuilderHandle, outputTensors);

    std::string platformOptionStr = "";
    if (Socs::soc_check_feature(useIonBuffers_, platformOptionStr)) {
      Snpe_SNPEBuilder_SetInitCacheMode(snpeBuilderHandle, true);
    }
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
    // Saves the container if there is a modification in any of the record.
    if (numInits > 1) Snpe_DlContainer_Save(containerHandle, model_path);

    Snpe_DlContainer_Delete(containerHandle);
    Snpe_SNPEBuilder_Delete(snpeBuilderHandle);
    Snpe_StringList_Delete(outputLayers);
    Snpe_PlatformConfig_Delete(platformConfigHandle);
  }
  if (!snpe_->snpeHandle) {
    LOG(FATAL) << "Error in init of the model " << snpe_;
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

void SnpeExecutor::map_inputs() {
  Snpe_UserBufferMap_Handle_t inputMapHandle = Snpe_UserBufferMap_Create();

  uint64_t offset = 0;
  std::vector<uint8_t> inputBufferShared;

  for (int bi = 0; bi < batchSize_ / inputBatch_; bi++) {
    for (size_t i = 0; i < Snpe_StringList_Size(networkInputTensorNamesHandle_);
         ++i) {
      const char *name = Snpe_StringList_At(networkInputTensorNamesHandle_, i);
      Snpe_IBufferAttributes_Handle_t ubaOptHandle =
          Snpe_SNPE_GetInputOutputBufferAttributes(snpe_->snpeHandle, name);
      Snpe_TensorShape_Handle_t dimsHandle =
          Snpe_IBufferAttributes_GetDims(ubaOptHandle);
      long bufSize =
          calcSizeFromDims_utils(Snpe_TensorShape_Rank(dimsHandle),
                                 Snpe_TensorShape_GetDimensions(dimsHandle));
      std::vector<Snpe_IUserBuffer_Handle_t> ubPtr;
      std::vector<uint8_t> inputBuffer;

      if (inputBufferType_ == QTIBufferType::FLOAT_32) {
        // Prepare float buffer
        bufSize *= sizeof(float);
        inputBuffer.resize(bufSize);
        auto stridesHandle = calcStrides_utils(
            Snpe_IBufferAttributes_GetDims(ubaOptHandle), sizeof(float));
        Snpe_UserBufferEncoding_Handle_t ubeFloatHandle =
            Snpe_UserBufferEncodingFloat_Create();

        ubPtr.push_back(
            Snpe_Util_CreateUserBuffer(inputBuffer.data(), inputBuffer.size(),
                                       stridesHandle, ubeFloatHandle));
        if (ubPtr.back() == nullptr) {
          Snpe_IBufferAttributes_Delete(ubaOptHandle);
          LOG(FATAL) << "Null pointer returned from CreateUserBuffer.";
        }
        Snpe_UserBufferMap_Add(inputMapHandle, name, ubPtr.back());

        Snpe_TensorShape_Delete(stridesHandle);
        Snpe_UserBufferEncodingFloat_Delete(ubeFloatHandle);
      } else {
        // Prepare tf8 buffer
        bufSize *= sizeof(uint8_t);
        // Pass the quantization parameters from the model to UB
        if (useIonBuffers_ && inputBatch_ > 1) {
          if (bi == 0) {
            inputBufferShared.resize(bufSize * (batchSize_ / inputBatch_));
          }
        } else {
          inputBuffer.resize(bufSize);
        }

        auto stridesHandle = calcStrides_utils(
            Snpe_IBufferAttributes_GetDims(ubaOptHandle), sizeof(uint8_t));
        auto ubeTfN = Snpe_IUserBuffer_GetEncoding_Ref(ubaOptHandle);

        // Set the default QP for model which doesn't have QP.
        // HTP may not need to use the QP for the current set of DLCs
        // This may be required for AIP though.
        // LOG(INFO) << "QP parameters from model: " << ubeTfN;
        if (!ubeTfN)
          ubeTfN = Snpe_UserBufferEncodingTfN_Create(128.0, 1.0 / 255, 8);

        if (useIonBuffers_ && inputBatch_ > 1) {
          ubPtr.push_back(Snpe_Util_CreateUserBufferShared(
              std::move(inputBufferShared.data()), bufSize, offset,
              stridesHandle, ubeTfN));
        } else {
          ubPtr.push_back(Snpe_Util_CreateUserBuffer(
              std::move(inputBuffer.data()), inputBuffer.size(), stridesHandle,
              ubeTfN));
        }

        Snpe_UserBufferMap_Add(inputMapHandle, name, ubPtr.back());

        Snpe_TensorShape_Delete(stridesHandle);
        Snpe_UserBufferEncodingTfN_Delete(ubeTfN);
      }
      inputBatchBufsize_ = bufSize;
      Snpe_IBufferAttributes_Delete(ubaOptHandle);
      Snpe_TensorShape_Delete(dimsHandle);
    }
    offset += inputBatchBufsize_;
    Snpe_UserBufferList_PushBack(inputMapListHandle_, inputMapHandle);
  }
  bufs_.resize(batchSize_ / inputBatch_);
  Snpe_UserBufferMap_Delete(inputMapHandle);
}

void SnpeExecutor::map_outputs() {
  Snpe_UserBufferMap_Handle_t outputMapHandle = Snpe_UserBufferMap_Create();

  uint64_t offset = 0;

  for (int bi = 0; bi < batchSize_ / inputBatch_; bi++) {
    for (size_t i = 0;
         i < Snpe_StringList_Size(networkOutputTensorNamesHandle_); ++i) {
      const char *name = Snpe_StringList_At(networkOutputTensorNamesHandle_, i);
      Snpe_IBufferAttributes_Handle_t ubaOptHandle =
          Snpe_SNPE_GetInputOutputBufferAttributes(snpe_->snpeHandle, name);
      Snpe_TensorShape_Handle_t dimsHandle =
          Snpe_IBufferAttributes_GetDims(ubaOptHandle);
      long bufSize =
          calcSizeFromDims_utils(Snpe_TensorShape_Rank(dimsHandle),
                                 Snpe_TensorShape_GetDimensions(dimsHandle));

      outputBatchBufsize_ = bufSize;

      if (useIonBuffers_) {
        Allocator<uint8_t>::useIonAllocator();
      } else {
        Allocator<uint8_t>::useDefaultAllocator();
      }
      if (outputBufferType_ == QTIBufferType::UINT_8) {
        auto ubeTfN = Snpe_UserBufferEncodingTfN_Create(0, 1.0f, 8);

        std::vector<Snpe_IUserBuffer_Handle_t> x;

        auto stridesHandle = calcStrides_utils(
            Snpe_IBufferAttributes_GetDims(ubaOptHandle), sizeof(uint8_t));

        if (useIonBuffers_ && inputBatch_ > 1) {
          if (bi == 0) {
            bufs_[bi].emplace(
                std::string(name),
                std::vector<uint8_t, Allocator<uint8_t>>(
                    bufSize * sizeof(uint8_t) * (batchSize_ / inputBatch_)));
          }
          x.push_back(Snpe_Util_CreateUserBufferShared(bufs_[0].at(name).data(),
                                                       bufSize, offset,
                                                       stridesHandle, ubeTfN));
        } else {
          bufs_[bi].emplace(std::string(name),
                            std::vector<uint8_t, Allocator<uint8_t>>(
                                bufSize * sizeof(uint8_t)));
          x.push_back(Snpe_Util_CreateUserBuffer(
              bufs_[bi].at(name).data(), bufSize, stridesHandle, ubeTfN));
        }

        Snpe_UserBufferMap_Add(outputMapHandle, name, x.back());
        if (useIonBuffers_ && inputBatch_ > 1) {
          Snpe_UserMemoryMap_AddFdOffset(
              userMemoryMappedBufferMapHandle_, name, bufs_[0].at(name).data(),
              bufSize * sizeof(uint8_t) * (batchSize_ / inputBatch_), fd,
              offset);
        } else if (useIonBuffers_) {
          Snpe_UserMemoryMap_AddFdOffset(userMemoryMappedBufferMapHandle_, name,
                                         bufs_[bi].at(name).data(),
                                         bufSize * sizeof(uint8_t), fd, 0);
        }
        Snpe_UserBufferEncodingTfN_Delete(ubeTfN);
        Snpe_TensorShape_Delete(stridesHandle);
      } else if (outputBufferType_ == QTIBufferType::INT_32) {
        auto ubeIntN = Snpe_UserBufferEncodingIntN_Create(32);
        std::vector<Snpe_IUserBuffer_Handle_t> x;
        bufs_[bi].emplace(std::string(name),
                          std::vector<uint8_t, Allocator<uint8_t>>(
                              bufSize * sizeof(int32_t)));
        auto stridesHandle = calcStrides_utils(
            Snpe_IBufferAttributes_GetDims(ubaOptHandle), sizeof(int32_t));
        x.push_back(Snpe_Util_CreateUserBuffer(bufs_[bi].at(name).data(),
                                               bufSize * sizeof(int32_t),
                                               stridesHandle, ubeIntN));

        Snpe_UserBufferMap_Add(outputMapHandle, name, x.back());
        if (useIonBuffers_)
          Snpe_UserMemoryMap_AddFdOffset(userMemoryMappedBufferMapHandle_, name,
                                         bufs_[bi].at(name).data(),
                                         bufSize * sizeof(int32_t), fd, 0);

        Snpe_UserBufferEncodingIntN_Delete(ubeIntN);
        Snpe_TensorShape_Delete(stridesHandle);
      } else {
        auto userBufferEncodingFloat = Snpe_UserBufferEncodingFloat_Create();

        std::vector<Snpe_IUserBuffer_Handle_t> x;
        bufs_[bi].emplace(
            std::string(name),
            std::vector<uint8_t, Allocator<uint8_t>>(bufSize * sizeof(float)));
        auto stridesHandle = calcStrides_utils(
            Snpe_IBufferAttributes_GetDims(ubaOptHandle), sizeof(float));
        x.push_back(Snpe_Util_CreateUserBuffer(
            bufs_[bi].at(name).data(), bufSize * sizeof(float), stridesHandle,
            userBufferEncodingFloat));
        Snpe_UserBufferMap_Add(outputMapHandle, name, x.back());
        if (useIonBuffers_) {
          Snpe_UserMemoryMap_AddFdOffset(userMemoryMappedBufferMapHandle_, name,
                                         bufs_[bi].at(name).data(),
                                         bufSize * sizeof(float), fd, 0);
        }
        Snpe_UserBufferEncodingFloat_Delete(userBufferEncodingFloat);
        Snpe_TensorShape_Delete(stridesHandle);
      }
      Snpe_IBufferAttributes_Delete(ubaOptHandle);
      Snpe_TensorShape_Delete(dimsHandle);
    }

    offset += outputBatchBufsize_;
    Snpe_UserBufferList_PushBack(outputMapListHandle_, outputMapHandle);
  }
  Snpe_UserBufferMap_Delete(outputMapHandle);
}

void SnpeExecutor::get_data_formats() {
  networkInputTensorNamesHandle_ =
      Snpe_SNPE_GetInputTensorNames(snpe_->snpeHandle);
  if (!networkInputTensorNamesHandle_) {
    throw std::runtime_error("Error obtaining Input tensor names");
  }

  for (size_t i = 0; i < Snpe_StringList_Size(networkInputTensorNamesHandle_);
       ++i) {
    const char *name = Snpe_StringList_At(networkInputTensorNamesHandle_, i);
    auto inputShapeHandle =
        Snpe_SNPE_GetInputDimensions(snpe_->snpeHandle, name);

    if (inputShapeHandle == nullptr)
      throw std::runtime_error("Failed to obtain input dimensions");
    inputBatch_ = Snpe_TensorShape_At(inputShapeHandle, 0);

    Snpe_IBufferAttributes_Handle_t ubaOptHandle =
        Snpe_SNPE_GetInputOutputBufferAttributes(snpe_->snpeHandle, name);

    Snpe_TensorShape_Handle_t dimsHandle =
        Snpe_IBufferAttributes_GetDims(ubaOptHandle);

    long bufSize =
        calcSizeFromDims_utils(Snpe_TensorShape_Rank(dimsHandle),
                               Snpe_TensorShape_GetDimensions(dimsHandle));

    if (inputBufferType_ == FLOAT_32) {
      // Input buffer type FLOAT
      inputFormat_.push_back(
          {mlperf_data_t::Type::Float32, bufSize / inputBatch_});
    } else {
      // Input buffer type UINT8
      inputFormat_.push_back(
          {mlperf_data_t::Type::Uint8, bufSize / inputBatch_});
    }
    Snpe_TensorShape_Delete(inputShapeHandle);
    Snpe_IBufferAttributes_Delete(ubaOptHandle);
    Snpe_TensorShape_Delete(dimsHandle);
  }

  networkOutputTensorNamesHandle_ =
      Snpe_SNPE_GetOutputTensorNames(snpe_->snpeHandle);
  if (!networkOutputTensorNamesHandle_) {
    throw std::runtime_error("Error obtaining Output tensor names");
  }

  for (size_t i = 0; i < Snpe_StringList_Size(networkOutputTensorNamesHandle_);
       ++i) {
    const char *name = Snpe_StringList_At(networkOutputTensorNamesHandle_, i);
    Snpe_IBufferAttributes_Handle_t ubaOptHandle =
        Snpe_SNPE_GetInputOutputBufferAttributes(snpe_->snpeHandle, name);
    Snpe_TensorShape_Handle_t dimsHandle =
        Snpe_IBufferAttributes_GetDims(ubaOptHandle);
    long bufSize =
        calcSizeFromDims_utils(Snpe_TensorShape_Rank(dimsHandle),
                               Snpe_TensorShape_GetDimensions(dimsHandle));
    if (outputBufferType_ == FLOAT_32) {
      if (snpeOutputLayers_ == "transpose" ||
          snpeOutputTensors_ == "transpose:0") {
        // For mobileBERT, return output size as half the size of computed
        // values,
        // because the DLC returns only single layer as output but the app needs
        // 2 tensors,
        // split from the single tensor
        bufSize = bufSize / 2;
        outputFormat_.push_back(
            {mlperf_data_t::Type::Float32, bufSize / inputBatch_});
        outputFormat_.push_back(
            {mlperf_data_t::Type::Float32, bufSize / inputBatch_});
      } else {
        // output buffer type FLOAT
        outputFormat_.push_back(
            {mlperf_data_t::Type::Float32, bufSize / inputBatch_});
      }
    } else if (outputBufferType_ == INT_32) {
      // output buffer type INT32
      outputFormat_.push_back(
          {mlperf_data_t::Type::Int32, bufSize / inputBatch_});
    } else {
      // output buffer type UINT8
      outputFormat_.push_back(
          {mlperf_data_t::Type::Uint8, bufSize / inputBatch_});
    }
    Snpe_IBufferAttributes_Delete(ubaOptHandle);
    Snpe_TensorShape_Delete(dimsHandle);
  }
}

mlperf_status_t SnpeExecutor::set_input(int32_t batchIndex, int32_t i,
                                        void *data) {
  void *batchedDataPtr = ((useIonBuffers_ == false) && (inputBatch_ <= 1))
                             ? data
                             : ChunkAllocator::GetBatchPtr(data);
  if (useIonBuffers_ && inputBatch_ > 1) {
    if (!SnpeExecutor::flag) {
      SnpeExecutor::count = 0;
      SnpeExecutor::flag++;
    }

    if (SnpeExecutor::count++ % inputBatch_ != 0) {
      return MLPERF_SUCCESS;
    }
  }

  // Set the input data pointer to the user buffer
  // "Snpe_UserBufferMap_GetUserBuffer_Ref" is a macro that expands to
  // "Snpe_UserBufferMap_GetUserBuffer
  Snpe_IUserBuffer_SetBufferAddress(
      Snpe_UserBufferMap_GetUserBuffer_Ref(
          Snpe_UserBufferList_At_Ref(inputMapListHandle_,
                                     batchIndex / inputBatch_),
          Snpe_StringList_At(networkInputTensorNamesHandle_, i)),
      batchedDataPtr);

  if (useIonBuffers_) {
    uint64_t offset = ChunkAllocator::GetOffset(data);

    Snpe_IUserBuffer_SetBufferAddressOffset(
        Snpe_UserBufferMap_GetUserBuffer_Ref(
            Snpe_UserBufferList_At_Ref(inputMapListHandle_,
                                       batchIndex / inputBatch_),
            Snpe_StringList_At(networkInputTensorNamesHandle_, i)),
        offset);
  }

  return MLPERF_SUCCESS;
}

mlperf_status_t SnpeExecutor::get_output(uint32_t batchIndex,
                                         int32_t outputIndex, void **data) {
  if (snpeOutputTensors_.find(
          "Postprocessor/BatchMultiClassNonMaxSuppression_classes") !=
          std::string::npos ||
      snpeOutputLayers_ == "Postprocessor/BatchMultiClassNonMaxSuppression") {
    // Reorder snpeOutputLayers_ for coco process_output
    const char *outputLayerName = odLayerMap[outputIndex].c_str();
    *data = bufs_[batchIndex].at(odLayerMap[outputIndex]).data();
    return MLPERF_SUCCESS;
  } else if (snpeOutputTensors_.find("transpose:0") != std::string::npos ||
             snpeOutputLayers_ == "transpose") {
    *data = bufs_[int(batchIndex / inputBatch_)]
                .at(Snpe_StringList_At(networkOutputTensorNamesHandle_, 0))
                .data() +
            (1 - outputIndex) * 384 * sizeof(float);
    return MLPERF_SUCCESS;
  }
  size_t size = sizeof(float);
  if (outputBufferType_ == Executor::QTIBufferType::UINT_8) {
    size = sizeof(uint8_t);
  }

  int index =
      (inputBatch_ > 1 && useIonBuffers_) ? 0 : int(batchIndex / inputBatch_);
  auto offsetAddress =
      (inputBatch_ > 1 && useIonBuffers_)
          ? (outputBatchBufsize_ * (batchIndex / inputBatch_) * size)
          : 0;

  *data =
      bufs_[index]
          .at(Snpe_StringList_At(networkOutputTensorNamesHandle_, outputIndex))
          .data() +
      (batchIndex % inputBatch_) * int(outputBatchBufsize_ / inputBatch_) *
          size +
      offsetAddress;

  return MLPERF_SUCCESS;
}

mlperf_status_t SnpeExecutor::execute(ft_callback /*callback*/,
                                      void * /*context*/) {
  if (useIonBuffers_ && !isIonRegistered_) {
    if (Snpe_SNPE_RegisterUserMemoryMappedBuffers(
            snpe_->snpeHandle, userMemoryMappedBufferMapHandle_) ==
        SNPE_SUCCESS) {
      LOG(INFO) << "Ion Buffer Registration Successful";
      isIonRegistered_ = true;
    } else {
      LOG(FATAL) << "Not able to do registration";
    }
  }

  auto in_ = Snpe_UserBufferList_At_Ref(inputMapListHandle_, 0);
  auto out_ = Snpe_UserBufferList_At_Ref(outputMapListHandle_, 0);

  if (Snpe_SNPE_ExecuteUserBuffers(snpe_->snpeHandle, in_, out_) !=
      SNPE_SUCCESS) {
    LOG(ERROR) << "Failed to execute SNPE model. Check input/output buffer "
                  "configuration.";
    return MLPERF_FAILURE;
  }
  return MLPERF_SUCCESS;
}

void *SnpeExecutor::getBuffer(size_t n) {
  void *batchedDataPtr = nullptr;

  if (useIonBuffers_) {
    const char *name = Snpe_StringList_At(networkInputTensorNamesHandle_, 0);

    size_t totalBlocks = ChunkAllocator::GetSize(n) + (inputBatch_ - 1);

    batchedDataPtr = get_buffer(n, totalBlocks);
    uint64_t Offset = ChunkAllocator::GetOffset(batchedDataPtr);

    if (SnpeExecutor::count % inputBatch_ == 0) {
      Snpe_UserMemoryMap_AddFdOffset(
          userMemoryMappedBufferMapHandle_, name,
          ChunkAllocator::GetBatchPtr(batchedDataPtr), n * totalBlocks, fd,
          Offset);
    }

  } else {
    batchedDataPtr = get_buffer(n, inputBatch_);
  }

  return batchedDataPtr;
}

void SnpeExecutor::deregister(void *p) {
  if (useIonBuffers_ && isIonRegistered_) {
    Snpe_StringList_Handle_t userBufferNames =
        Snpe_UserMemoryMap_GetUserBufferNames(userMemoryMappedBufferMapHandle_);
    if (Snpe_SNPE_DeregisterUserMemoryMappedBuffers(
            snpe_->snpeHandle, userBufferNames) != SNPE_SUCCESS)
      LOG(INFO) << "Deregistration Failed !";

    auto input_buffer_name = Snpe_StringList_At(userBufferNames, 0);
    Snpe_UserMemoryMap_Remove(userMemoryMappedBufferMapHandle_,
                              input_buffer_name);
    Snpe_StringList_Delete(userBufferNames);
    isIonRegistered_ = false;
  }
  release_buffer(p);
}

bool SnpeExecutor::setupPerfHandle() {
  customPerfProfile_ = Snpe_SNPEPerfProfile_CreatePreset(perfProfile_);
  for (std::unordered_map<std::string, std::string>::iterator mapIter =
           customPerfProfileMap_.begin();
       mapIter != customPerfProfileMap_.end(); ++mapIter) {
    Snpe_ErrorCode_t err;
    std::string setting = mapIter->first;
    std::string value = mapIter->second;
    // Set various settings using the APIs
    if (setting == "DSP_ENABLE_DCVS_START") {
      err = Snpe_SNPEPerfProfile_SetEnableDspDcvsStart(customPerfProfile_,
                                                       value == "true");
    } else if (setting == "DSP_ENABLE_DCVS_DONE") {
      err = Snpe_SNPEPerfProfile_SetEnableDspDcvsDone(customPerfProfile_,
                                                      value == "true");
    } else if (setting == "ASYNC_VOTING_ENABLE") {
      err = Snpe_SNPEPerfProfile_SetEnableAsyncVoting(customPerfProfile_,
                                                      value == "true");
    } else if (setting == "DSP_SLEEP_LATENCY_START_US") {
      err = Snpe_SNPEPerfProfile_SetSleepLatencyStart(customPerfProfile_,
                                                      std::stoi(value));
    } else if (setting == "DSP_SLEEP_LATENCY_DONE_US") {
      err = Snpe_SNPEPerfProfile_SetSleepLatencyDone(customPerfProfile_,
                                                     std::stoi(value));
    } else if (setting == "HIGH_PERFORMANCE_MODE") {
      err = Snpe_SNPEPerfProfile_SetHighPerformanceModeEnabled(
          customPerfProfile_, value == "true");
    } else if (setting == "DSP_HYSTERESIS_TIME_US") {
      err = Snpe_SNPEPerfProfile_SetDspHysteresisTime(customPerfProfile_,
                                                      std::stoi(value));
    } else if (setting == "POWERMODE_START") {
      err = Snpe_SNPEPerfProfile_SetPowerModeStart(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_PowerMode_t>(powerModeMap[value]));
    } else if (setting == "DCVS_VOLTAGE_CORNER_MIN_START") {
      err = Snpe_SNPEPerfProfile_SetDcvsVoltageCornerDcvsVCornerMinStart(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_VoltageCorner_t>(voltageCornerMap[value]));
    } else if (setting == "DCVS_VOLTAGE_CORNER_MIN_DONE") {
      err = Snpe_SNPEPerfProfile_SetDcvsVoltageCornerDcvsVCornerMinDone(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_VoltageCorner_t>(voltageCornerMap[value]));
    } else if (setting == "DCVS_VOLTAGE_CORNER_MAX_START") {
      err = Snpe_SNPEPerfProfile_SetDcvsVoltageCornerDcvsVCornerMaxStart(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_VoltageCorner_t>(voltageCornerMap[value]));
    } else if (setting == "DCVS_VOLTAGE_CORNER_MAX_DONE") {
      err = Snpe_SNPEPerfProfile_SetDcvsVoltageCornerDcvsVCornerMaxDone(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_VoltageCorner_t>(voltageCornerMap[value]));
    } else if (setting == "DCVS_VOLTAGE_CORNER_TARGET_START") {
      err = Snpe_SNPEPerfProfile_SetDcvsVoltageCornerDcvsVCornerTargetStart(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_VoltageCorner_t>(voltageCornerMap[value]));
    } else if (setting == "DCVS_VOLTAGE_CORNER_TARGET_DONE") {
      err = Snpe_SNPEPerfProfile_SetDcvsVoltageCornerDcvsVCornerTargetDone(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_VoltageCorner_t>(voltageCornerMap[value]));
    } else if (setting == "DSP_SLEEP_DISABLE_MS") {
      err = Snpe_SNPEPerfProfile_SetSleepDisable(customPerfProfile_,
                                                 std::stoi(value));
    } else if (setting == "DSP_RPC_POLLING_TIME_US") {
      err = Snpe_SNPEPerfProfile_SetDspRpcPollingTime(customPerfProfile_,
                                                      std::stoi(value));
    } else if (setting == "BUS_VOLTAGE_CORNER_MIN_START") {
      err = Snpe_SNPEPerfProfile_SetBusVoltageCornerMinStart(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_VoltageCorner_t>(voltageCornerMap[value]));
    } else if (setting == "BUS_VOLTAGE_CORNER_MIN_DONE") {
      err = Snpe_SNPEPerfProfile_SetBusVoltageCornerMinDone(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_VoltageCorner_t>(voltageCornerMap[value]));
    } else if (setting == "BUS_VOLTAGE_CORNER_MAX_START") {
      err = Snpe_SNPEPerfProfile_SetBusVoltageCornerMaxStart(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_VoltageCorner_t>(voltageCornerMap[value]));
    } else if (setting == "BUS_VOLTAGE_CORNER_MAX_DONE") {
      err = Snpe_SNPEPerfProfile_SetBusVoltageCornerMaxDone(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_VoltageCorner_t>(voltageCornerMap[value]));
    } else if (setting == "BUS_VOLTAGE_CORNER_TARGET_START") {
      err = Snpe_SNPEPerfProfile_SetBusVoltageCornerTargetStart(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_VoltageCorner_t>(voltageCornerMap[value]));
    } else if (setting == "BUS_VOLTAGE_CORNER_TARGET_DONE") {
      err = Snpe_SNPEPerfProfile_SetBusVoltageCornerTargetDone(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_VoltageCorner_t>(voltageCornerMap[value]));
    } else if (setting == "CORE_VOLTAGE_CORNER_MIN_START") {
      err = Snpe_SNPEPerfProfile_SetCoreVoltageCornerminMvStart(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_VoltageCorner_t>(voltageCornerMap[value]));
    } else if (setting == "CORE_VOLTAGE_CORNER_MIN_DONE") {
      err = Snpe_SNPEPerfProfile_SetCoreVoltageCornerMinMvDone(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_VoltageCorner_t>(voltageCornerMap[value]));
    } else if (setting == "CORE_VOLTAGE_CORNER_MAX_START") {
      err = Snpe_SNPEPerfProfile_SetCoreVoltageCornerMaxMvStart(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_VoltageCorner_t>(voltageCornerMap[value]));
    } else if (setting == "CORE_VOLTAGE_CORNER_MAX_DONE") {
      err = Snpe_SNPEPerfProfile_SetCoreVoltageCornerMaxMvDone(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_VoltageCorner_t>(voltageCornerMap[value]));
    } else if (setting == "CORE_VOLTAGE_CORNER_TARGET_START") {
      err = Snpe_SNPEPerfProfile_SetCoreVoltageCornerTargetMvStart(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_VoltageCorner_t>(voltageCornerMap[value]));
    } else if (setting == "CORE_VOLTAGE_CORNER_TARGET_DONE") {
      err = Snpe_SNPEPerfProfile_SetCoreVoltageCornerTargetMvDone(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_VoltageCorner_t>(voltageCornerMap[value]));
    } else if (setting == "POWERMODE_DONE") {
      err = Snpe_SNPEPerfProfile_SetPowerModeDone(
          customPerfProfile_,
          static_cast<Snpe_DspPerf_PowerMode_t>(powerModeMap[value]));
    } else if (setting == "FAST_INIT_ENABLE") {
      err = Snpe_SNPEPerfProfile_SetFastInitEnabled(customPerfProfile_,
                                                    value == "true");
    } else if (setting == "DSP_HMX_CLOCK_PERF_MODE") {
      err = Snpe_SNPEPerfProfile_SetHmxClkPerfMode(
          customPerfProfile_,
          static_cast<Snpe_DspHmx_ClkPerfMode_t>(hmxClkPerfModeMap[value]));
    } else if (setting == "DSP_HMX_VOLTAGE_CORNER_MIN") {
      err = Snpe_SNPEPerfProfile_SetHmxVoltageCornerMin(
          customPerfProfile_, static_cast<Snpe_DspHmx_ExpVoltageCorner_t>(
                                  hmxVoltageCornerMap[value]));
    } else if (setting == "DSP_HMX_VOLTAGE_CORNER_MAX") {
      err = Snpe_SNPEPerfProfile_SetHmxVoltageCornerMax(
          customPerfProfile_, static_cast<Snpe_DspHmx_ExpVoltageCorner_t>(
                                  hmxVoltageCornerMap[value]));
    } else if (setting == "DSP_HMX_VOLTAGE_CORNER_TARGET") {
      err = Snpe_SNPEPerfProfile_SetHmxVoltageCornerTarget(
          customPerfProfile_, static_cast<Snpe_DspHmx_ExpVoltageCorner_t>(
                                  hmxVoltageCornerMap[value]));
    }
    if (err != SNPE_SUCCESS) {
      LOG(ERROR) << "could not parse setting " << setting << std::endl;
      return false;
    }
    LOG(INFO) << "Setting " << setting << " to " << value << std::endl;
  }
  return true;
}

const char *SnpeExecutor::get_name_() const { return name_; }

bool SnpeExecutor::getUseIonBuffers_() const { return useIonBuffers_; }

std::vector<mlperf_data_t> SnpeExecutor::getInputFormat_() const {
  return inputFormat_;
}

std::vector<mlperf_data_t> SnpeExecutor::getOutputFormat_() const {
  return outputFormat_;
}

void SnpeExecutor::setConfigs(const mlperf_backend_configuration_t *configs) {
  // Setting defaults
  setDelegate_utils(configs->accelerator);

  // Batch size is zero if not specified
  batchSize_ = (configs->batch_size == 0) ? 1 : configs->batch_size;

  // set ion buffers to true, unless specified from config
  useIonBuffers_ = true;

  // Handle custom settings
  std::string perfProfile = "burst";
  std::string profileLevel = "off";
  std::string scenario = "";

  // Applying config settings to backend
  for (int i = 0; i < configs->count; ++i) {
    if (strcmp(configs->keys[i], "scenario") == 0) {
      setScenario_utils(configs->values[i]);
      scenario = configs->values[i];
    } else if (strcmp(configs->keys[i], "snpe_output_layers") == 0) {
      snpeOutputLayers_ = (configs->values[i]);
    } else if (strcmp(configs->keys[i], "snpe_output_tensors") == 0) {
      snpeOutputTensors_ = (configs->values[i]);
    } else if (strcmp(configs->keys[i], "input_buffer_type") == 0) {
      if (std::strcmp(configs->values[i], "float_32") == 0) {
        inputBufferType_ = Executor::QTIBufferType::FLOAT_32;
      } else {
        inputBufferType_ = Executor::QTIBufferType::UINT_8;
      }
    } else if (strcmp(configs->keys[i], "output_buffer_type") == 0) {
      if (std::strcmp(configs->values[i], "float_32") == 0) {
        outputBufferType_ = Executor::QTIBufferType::FLOAT_32;
      } else if (std::strcmp(configs->values[i], "int_32") == 0) {
        outputBufferType_ = Executor::QTIBufferType::INT_32;
      } else {
        outputBufferType_ = Executor::QTIBufferType::UINT_8;
      }
    } else if (strcmp(configs->keys[i], "use_ion_buffer") == 0) {
      if (std::strcmp(configs->values[i], "true") == 0) {
        useIonBuffers_ = true;
      } else {
        useIonBuffers_ = false;
      }
      useIonBuffers_ =
          useIonBuffers_ && Socs::needs_rpcmem() && get_rpc_status();
    } else if (strcmp(configs->keys[i], "perf_profile") == 0) {
      perfProfile = configs->values[i];
      if ((std::strcmp(configs->values[i], "default") == 0) ||
          (std::strcmp(configs->values[i], "balanced") == 0)) {
        perfProfile_ = SNPE_PERFORMANCE_PROFILE_BALANCED;
      } else if (std::strcmp(configs->values[i], "high_performance") == 0) {
        perfProfile_ = SNPE_PERFORMANCE_PROFILE_HIGH_PERFORMANCE;
      } else if (std::strcmp(configs->values[i], "power_saver") == 0) {
        perfProfile_ = SNPE_PERFORMANCE_PROFILE_POWER_SAVER;
      } else if (std::strcmp(configs->values[i], "system_settings") == 0) {
        perfProfile_ = SNPE_PERFORMANCE_PROFILE_SYSTEM_SETTINGS;
      } else if (std::strcmp(configs->values[i],
                             "sustained_high_performance") == 0) {
        perfProfile_ = SNPE_PERFORMANCE_PROFILE_SUSTAINED_HIGH_PERFORMANCE;
      } else if (std::strcmp(configs->values[i], "burst") == 0) {
        perfProfile_ = SNPE_PERFORMANCE_PROFILE_BURST;
      } else if (std::strcmp(configs->values[i], "low_power_saver") == 0) {
        perfProfile_ = SNPE_PERFORMANCE_PROFILE_LOW_POWER_SAVER;
      } else if (std::strcmp(configs->values[i], "high_power_saver") == 0) {
        perfProfile_ = SNPE_PERFORMANCE_PROFILE_HIGH_POWER_SAVER;
      } else if (std::strcmp(configs->values[i], "low_balanced") == 0) {
        perfProfile_ = SNPE_PERFORMANCE_PROFILE_LOW_BALANCED;
      } else {
        LOG(INFO) << "Unrecognized performance profile: " << perfProfile;
        perfProfile_ = SNPE_PERFORMANCE_PROFILE_BURST;
      }
    } else if (strcmp(configs->keys[i], "bus_voltage_start") == 0) {
      customPerfProfileMap_["BUS_VOLTAGE_CORNER_MIN_START"] =
          configs->values[i];
      customPerfProfileMap_["BUS_VOLTAGE_CORNER_TARGET_START"] =
          configs->values[i];
      customPerfProfileMap_["BUS_VOLTAGE_CORNER_MAX_START"] =
          configs->values[i];
    } else if (strcmp(configs->keys[i], "core_voltage_start") == 0) {
      customPerfProfileMap_["CORE_VOLTAGE_CORNER_MIN_START"] =
          configs->values[i];
      customPerfProfileMap_["CORE_VOLTAGE_CORNER_TARGET_START"] =
          configs->values[i];
      customPerfProfileMap_["CORE_VOLTAGE_CORNER_MAX_START"] =
          configs->values[i];
    } else if (strcmp(configs->keys[i], "bus_voltage_done") == 0) {
      customPerfProfileMap_["BUS_VOLTAGE_CORNER_MIN_DONE"] = configs->values[i];
      customPerfProfileMap_["BUS_VOLTAGE_CORNER_TARGET_DONE"] =
          configs->values[i];
      customPerfProfileMap_["BUS_VOLTAGE_CORNER_MAX_DONE"] = configs->values[i];
    } else if (strcmp(configs->keys[i], "bus_voltage_done") == 0) {
      customPerfProfileMap_["CORE_VOLTAGE_CORNER_MIN_DONE"] =
          configs->values[i];
      customPerfProfileMap_["CORE_VOLTAGE_CORNER_TARGET_DONE"] =
          configs->values[i];
      customPerfProfileMap_["CORE_VOLTAGE_CORNER_MAX_DONE"] =
          configs->values[i];
    } else if (strcmp(configs->keys[i], "hmx_voltage") == 0) {
      customPerfProfileMap_["DSP_HMX_VOLTAGE_CORNER_TARGET"] =
          configs->values[i];
      customPerfProfileMap_["DSP_HMX_VOLTAGE_CORNER_MAX"] = configs->values[i];
      customPerfProfileMap_["DSP_HMX_VOLTAGE_CORNER_MIN"] = configs->values[i];
    } else if (strcmp(configs->keys[i], "hmx_clock_perf") == 0) {
      customPerfProfileMap_["DSP_HMX_CLOCK_PERF_MODE"] = configs->values[i];
    } else if (strcmp(configs->keys[i], "dsp_start_sleep_latency") == 0) {
      customPerfProfileMap_["DSP_SLEEP_LATENCY_START_US"] = configs->values[i];
    } else if (strcmp(configs->keys[i], "dsp_done_sleep_latency") == 0) {
      customPerfProfileMap_["DSP_SLEEP_LATENCY_DONE_US"] = configs->values[i];
    } else if (strcmp(configs->keys[i], "profiling_level") == 0) {
      profileLevel = configs->values[i];
      if (std::strcmp(configs->values[i], "off") == 0) {
        profilingLevel_ = SNPE_PROFILING_LEVEL_OFF;
      } else if (std::strcmp(configs->values[i], "basic") == 0) {
        profilingLevel_ = SNPE_PROFILING_LEVEL_BASIC;
      } else if (std::strcmp(configs->values[i], "moderate") == 0) {
        profilingLevel_ = SNPE_PROFILING_LEVEL_MODERATE;
      } else if (std::strcmp(configs->values[i], "detailed") == 0) {
        profilingLevel_ = SNPE_PROFILING_LEVEL_DETAILED;
      } else {
        LOG(INFO) << "Unrecognized  profiling level: " << profileLevel;
        profilingLevel_ = SNPE_PROFILING_LEVEL_OFF;
      }
    } else if (strcmp(configs->keys[i], "cpu_int8") == 0) {
      if (std::strcmp(configs->values[i], "true") == 0) {
        useCpuInt8_ = true;
      } else {
        useCpuInt8_ = false;
      }
    }
  }

  LOG(INFO) << " | scenario: " << scenario
            << " | output layer: " << snpeOutputLayers_
            << " | output tensor: " << snpeOutputTensors_
            << " | batchSize: " << batchSize_
            << " | inputBufferType: " << inputBufferType_
            << " | outputBufferType: " << outputBufferType_
            << " | perfProfile: " << perfProfile
            << " | profileLevel: " << profileLevel
            << " | useIonBuffer: " << useIonBuffers_
            << " | useCpuInt8: " << useCpuInt8_;
}