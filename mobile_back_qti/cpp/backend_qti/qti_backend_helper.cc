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

#include "qti_backend_helper.h"

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

static Snpe_StringList_Handle_t ResolveOutputLayerNames(std::string &line) {
  Snpe_StringList_Handle_t outputLayersHandle = Snpe_StringList_Create();
  if (!line.empty()) {
    std::vector<std::string> names;
    split(names, line.substr(0), ',');
    for (auto &name : names)
      Snpe_StringList_Append(outputLayersHandle, name.c_str());
  }
  return outputLayersHandle;
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

static Snpe_Runtime_t Str2Delegate(const snpe_runtimes_t delegate) {
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
    LOG(FATAL) << "runtime " << delegate
               << " is not available on this platform";
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
        ResolveOutputLayerNames(snpeOutputLayers_);

    Snpe_SNPEBuilder_Handle_t snpeBuilderHandle =
        Snpe_SNPEBuilder_Create(containerHandle);
    dummyInputRuntimeListHandle = Snpe_RuntimeList_Create();
    Snpe_RuntimeList_Add(dummyInputRuntimeListHandle, SNPE_RUNTIME_CPU);
    Snpe_SNPEBuilder_SetPerformanceProfile(snpeBuilderHandle, perfProfile_);
    Snpe_SNPEBuilder_SetExecutionPriorityHint(snpeBuilderHandle,
                                              SNPE_EXECUTION_PRIORITY_HIGH);
    Snpe_SNPEBuilder_SetRuntimeProcessorOrder(snpeBuilderHandle,
                                              dummyInputRuntimeListHandle);
    Snpe_SNPEBuilder_SetOutputLayers(snpeBuilderHandle, outputLayers);

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
    if (Snpe_SNPE_RegisterIonBuffers(snpe_->snpeHandle, ionBufferMapHandle_) ==
        SNPE_SUCCESS)
      LOG(INFO) << "Ion Buffer Registration Successful";
    isIonRegistered = true;
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
        ResolveOutputLayerNames(snpeOutputLayers_);
    Snpe_SNPEBuilder_SetPerformanceProfile(snpeBuilderHandle, perfProfile_);
    Snpe_SNPEBuilder_SetProfilingLevel(snpeBuilderHandle, profilingLevel_);
    Snpe_SNPEBuilder_SetExecutionPriorityHint(snpeBuilderHandle,
                                              SNPE_EXECUTION_PRIORITY_HIGH);
    Snpe_SNPEBuilder_SetRuntimeProcessorOrder(snpeBuilderHandle,
                                              inputRuntimeListHandle);
    Snpe_SNPEBuilder_SetUseUserSuppliedBuffers(snpeBuilderHandle, true);
    Snpe_SNPEBuilder_SetOutputLayers(snpeBuilderHandle, outputLayers);

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

      // LOG(INFO) << "inputbuffer: " << inputBufferType_ << " name: " << name;
      if (inputBufferType_ == QTIBufferType::FLOAT_32) {
        // Prepare float buffer
        bufSize *= sizeof(float);
        std::vector<uint8_t> inputBuffer(bufSize);
        auto stridesHandle = calcStrides(
            Snpe_IBufferAttributes_GetDims(ubaOptHandle), sizeof(float));
        Snpe_UserBufferEncoding_Handle_t ubeFloatHandle =
            Snpe_UserBufferEncodingFloat_Create();
        ubPtr.push_back(Snpe_Util_CreateUserBuffer(
            std::move(inputBuffer.data()), inputBuffer.size(), stridesHandle,
            ubeFloatHandle));
        Snpe_UserBufferMap_Add(inputMapHandle, name, ubPtr.back());

        Snpe_TensorShape_Delete(stridesHandle);
        Snpe_UserBufferEncodingFloat_Delete(ubeFloatHandle);
      } else {
        // Prepare tf8 buffer
        bufSize *= sizeof(uint8_t);
        // Pass the quantization parameters from the model to UB
        std::vector<uint8_t> inputBuffer(bufSize);
        auto stridesHandle = calcStrides(
            Snpe_IBufferAttributes_GetDims(ubaOptHandle), sizeof(uint8_t));
        auto ubeTfN = Snpe_IUserBuffer_GetEncoding_Ref(ubaOptHandle);

        // Set the default QP for model which doesn't have QP.
        // HTP may not need to use the QP for the current set of DLCs
        // This may be required for AIP though.
        // LOG(INFO) << "QP parameters from model: " << ubeTfN;
        if (!ubeTfN)
          ubeTfN = Snpe_UserBufferEncodingTfN_Create(128.0, 1.0 / 255, 8);

        ubPtr.push_back(Snpe_Util_CreateUserBuffer(
            std::move(inputBuffer.data()), inputBuffer.size(), stridesHandle,
            ubeTfN));
        Snpe_UserBufferMap_Add(inputMapHandle, name, ubPtr.back());

        Snpe_TensorShape_Delete(stridesHandle);
        Snpe_UserBufferEncodingTfN_Delete(ubeTfN);
      }
      Snpe_IBufferAttributes_Delete(ubaOptHandle);
      Snpe_TensorShape_Delete(dimsHandle);
    }
    Snpe_UserBufferList_PushBack(inputMapListHandle_, inputMapHandle);
  }
  bufs_.resize(batchSize_ / inputBatch_);
  Snpe_UserBufferMap_Delete(inputMapHandle);
}

