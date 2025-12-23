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

#include "qti_backend_helper.h"

#include <random>
#include <sstream>
#include <string>
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
#include "SNPE/SNPEBuilder.h"
#include "SNPE/SNPEUtil.h"
#include "SNPE/UserBufferList.h"
#include "absl/strings/ascii.h"
#include "cpuctrl.h"
#include "soc_utility.h"
#include "tensorflow/core/platform/logging.h"
#include "tflite_c.h"

static size_t calcSizeFromDims(const size_t rank, const size_t *dims) {
  if (rank == 0) return 0;
  size_t size = 1;
  for (size_t i = rank; i > 0; i--) {
    if (*dims != 0)
      size *= *dims;
    else
      size *= 10;
    dims++;
  }
  return size;
}

// Helper for splitting tokenized strings
static void split(std::vector<std::string> &split_string,
                  const std::string &tokenized_string, const char separator) {
  split_string.clear();
  std::istringstream tokenized_string_stream(tokenized_string);

  while (!tokenized_string_stream.eof()) {
    std::string value;
    getline(tokenized_string_stream, value, separator);
    if (!value.empty()) {
      split_string.push_back(value);
    }
  }
}

static Snpe_StringList_Handle_t ResolveCommaSeparatedList(std::string &line) {
  Snpe_StringList_Handle_t stringListHandle = Snpe_StringList_Create();
  if (!line.empty()) {
    std::vector<std::string> names;
    split(names, line.substr(0), ',');
    for (auto &name : names)
      Snpe_StringList_Append(stringListHandle, name.c_str());
  }
  return stringListHandle;
}

static Snpe_TensorShape_Handle_t calcStrides(
    Snpe_TensorShape_Handle_t dimsHandle, size_t elementSize) {
  std::vector<size_t> strides(Snpe_TensorShape_Rank(dimsHandle));
  strides[strides.size() - 1] = elementSize;
  size_t stride = strides[strides.size() - 1];
  for (size_t i = Snpe_TensorShape_Rank(dimsHandle) - 1; i > 0; i--) {
    if (Snpe_TensorShape_At(dimsHandle, i) != 0)
      stride *= Snpe_TensorShape_At(dimsHandle, i);
    else
      stride *= 10;
    strides[i - 1] = stride;
  }
  Snpe_TensorShape_Handle_t tensorShapeHandle = Snpe_TensorShape_CreateDimsSize(
      strides.data(), Snpe_TensorShape_Rank(dimsHandle));
  return tensorShapeHandle;
}

static Snpe_Runtime_t Str2Delegate(const snpe_runtimes_t delegate,
                                   bool isFatal = false) {
  Snpe_Runtime_t runtime;

  switch (delegate) {
    case SNPE_DSP:
      runtime = SNPE_RUNTIME_DSP;
      break;
    case SNPE_GPU:
      runtime = SNPE_RUNTIME_GPU;
      break;
    case SNPE_CPU:
      runtime = SNPE_RUNTIME_CPU;
      break;
    case SNPE_GPU_FP16:
      runtime = SNPE_RUNTIME_GPU_FLOAT16;
      break;
    default:
      runtime = SNPE_RUNTIME_UNSET;
      LOG(ERROR) << "runtime not supported";
      break;
  }

  if (Snpe_Util_IsRuntimeAvailableCheckOption(
          runtime, SNPE_RUNTIME_CHECK_OPTION_UNSIGNEDPD_CHECK)) {
    LOG(INFO) << "runtime " << delegate << " is available on this platform";
  } else {
    std::stringstream log_err_string;
    log_err_string << "runtime " << delegate
                   << " is not available on this platform";
    if (isFatal)
      LOG(FATAL) << log_err_string.str();
    else
      LOG(ERROR) << log_err_string.str();
  }

  return runtime;
}

bool QTIBackendHelper::IsRuntimeAvailable(const snpe_runtimes_t delegate) {
  return (Str2Delegate(delegate) != SNPE_RUNTIME_UNSET);
}

