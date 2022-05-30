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

#include "qti_backend_helper.h"

#include <string>
#include <vector>

#include "DiagLog/IDiagLog.hpp"
#include "DlContainer/IDlContainer.hpp"
#include "DlSystem/DlEnums.hpp"
#include "DlSystem/DlError.hpp"
#include "DlSystem/IBufferAttributes.hpp"
#include "DlSystem/IUserBuffer.hpp"
#include "DlSystem/PlatformConfig.hpp"
#include "DlSystem/StringList.hpp"
#include "DlSystem/TensorMap.hpp"
#include "DlSystem/TensorShape.hpp"
#include "DlSystem/TensorShapeMap.hpp"
#include "DlSystem/UserBufferMap.hpp"
#include "SNPE/SNPEBuilder.hpp"
#include "SNPE/SNPEFactory.hpp"
#include "SNPE/UserBufferList.hpp"
#include "absl/strings/ascii.h"
#include "cpuctrl.h"
#include "tensorflow/core/platform/logging.h"
#include "tflite_c.h"

int isSignedStatus = DEFAULT;

enum snpe_runtimes_t { SNPE_DSP = 0, SNPE_AIP = 1, SNPE_GPU = 2, SNPE_CPU = 3 };

static long calcSizeFromDims(const size_t rank,
                             const zdl::DlSystem::Dimension *dims) {
  if (rank == 0) return 0;
  size_t size = 1;
  for (size_t i = rank; i > 0; i--) {
    if (*dims != 0)
      size *= *dims;
    else
      size *= 10;
    dims++;
  }
  return (long)size;
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

static zdl::DlSystem::StringList ResolveOutputLayerNames(std::string &line) {
  zdl::DlSystem::StringList outputLayers;
  if (!line.empty()) {
    std::vector<std::string> names;
    split(names, line.substr(0), ',');
    for (auto &name : names) outputLayers.append(name.c_str());
  }
  return outputLayers;
}

static std::vector<size_t> calcStrides(zdl::DlSystem::TensorShape dims,
                                       size_t elementSize) {
  std::vector<size_t> strides(dims.rank());
  strides[strides.size() - 1] = elementSize;
  size_t stride = strides[strides.size() - 1];
  for (size_t i = dims.rank() - 1; i > 0; i--) {
    if (dims[i] != 0)
      stride *= dims[i];
    else
      stride *= 10;
    strides[i - 1] = stride;
  }
  return strides;
}

static zdl::DlSystem::Runtime_t Str2Delegate(const snpe_runtimes_t delegate) {
  zdl::DlSystem::Runtime_t runtime;
  bool isDSP = false;

  switch (delegate) {
    case SNPE_DSP:
      runtime = zdl::DlSystem::Runtime_t::DSP;
      isDSP = true;
      break;
    case SNPE_AIP:
      runtime = zdl::DlSystem::Runtime_t::AIP_FIXED_TF;
      isDSP = true;
      break;
    case SNPE_GPU:
      runtime = zdl::DlSystem::Runtime_t::GPU;
      break;
    case SNPE_CPU:
      runtime = zdl::DlSystem::Runtime_t::CPU;
      break;
    default:
      LOG(ERROR) << "runtime not supported";
      break;
  }

  if (isDSP) {
    if (isSignedStatus == DEFAULT) {
      if (zdl::SNPE::SNPEFactory::isRuntimeAvailable(
              runtime, zdl::DlSystem::RuntimeCheckOption_t::UNSIGNEDPD_CHECK)) {
        isSignedStatus = UNSIGNED_PD;
        LOG(INFO) << "runtime " << delegate
                  << " is available on this platform with UnsignedPD";
      } else {
        if (zdl::SNPE::SNPEFactory::isRuntimeAvailable(
                runtime, zdl::DlSystem::RuntimeCheckOption_t::NORMAL_CHECK)) {
          isSignedStatus = SIGNED_PD;
          LOG(INFO) << "runtime " << delegate
                    << " is available on this platform with SignedPD";
        } else {
          // This platform doesn't support DSP runtime
          LOG(FATAL) << "runtime " << delegate
                     << " is not available on this platform";
        }
      }
    }

    return runtime;
  } else {
    if (!zdl::SNPE::SNPEFactory::isRuntimeAvailable(runtime)) {
      LOG(FATAL) << "runtime " << delegate
                 << " is not available on this platform";
    } else {
      LOG(INFO) << "runtime " << delegate << " is available on this platform";
      return runtime;
    }
  }
}

void QTIBackendHelper::use_psnpe(const char *model_path) {
  uint32_t soc_id = CpuCtrl::getSocId();
  uint32_t numInits = get_num_inits();
  LOG(INFO) << "numInits: " << numInits;

  bool psnpe_buildStatus = false;
  // Init cache generated on device giving better performance.
  // So build the DLC twice to use the init cache generated in the first run
  for (int i = 0; i < numInits; i++) {
    // Open the DL container that contains the network to execute.
    std::unique_ptr<zdl::DlContainer::IDlContainer> container;
    // destroys previous snpe instance and creates a new one.
    psnpe_.reset(new zdl::PSNPE::PSNPE());
    // Loads the container after destroying the previous instance
    container = zdl::DlContainer::IDlContainer::open(model_path);
    if (!container) {
      LOG(FATAL) << "Container is not available " << model_path;
    }

    zdl::PSNPE::BuildConfig buildConfig;
    buildConfig.container = container.get();
    buildConfig.runtimeConfigList = runtimeConfigsList;
    buildConfig.inputOutputTransmissionMode =
        zdl::PSNPE::InputOutputTransmissionMode::sync;

    zdl::DlSystem::StringList outputLayers =
        ResolveOutputLayerNames(snpeOutputLayers_);

    zdl::SNPE::SNPEBuilder snpeBuilder(container.get());
    dummyInputRuntimeList.add(zdl::DlSystem::Runtime_t::CPU);
    snpeBuilder.setPerformanceProfile(perfProfile_)
        .setExecutionPriorityHint(zdl::DlSystem::ExecutionPriorityHint_t::HIGH)
        .setRuntimeProcessorOrder(dummyInputRuntimeList)
        .setOutputLayers(outputLayers);

    if (outputLayers.size() > 0) {
      buildConfig.outputBufferNames = outputLayers;
    }

    std::string platformOptionStr = "";
    if (useDspFeatures && isSignedStatus == UNSIGNED_PD) {
      // use unsignedPD feature for untrusted app.
      platformOptionStr += "unsignedPD:ON";
    }

    // These features are not for SDM865, so turning them off.
    if (useDspFeatures &&
        (soc_id == SDM888 || soc_id == SDM778 || soc_id == SD8G1)) {
      // use Zero copy for input and output buffers.
      // Requires rpc registered ion buffers.
      if (useIonBuffers_) {
        platformOptionStr += ";useDspZeroCopy:ON";
      }
      platformOptionStr += ";dspPowerSettingContext:ON";
      buildConfig.enableInitCache = true;
    }
    buildConfig.platformOptions = platformOptionStr;

    zdl::DlSystem::PlatformConfig platformConfig;
    bool setSuccess = platformConfig.setPlatformOptions(platformOptionStr);
    bool isValid = platformConfig.isOptionsValid();
    if (!isValid) {
      LOG(INFO) << "platformconfig option is invalid";
    }
    snpeBuilder.setPlatformConfig(platformConfig);
    snpe_ = snpeBuilder.build();

    psnpe_buildStatus = psnpe_->build(buildConfig);

    // Saves the container if there is a modification in any of the record.
    if (numInits > 1) container->save(model_path);
  }

  if (!psnpe_buildStatus) {
    LOG(FATAL) << "Error in init of psnpe_ ";
  }
  if (!snpe_) {
    LOG(FATAL) << "Error in init of snpe_ " << snpe_;
  }
}

void QTIBackendHelper::use_snpe(const char *model_path) {
  uint32_t soc_id = CpuCtrl::getSocId();
  uint32_t numInits = get_num_inits();
  LOG(INFO) << "numInits: " << numInits;

  // Use SNPE
  for (int i = 0; i < numInits; i++) {
    // Open the DL container that contains the network to execute.
    std::unique_ptr<zdl::DlContainer::IDlContainer> container;
    // Loads the container after destroying the previous instance
    container = zdl::DlContainer::IDlContainer::open(model_path);
    if (!container) {
      LOG(FATAL) << "Container is not available " << model_path;
    }

    zdl::SNPE::SNPEBuilder snpeBuilder(container.get());
    zdl::DlSystem::StringList outputLayers =
        ResolveOutputLayerNames(snpeOutputLayers_);

    snpeBuilder.setPerformanceProfile(perfProfile_)
        .setExecutionPriorityHint(zdl::DlSystem::ExecutionPriorityHint_t::HIGH)
        .setRuntimeProcessorOrder(inputRuntimeList)
        .setUseUserSuppliedBuffers(true)
        .setOutputLayers(outputLayers);

    std::string platformOptionStr = "";
    if (useDspFeatures && isSignedStatus == UNSIGNED_PD) {
      // use unsignedPD feature for untrusted app.
      platformOptionStr += "unsignedPD:ON";
    }

    // These features are not for SDM865, so turning them off.
    if (useDspFeatures &&
        (soc_id == SDM888 || soc_id == SDM778 || soc_id == SD8G1)) {
      // use Zero copy for input and output buffers.
      // Requires rpc registered ion buffers.
      if (useIonBuffers_) {
        platformOptionStr += ";useDspZeroCopy:ON";
      }
      platformOptionStr += ";dspPowerSettingContext:ON";
      snpeBuilder.setInitCacheMode(true);
    }
    zdl::DlSystem::PlatformConfig platformConfig;
    bool setSuccess = platformConfig.setPlatformOptions(platformOptionStr);
    bool isValid = platformConfig.isOptionsValid();
    if (!isValid) {
      LOG(INFO) << "platformconfig option is invalid";
    }
    snpeBuilder.setPlatformConfig(platformConfig);
    snpe_ = snpeBuilder.build();

    // Saves the container if there is a modification in any of the record.
    if (numInits > 1) container->save(model_path);
  }
  if (!snpe_) {
    LOG(FATAL) << "Error in init of the model " << snpe_;
  }
}

inline int QTIBackendHelper::get_num_inits() {
  uint32_t soc_id = CpuCtrl::getSocId();
  if (!useDspFeatures || soc_id == SDM865) {
    return 1;
  } else {
    return 2;
  }
}

void QTIBackendHelper::get_accelerator_instances(int &num_dsp, int &num_aip,
                                                 int &num_gpu, int &num_cpu) {
  uint32_t soc_id = CpuCtrl::getSocId();
  std::string &delegate = delegate_;
  num_dsp = 0;
  num_aip = 0;
  num_gpu = 0;
  num_cpu = 0;
  if (scenario_ == "Offline") {
    // For 865 use DSP+AIP
    if (soc_id == SDM865) {
      if (delegate != "snpe_aip" && delegate != "psnpe_aip") {
        LOG(FATAL) << "Error: Unsupported delegate for offline mode";
      }
      useDspFeatures = true;
      num_dsp = 1;
      num_aip = 6;
    } else if (soc_id == SDM888) {
      if (delegate != "snpe_dsp" && delegate != "psnpe_dsp") {
        LOG(FATAL) << "Error: Unsupported delegate for offline mode";
      }
      useDspFeatures = true;
      num_dsp = 2;
      num_gpu = 4;
    } else if (soc_id == SDM778) {
      if (delegate != "snpe_dsp" && delegate != "psnpe_dsp") {
        LOG(FATAL) << "Error: Unsupported delegate for offline mode";
      }
      useDspFeatures = true;
      num_dsp = 2;
      num_gpu = 0;
    } else if (soc_id == SD8G1) {
      if (delegate != "snpe_dsp" && delegate != "psnpe_dsp") {
        LOG(FATAL) << "Error: Unsupported delegate for offline mode";
      }
      useDspFeatures = true;
      num_dsp = 2;
      num_aip = 0;
      num_gpu = 0;
    }
  } else {
    if (delegate == "snpe_dsp" || delegate == "psnpe_dsp") {
      num_dsp = 1;
      useDspFeatures = true;
    } else if (delegate == "snpe_aip" || delegate == "psnpe_aip") {
      num_aip = 1;
      useDspFeatures = true;
    } else if (delegate == "snpe_gpu" || delegate == "psnpe_gpu") {
      num_gpu = 1;
      useDspFeatures = false;
    } else if (delegate == "snpe_cpu" || delegate == "psnpe_cpu") {
      num_cpu = 1;
      useDspFeatures = false;
    } else {
      LOG(FATAL) << "Error: Unsupported delegate " << delegate << " SoC ID "
                 << soc_id;
    }
  }
  LOG(INFO) << "Using " << num_dsp << " dsp " << num_aip << " aip " << num_gpu
            << " gpu and " << num_cpu << " cpu";
}

void QTIBackendHelper::map_inputs() {
  zdl::DlSystem::UserBufferMap inputMap;
  for (int bi = 0; bi < batchSize_ / inputBatch_; bi++) {
    for (const auto &name : networkInputTensorNames_) {
      zdl::DlSystem::IUserBufferFactory &ubFactory =
          zdl::SNPE::SNPEFactory::getUserBufferFactory();
      auto ubaOpt = snpe_->getInputOutputBufferAttributes(name);
      long bufSize = calcSizeFromDims((*ubaOpt)->getDims().rank(),
                                      (*ubaOpt)->getDims().getDimensions());
      std::vector<size_t> strides;
      std::unique_ptr<zdl::DlSystem::IUserBuffer> ubPtr;

      LOG(INFO) << "inputbuffer: " << inputBufferType_ << " name: " << name;
      if (inputBufferType_ == QTIBufferType::FLOAT_32) {
        // Prepare float buffer
        bufSize *= sizeof(float);
        std::vector<uint8_t> inputBuffer(bufSize);
        strides = calcStrides((*ubaOpt)->getDims(), sizeof(float));
        zdl::DlSystem::UserBufferEncodingFloat ubeFloat;

        ubPtr =
            ubFactory.createUserBuffer(std::move(inputBuffer.data()),
                                       inputBuffer.size(), strides, &ubeFloat);
        inputMap.add(name, ubPtr.release());
      } else {
        // Prepare tf8 buffer
        bufSize *= sizeof(uint8_t);
        // Pass the quantization parameters from the model to UB
        zdl::DlSystem::UserBufferEncodingTfN *ubeTfN = nullptr;
        zdl::DlSystem::UserBufferEncodingTfN temp(128.0, 1.0 / 255);
        std::vector<uint8_t> inputBuffer(bufSize);
        strides = calcStrides((*ubaOpt)->getDims(), sizeof(uint8_t));
        ubeTfN = dynamic_cast<zdl::DlSystem::UserBufferEncodingTfN *>(
            (*ubaOpt)->getEncoding());

        // Set the default QP for model which doesn't have QP.
        // HTP may not need to use the QP for the current set of DLCs
        // This may be required for AIP though.
        // LOG(INFO) << "QP parameters from model: " << ubeTfN;
        if (!ubeTfN) ubeTfN = &temp;

        ubPtr = ubFactory.createUserBuffer(std::move(inputBuffer.data()),
                                           inputBuffer.size(), strides, ubeTfN);
        inputMap.add(name, ubPtr.release());
      }
    }
    inputMap_.push_back(inputMap);
  }
  bufs_.resize(batchSize_ / inputBatch_);
}

void QTIBackendHelper::map_outputs() {
  zdl::DlSystem::UserBufferMap outputMap;
  zdl::DlSystem::IUserBufferFactory &ubFactory =
      zdl::SNPE::SNPEFactory::getUserBufferFactory();
  for (int bi = 0; bi < batchSize_ / inputBatch_; bi++) {
    for (const auto &name : networkOutputTensorNames_) {
      auto ubaOpt = snpe_->getInputOutputBufferAttributes(name);
      long bufSize = calcSizeFromDims((*ubaOpt)->getDims().rank(),
                                      (*ubaOpt)->getDims().getDimensions());

      outputBatchBufsize_ = bufSize;
      LOG(INFO) << "outputBufferType: " << outputBufferType_
                << " name: " << name;
      if (useIonBuffers_) {
        Allocator<uint8_t>::useIonAllocator();
      } else {
        Allocator<uint8_t>::useDefaultAllocator();
      }
      if (outputBufferType_ == QTIBufferType::UINT_8) {
        zdl::DlSystem::UserBufferEncodingTfN ubeTfN(0, 1.0f, 8);
        bufs_[bi].emplace(std::string(name),
                          std::vector<uint8_t, Allocator<uint8_t>>(
                              bufSize * sizeof(uint8_t)));
        auto x = ubFactory.createUserBuffer(
            bufs_[bi].at(name).data(), bufSize * sizeof(uint8_t),
            calcStrides((*ubaOpt)->getDims(), sizeof(uint8_t)), &ubeTfN);
        outputMap.add(name, x.release());
      } else {
        zdl::DlSystem::UserBufferEncodingFloat userBufferEncodingFloat;
        bufs_[bi].emplace(
            std::string(name),
            std::vector<uint8_t, Allocator<uint8_t>>(bufSize * sizeof(float)));
        auto x = ubFactory.createUserBuffer(
            bufs_[bi].at(name).data(), bufSize * sizeof(float),
            calcStrides((*ubaOpt)->getDims(), sizeof(float)),
            &userBufferEncodingFloat);
        outputMap.add(name, x.release());
      }
    }
    outputMap_.push_back(outputMap);
  }
}

void QTIBackendHelper::get_data_formats() {
  const auto &strList_input = snpe_->getInputTensorNames();
  if (!strList_input) {
    throw std::runtime_error("Error obtaining Input tensor names");
  }
  networkInputTensorNames_ = *strList_input;

  zdl::DlSystem::TensorShape tensorShape;
  tensorShape = snpe_->getInputDimensions();
  inputBatch_ = tensorShape.getDimensions()[0];

  for (const auto &name : networkInputTensorNames_) {
    auto ubaOpt = snpe_->getInputOutputBufferAttributes(name);
    long bufSize = calcSizeFromDims((*ubaOpt)->getDims().rank(),
                                    (*ubaOpt)->getDims().getDimensions());
    if (inputBufferType_ == FLOAT_32) {
      // Input buffer type FLOAT
      inputFormat_.push_back(
          {mlperf_data_t::Type::Float32, bufSize / inputBatch_});
    } else {
      // Input buffer type UINT8
      inputFormat_.push_back(
          {mlperf_data_t::Type::Uint8, bufSize / inputBatch_});
    }
  }

  const auto &strList_output = snpe_->getOutputTensorNames();
  if (!strList_output) {
    throw std::runtime_error("Error obtaining Output tensor names");
  }
  networkOutputTensorNames_ = *strList_output;

  for (const auto &name : networkOutputTensorNames_) {
    auto ubaOpt = snpe_->getInputOutputBufferAttributes(name);
    long bufSize = calcSizeFromDims((*ubaOpt)->getDims().rank(),
                                    (*ubaOpt)->getDims().getDimensions());
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
    } else {
      // output buffer type UINT8
      outputFormat_.push_back(
          {mlperf_data_t::Type::Uint8, bufSize / inputBatch_});
    }
  }
}