void QTIBackendHelper::map_outputs() {
  Snpe_UserBufferMap_Handle_t outputMapHandle = Snpe_UserBufferMap_Create();

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
      // LOG(INFO) << "outputBufferType: " << outputBufferType_
      //          << " name: " << name;
      if (useIonBuffers_) {
        Allocator<uint8_t>::useIonAllocator();
      } else {
        Allocator<uint8_t>::useDefaultAllocator();
      }
      if (outputBufferType_ == QTIBufferType::UINT_8) {
        auto ubeTfN = Snpe_UserBufferEncodingTfN_Create(0, 1.0f, 8);

        std::vector<Snpe_IUserBuffer_Handle_t> x;
        bufs_[bi].emplace(std::string(name),
                          std::vector<uint8_t, Allocator<uint8_t>>(
                              bufSize * sizeof(uint8_t)));
        auto stridesHandle = calcStrides(
            Snpe_IBufferAttributes_GetDims(ubaOptHandle), sizeof(uint8_t));
        x.push_back(Snpe_Util_CreateUserBuffer(bufs_[bi].at(name).data(),
                                               bufSize * sizeof(uint8_t),
                                               stridesHandle, ubeTfN));
        Snpe_UserBufferMap_Add(outputMapHandle, name, x.back());
        if (useIonBuffers_)
          Snpe_UserMemoryMap_Add(ionBufferMapHandle_, name,
                                 bufs_[bi].at(name).data());

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
          Snpe_UserMemoryMap_Add(ionBufferMapHandle_, name,
                                 bufs_[bi].at(name).data());

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
        if (useIonBuffers_)
          Snpe_UserMemoryMap_Add(ionBufferMapHandle_, name,
                                 bufs_[bi].at(name).data());

        Snpe_UserBufferEncodingFloat_Delete(userBufferEncodingFloat);
        Snpe_TensorShape_Delete(stridesHandle);
      }
      Snpe_IBufferAttributes_Delete(ubaOptHandle);
      Snpe_TensorShape_Delete(dimsHandle);
    }
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
      if (snpeOutputLayers_ == "transpose") {
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
      runtime = Str2Delegate(SNPE_DSP);
    }
    auto runtimeConfigHandle = Snpe_RuntimeConfig_Create();

    Snpe_RuntimeConfig_SetRuntime(runtimeConfigHandle, runtime);
    Snpe_RuntimeConfig_SetPerformanceProfile(runtimeConfigHandle, perfProfile_);
    Snpe_RuntimeConfigList_PushBack(runtimeConfigsListHandle,
                                    runtimeConfigHandle);
    Snpe_RuntimeList_Add(inputRuntimeListHandle, runtime);

    Snpe_RuntimeConfig_Delete(runtimeConfigHandle);
  }

  for (int i = 0; i < numGPU; i++) {
    if (i == 0) {
      runtime = Str2Delegate(SNPE_GPU);
    }
    auto runtimeConfigHandle = Snpe_RuntimeConfig_Create();
    Snpe_RuntimeConfig_SetRuntime(runtimeConfigHandle, runtime);
    Snpe_RuntimeConfig_SetPerformanceProfile(runtimeConfigHandle, perfProfile_);
    Snpe_RuntimeConfigList_PushBack(runtimeConfigsListHandle,
                                    runtimeConfigHandle);
    Snpe_RuntimeList_Add(inputRuntimeListHandle, runtime);
    Snpe_RuntimeConfig_Delete(runtimeConfigHandle);
  }

  for (int i = 0; i < numCPU; i++) {
    if (i == 0) {
      runtime = Str2Delegate(SNPE_CPU);
    }
    auto runtimeConfigHandle = Snpe_RuntimeConfig_Create();
    Snpe_RuntimeConfig_SetRuntime(runtimeConfigHandle, runtime);
    Snpe_RuntimeConfig_SetPerformanceProfile(runtimeConfigHandle, perfProfile_);
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
    Snpe_RuntimeConfig_SetPerformanceProfile(runtimeConfigHandle, perfProfile_);
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