void QTIBackendHelper::use_psnpe(const char *model_path) {
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
        ResolveCommaSeparatedList(snpeOutputLayers_);

    Snpe_StringList_Handle_t outputTensors =
        ResolveCommaSeparatedList(snpeOutputTensors_);

    Snpe_SNPEBuilder_Handle_t snpeBuilderHandle =
        Snpe_SNPEBuilder_Create(containerHandle);
    dummyInputRuntimeListHandle = Snpe_RuntimeList_Create();
    Snpe_RuntimeList_Add(dummyInputRuntimeListHandle, SNPE_RUNTIME_CPU);
    setupPerfHandle();
    Snpe_SNPEBuilder_SetCustomPerfProfile(snpeBuilderHandle,
                                          customPerfProfile_);
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

    // psnpe_buildStatus = Snpe_PSNPE_Build(psnpe_->psnpeHandle,
    // buildConfigHandle);
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
  if (!snpe_->snpeHandle) {
    LOG(FATAL) << "Error in init of snpe_ " << snpe_->snpeHandle;
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
  // Snpe_DlContainer_Delete(containerHandle);
}

mlperf_status_t QTIBackendHelper::execute() {
  if (useIonBuffers_ && !isIonRegistered) {
    if (!useSnpe_) {
      if (Snpe_PSNPE_RegisterUserMemoryMappedBuffers(
              psnpe_->psnpeHandle, userMemoryMappedBufferMapHandle_) ==
          SNPE_SUCCESS) {
        LOG(INFO) << "Ion Buffer Registration Successful";
        isIonRegistered = true;
      } else {
        LOG(FATAL) << "Not able to do registration";
      }
    } else {
      if (Snpe_SNPE_RegisterUserMemoryMappedBuffers(
              snpe_->snpeHandle, userMemoryMappedBufferMapHandle_) ==
          SNPE_SUCCESS) {
        LOG(INFO) << "Ion Buffer Registration Successful";
        isIonRegistered = true;
      } else {
        LOG(FATAL) << "Not able to do registration";
      }
    }
  }
  if (!useSnpe_) {
    if (Snpe_PSNPE_Execute(psnpe_->psnpeHandle, inputMapListHandle_,
                           outputMapListHandle_) != SNPE_SUCCESS) {
      return MLPERF_FAILURE;
    }
  } else {
    auto in_ = Snpe_UserBufferList_At_Ref(inputMapListHandle_, 0);
    auto out_ = Snpe_UserBufferList_At_Ref(outputMapListHandle_, 0);
    if (Snpe_SNPE_ExecuteUserBuffers(snpe_->snpeHandle, in_, out_) !=
        SNPE_SUCCESS) {
      return MLPERF_FAILURE;
    }
  }
  return MLPERF_SUCCESS;
}

void QTIBackendHelper::use_snpe(const char *model_path) {
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
        ResolveCommaSeparatedList(snpeOutputLayers_);
    Snpe_StringList_Handle_t outputTensors =
        ResolveCommaSeparatedList(snpeOutputTensors_);
    setupPerfHandle();
    Snpe_SNPEBuilder_SetCustomPerfProfile(snpeBuilderHandle,
                                          customPerfProfile_);
    Snpe_SNPEBuilder_SetProfilingLevel(snpeBuilderHandle, profilingLevel_);
    Snpe_SNPEBuilder_SetExecutionPriorityHint(snpeBuilderHandle,
                                              SNPE_EXECUTION_PRIORITY_HIGH);
    Snpe_SNPEBuilder_SetRuntimeProcessorOrder(snpeBuilderHandle,
                                              inputRuntimeListHandle);
    Snpe_SNPEBuilder_SetUseUserSuppliedBuffers(snpeBuilderHandle, true);
    Snpe_SNPEBuilder_SetOutputLayers(snpeBuilderHandle, outputLayers);
    Snpe_SNPEBuilder_SetOutputTensors(snpeBuilderHandle, outputTensors);

    std::string platformOptionStr = "";
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

inline int QTIBackendHelper::get_num_inits() { return Socs::soc_num_inits(); }

void QTIBackendHelper::get_accelerator_instances(int &num_dsp, int &num_gpu,
                                                 int &num_cpu,
                                                 int &num_gpu_fp16) {
  std::string &delegate = delegate_;
  num_dsp = 0;
  num_gpu = 0;
  num_cpu = 0;
  num_gpu_fp16 = 0;
  if (scenario_ == "Offline") {
    Socs::soc_offline_core_instance(num_dsp, num_gpu, num_cpu, num_gpu_fp16,
                                    delegate);
  } else {
    if (delegate == "snpe_dsp" || delegate == "psnpe_dsp") {
      num_dsp = 1;
      Socs::set_use_dsp_features(true);
    } else if (delegate == "snpe_gpu" || delegate == "psnpe_gpu") {
      num_gpu = 1;
      Socs::set_use_dsp_features(false);
    } else if (delegate == "snpe_cpu" || delegate == "psnpe_cpu") {
      num_cpu = 1;
      Socs::set_use_dsp_features(false);
    } else if (delegate == "snpe_gpu_fp16" || delegate == "psnpe_gpu_fp16") {
      num_gpu_fp16 = 1;
      Socs::set_use_dsp_features(false);
    } else {
      LOG(FATAL) << "Error: Unsupported delegate " << delegate << " SoC ID "
                 << Socs::get_soc_name();
    }
  }
  LOG(INFO) << "Using " << num_dsp << " dsp " << num_gpu << " gpu" << num_cpu
            << " cpu" << num_gpu_fp16 << " gpu_fp16";
}

void QTIBackendHelper::map_inputs() {
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
          calcSizeFromDims(Snpe_TensorShape_Rank(dimsHandle),
                           Snpe_TensorShape_GetDimensions(dimsHandle));
      std::vector<Snpe_IUserBuffer_Handle_t> ubPtr;
      std::vector<uint8_t> inputBuffer;

      if (inputBufferType_ == QTIBufferType::FLOAT_32) {
        // Prepare float buffer
        bufSize *= sizeof(float);

        if (useIonBuffers_ && inputBatch_ > 1) {
          if (bi == 0) {
            inputBufferShared.resize(bufSize * (batchSize_ / inputBatch_));
          }
        } else {
          inputBuffer.resize(bufSize);
        }
        auto stridesHandle = calcStrides(
            Snpe_IBufferAttributes_GetDims(ubaOptHandle), sizeof(float));
        Snpe_UserBufferEncoding_Handle_t ubeFloatHandle =
            Snpe_UserBufferEncodingFloat_Create();

        if (useIonBuffers_ && inputBatch_ > 1) {
          ubPtr.push_back(Snpe_Util_CreateUserBufferShared(
              std::move(inputBufferShared.data()), bufSize, offset,
              stridesHandle, ubeFloatHandle));
        } else {
          ubPtr.push_back(Snpe_Util_CreateUserBuffer(
              std::move(inputBuffer.data()), inputBuffer.size(), stridesHandle,
              ubeFloatHandle));
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

        auto stridesHandle = calcStrides(
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

void QTIBackendHelper::map_outputs() {
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
          calcSizeFromDims(Snpe_TensorShape_Rank(dimsHandle),
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

        auto stridesHandle = calcStrides(
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
        auto stridesHandle = calcStrides(
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
        auto stridesHandle = calcStrides(
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

void QTIBackendHelper::get_data_formats() {
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

    long bufSize = calcSizeFromDims(Snpe_TensorShape_Rank(dimsHandle),
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
    long bufSize = calcSizeFromDims(Snpe_TensorShape_Rank(dimsHandle),
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

void QTIBackendHelper::set_runtime_config() {
  int numDSP = 0, numGPU = 0, numCPU = 0, numGPU_FP16 = 0;
  get_accelerator_instances(numDSP, numGPU, numCPU, numGPU_FP16);

  Snpe_Runtime_t runtime;
  for (int i = 0; i < numDSP; i++) {
    if (i == 0) {
      runtime = Str2Delegate(SNPE_DSP, true);
    }
    auto runtimeConfigHandle = Snpe_RuntimeConfig_Create();

    Snpe_RuntimeConfig_SetRuntime(runtimeConfigHandle, runtime);
    setupPerfHandle();
    Snpe_RuntimeConfig_SetCustomPerfProfile(runtimeConfigHandle,
                                            customPerfProfile_);
    Snpe_RuntimeConfigList_PushBack(runtimeConfigsListHandle,
                                    runtimeConfigHandle);
    Snpe_RuntimeList_Add(inputRuntimeListHandle, runtime);

    Snpe_RuntimeConfig_Delete(runtimeConfigHandle);
  }

  for (int i = 0; i < numGPU; i++) {
    if (i == 0) {
      runtime = Str2Delegate(SNPE_GPU, true);
    }
    auto runtimeConfigHandle = Snpe_RuntimeConfig_Create();
    Snpe_RuntimeConfig_SetRuntime(runtimeConfigHandle, runtime);
    setupPerfHandle();
    Snpe_RuntimeConfig_SetCustomPerfProfile(runtimeConfigHandle,
                                            customPerfProfile_);
    Snpe_RuntimeConfigList_PushBack(runtimeConfigsListHandle,
                                    runtimeConfigHandle);
    Snpe_RuntimeList_Add(inputRuntimeListHandle, runtime);
    Snpe_RuntimeConfig_Delete(runtimeConfigHandle);
  }

  for (int i = 0; i < numCPU; i++) {
    if (i == 0) {
      runtime = Str2Delegate(SNPE_CPU, true);
    }
    auto runtimeConfigHandle = Snpe_RuntimeConfig_Create();
    Snpe_RuntimeConfig_SetRuntime(runtimeConfigHandle, runtime);
    setupPerfHandle();
    Snpe_RuntimeConfig_SetCustomPerfProfile(runtimeConfigHandle,
                                            customPerfProfile_);
    Snpe_RuntimeConfigList_PushBack(runtimeConfigsListHandle,
                                    runtimeConfigHandle);
    Snpe_RuntimeList_Add(inputRuntimeListHandle, runtime);
    Snpe_RuntimeConfig_Delete(runtimeConfigHandle);
  }

  for (int i = 0; i < numGPU_FP16; i++) {
    if (i == 0) {
      runtime = Str2Delegate(SNPE_GPU_FP16);
    }
    auto runtimeConfigHandle = Snpe_RuntimeConfig_Create();
    Snpe_RuntimeConfig_SetRuntime(runtimeConfigHandle, runtime);
    setupPerfHandle();
    Snpe_RuntimeConfig_SetCustomPerfProfile(runtimeConfigHandle,
                                            customPerfProfile_);
    Snpe_RuntimeConfigList_PushBack(runtimeConfigsListHandle,
                                    runtimeConfigHandle);
    Snpe_RuntimeList_Add(inputRuntimeListHandle, runtime);

    Snpe_RuntimeConfig_Delete(runtimeConfigHandle);
  }
}

std::string QTIBackendHelper::get_snpe_version() {
  Snpe_DlVersion_Handle_t version = Snpe_Util_GetLibraryVersion();
  return Snpe_DlVersion_GetBuild(version);
}

std::vector<float> get_normal(unsigned numbers, unsigned seed = 5,
                              float mean = 0.0, float stddev = 1.0) {
  std::default_random_engine generator(seed);
  std::normal_distribution<float> distribution(mean, stddev);

  std::vector<float> d;
  for (unsigned i = 0; i < numbers; i++) d.push_back(distribution(generator));

  return d;
}

void QTIBackendHelper::initSd(const char *model_path, const char *lib_path) {
#ifdef STABLEDIFFUSION_FLAG
  bool use_mmap = false;  // we don't want to use cached
  uint64_t context_bin_mmap_read_budget = 100000;
  std::string temp(lib_path);
  native_lib_path = temp;
  std::string newtemp(model_path);
  data_folder_path = newtemp;

  // TODO: Below vars are using in preprocessInputSd
  // May need to be set from the configuration from MLC. Hardcoded for now.
  num_steps = 20;
  seed = 633994880;
  guidance_scale = 7.5;

  mlperf_data_t input;
  input.type = mlperf_data_t::Int32;
  input.size = 77 * 1;  // tokenized inputs 77 numbers
  inputFormat_.push_back(input);

  mlperf_data_t output;
  output.type = mlperf_data_t::Uint8;
  output.size = 512 * 512 * 3;
  outputFormat_.push_back(output);

  sd_pipeline = new QnnApiHelpers();

  if (0 != sd_pipeline->Init(data_folder_path, native_lib_path, 768, 77, 1.0,
                             512, 512, 3.0, use_mmap,
                             context_bin_mmap_read_budget)) {
    LOG(FATAL) << "Initialization Failure";
  }
#endif
}

bool QTIBackendHelper::preprocessInputSd(void *data) {
#ifdef STABLEDIFFUSION_FLAG
  int32_t *input_prompt_ids = (int32_t *)data;
  std::vector<float32_t> noise = get_normal(64 * 64 * 4, seed);
  return sd_pipeline->PreProcessInput(input_prompt_ids, noise, num_steps,
                                      guidance_scale);
#else
  return false;
#endif
}

bool QTIBackendHelper::executeSd() {
#ifdef STABLEDIFFUSION_FLAG
  for (int stepIdx = 0; stepIdx < num_steps; stepIdx++) {
    bool runVAE = ((stepIdx + 1) == num_steps);
    if (true != sd_pipeline->RunInference(runVAE)) {
      LOG(FATAL) << "RunInference failure";
      return false;
    }
  }
  return true;
#else
  return false;
#endif
}

bool QTIBackendHelper::getOutputSd(void **data) {
#ifdef STABLEDIFFUSION_FLAG
  JniHelpers::InferenceReturn inferenceReturn;
  if (true != sd_pipeline->PostProcessOutput(false, false, inferenceReturn)) {
    LOG(FATAL) << "PostProcessOutput failure";
    return false;
  }
  *data = inferenceReturn.m_ImageData;

  // delete sd_pipeline;
  // sd_pipeline = new QnnApiHelpers();
  return true;
#else
  return false;
#endif
}

void QTIBackendHelper::deinitSd() {
#ifdef STABLEDIFFUSION_FLAG
  bool use_mmap = false;  // we don't want to use cached
  uint64_t context_bin_mmap_read_budget = 100000;
  /*if (0 != sd_pipeline->Init(data_folder_path, native_lib_path,
                     768, 77, 1.0,
                     512, 512, 3.0,
                     use_mmap, context_bin_mmap_read_budget)) {
                     LOG(FATAL) << "Initialization Failure";
                     }
*/
  delete sd_pipeline;
  sd_pipeline = nullptr;
#endif
}

bool QTIBackendHelper::setupPerfHandle() {
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