void QTIBackendHelper::set_runtime_config() {
  int numDSP = 0, numAIP = 0, numGPU = 0, numCPU = 0;
  get_accelerator_instances(numDSP, numAIP, numGPU, numCPU);

  zdl::DlSystem::Runtime_t runtime;
  for (int i = 0; i < numDSP; i++) {
    if (i == 0) {
      runtime = Str2Delegate(SNPE_DSP);
    }
    zdl::PSNPE::RuntimeConfig runtimeConfig;
    runtimeConfig.runtime = runtime;
    runtimeConfig.perfProfile = perfProfile_;
    runtimeConfigsList.push_back(runtimeConfig);
    inputRuntimeList.add(runtime);
  }

  for (int i = 0; i < numAIP; i++) {
    if (i == 0) {
      runtime = Str2Delegate(SNPE_AIP);
    }
    zdl::PSNPE::RuntimeConfig runtimeConfig;
    runtimeConfig.runtime = runtime;
    runtimeConfig.perfProfile = perfProfile_;
    runtimeConfigsList.push_back(runtimeConfig);
    inputRuntimeList.add(runtime);
  }

  for (int i = 0; i < numGPU; i++) {
    if (i == 0) {
      runtime = Str2Delegate(SNPE_GPU);
    }
    zdl::PSNPE::RuntimeConfig runtimeConfig;
    runtimeConfig.runtime = runtime;
    runtimeConfig.perfProfile = perfProfile_;
    runtimeConfigsList.push_back(runtimeConfig);
    inputRuntimeList.add(runtime);
  }

  for (int i = 0; i < numCPU; i++) {
    if (i == 0) {
      runtime = Str2Delegate(SNPE_CPU);
    }
    zdl::PSNPE::RuntimeConfig runtimeConfig;
    runtimeConfig.runtime = runtime;
    runtimeConfig.perfProfile = perfProfile_;
    runtimeConfigsList.push_back(runtimeConfig);
    inputRuntimeList.add(runtime);
  }
}

std::string QTIBackendHelper::get_snpe_version() {
  zdl::DlSystem::Version_t version =
      zdl::SNPE::SNPEFactory::getLibraryVersion();
  return version.Build;
}
