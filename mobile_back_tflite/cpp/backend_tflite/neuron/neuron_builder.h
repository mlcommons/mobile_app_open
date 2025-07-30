/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */
/* MediaTek Inc. (C) 2023. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK
 * SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO
 * RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN
 * FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER
 * WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
 * ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER
 * TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/**
 * @file neuron_backend.cc
 */

#pragma once

#include <stdlib.h>
#include <string.h>

#include <fstream>

#include "NeuronAdapter.h"
#include "NeuronAdapterShim.h"
#include "neuron_backend.h"
#include "tensorflow/core/platform/logging.h"

typedef enum {
  TYPE_INPUT = 0,
  TYPE_OUTPUT,
};

typedef enum {
  AHWB_OFTEN = 0,
  AHWB_RARELY,
};

inline uint32_t GetNeuronTypeSize(int type) {
  int size = 1;
  switch (type) {
    case NEURON_FLOAT32:
    case NEURON_INT32:
    case NEURON_UINT32:
    case NEURON_TENSOR_FLOAT32:
    case NEURON_TENSOR_INT32:
      size = 4;
      break;
    case NEURON_TENSOR_QUANT16_SYMM:
    case NEURON_TENSOR_FLOAT16:
    case NEURON_FLOAT16:
    case NEURON_TENSOR_QUANT16_ASYMM:
      size = 2;
      break;
    case NEURON_TENSOR_BOOL8:
    case NEURON_TENSOR_QUANT8_SYMM:
    case NEURON_TENSOR_QUANT8_ASYMM:
    case NEURON_TENSOR_QUANT8_ASYMM_SIGNED:
      size = 1;
      break;
    default:
      LOG(ERROR) << "Get Neuron Type Error";
      size = 0;
  }
  return size;
}

inline uint32_t GetNeuronOperandSize(NeuronOperandType op) {
  uint32_t shapeElements = 1;
  for (int i = 0; i < op.dimensionCount; i++) {
    shapeElements *= op.dimensions[i];
  }
  return shapeElements * GetNeuronTypeSize(op.type);
}

inline int load_file(const char *dla_path, uint8_t **buffer) {
  // read dla raw data
  std::ifstream input(dla_path, std::ios::binary);
  if (!input) {
    LOG(ERROR) << "open " << dla_path << " fail";
    return 0;
  }

  // get length of file:
  input.seekg(0, input.end);
  int length = input.tellg();
  input.seekg(0, input.beg);

  // backend_data->buffer = new char[length];
  char *tmp = (char *)malloc(length);

  LOG(INFO) << "Reading " << length << " characters from: " << dla_path;
  // read data as a block:
  input.read(tmp, length);
  input.close();
  *buffer = (uint8_t *)tmp;
  return length;
}

struct NeuronDeleter {
  void operator()(NeuronModel *model) {
    if (model != nullptr) {
      NeuronModel_free(model);
    }
  }
  void operator()(NeuronCompilation *compilation) {
    if (compilation != nullptr) {
      NeuronCompilation_free(compilation);
    }
  }
  void operator()(NeuronExecution *execution) {
    if (execution != nullptr) {
      NeuronExecution_free(execution);
    }
  }
  void operator()(NeuronMemory *memory) {
    if (memory != nullptr) {
      NeuronMemory_free(memory);
    }
  }
};

struct BufferDeleter {
  void operator()(AHardwareBuffer *buffer) {
    if (buffer != nullptr) {
      AHardwareBuffer_unlock(buffer, nullptr);
      AHardwareBuffer_release(buffer);
    }
  }
};

class NeuronBuilder {
 public:
  ~NeuronBuilder() {}

  NeuronBuilder &setInputOperand(std::vector<NeuronOperandType> &input) {
    mInputOperand = input;
    return *this;
  }

  NeuronBuilder &setOutputOperand(std::vector<NeuronOperandType> &output) {
    mOutputOperand = output;
    return *this;
  }

  NeuronBuilder &setSuppressConversion(int type, bool res) {
    if (type == TYPE_INPUT) {
      mInputSuppress = res;
    } else {
      mOutputSuppress = res;
    }
    return *this;
  }

  NeuronBuilder &setAhwbType(int res) {
    if (res == AHWB_RARELY) {
      mAhwbType = AHARDWAREBUFFER_USAGE_CPU_READ_RARELY |
                  AHARDWAREBUFFER_USAGE_CPU_WRITE_RARELY;
    } else {
      mAhwbType = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN |
                  AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
    }
    return *this;
  }

  NeuronBuilder &setExecutionFlushValue(uint8_t value) {
    mNeuronExecutionFlushValue = value;
    return *this;
  }

  NeuronBuilder &setThreadNumber(uint32_t num) {
    mThreadNum = num;
    return *this;
  }

  NeuronBuilder &setShardsNumber(uint32_t num) {
    mShardsNum = num;
    return *this;
  }

  NeuronBuilder &setUseThroughputMode(bool use){
    mUseThroughputMode = use;
    return *this;
  }

  NeuronBuilder &setModelPath(const char *dla_path) {
    mModelPath = dla_path;
    return *this;
  }

  NeuronBuilder &setPreference(NeuronAdapterPreferenceCode preference) {
    mPreference = preference;
    return *this;
  }

  bool create(neuron_backend_ptr_t backend_ptr) {
    if (!createNeuron()) {
      return false;
    }
    if (!createMemory(TYPE_INPUT)) {
      return false;
    }
    if (!createMemory(TYPE_OUTPUT)) {
      return false;
    }

    return moveToBackend(backend_ptr);
  }

 private:
  bool createNeuron() {
    NeuronModel *model = nullptr;
    NeuronCompilation *compilation = nullptr;
    NeuronExecution *execution = nullptr;

    LOG(INFO) << "createNeuron load DLA from file: " << mModelPath;
    uint8_t *dlaBuffer = nullptr;
    int length = load_file(mModelPath.c_str(), &dlaBuffer);
    if (length == 0) {
      return false;
    }
    LOG(INFO) << "createNeuron load DLA end Successfully with buffer length: " << length;
    // ---------------------------Model------------------------------------
    LOG(INFO) << "createNeuron create Model";
    int err = NEURON_NO_ERROR;
    err = NeuronModel_create(&model);
    RETURN_FALSE_ON_ERR(err, "Create Neuron Model Failed");
    mModel = std::unique_ptr<NeuronModel, NeuronDeleter>(model);

    std::vector<uint32_t> input_op_number;
    for (int i = 0; i < mInputOperand.size(); i++) {
      err = NeuronModel_addOperand(model, &mInputOperand[i]);
      RETURN_FALSE_ON_ERR(err, "Add Input: [ " << i << " ] Operand Failed");
      input_op_number.emplace_back(i);
    }

    int32_t operandType = 0;
    const uint16_t network_operand_restore_data = RESTORE_DLA_EXTENSION_OPERAND_TYPE;
    const char *extensionRestroeCompiledNetwork = RESTORE_DLA_EXTENSION_NAME;
    err = NeuronModel_getExtensionOperandType(
        model, extensionRestroeCompiledNetwork, network_operand_restore_data,
        &operandType);
    RETURN_FALSE_ON_ERR(err, "Get Extension Operand Type Failed");

    NeuronOperandType extenOperandType{
        .type = operandType,
        .scale = 0.0f,
        .zeroPoint = 0,
        .dimensionCount = 0,
    };

    err = NeuronModel_addOperand(model, &extenOperandType);
    RETURN_FALSE_ON_ERR(err, "Add DLA Operand Failed");
    input_op_number.emplace_back(input_op_number.size());

    std::vector<uint32_t> output_op_number;
    for (int i = 0; i < mOutputOperand.size(); i++) {
      err = NeuronModel_addOperand(model, &mOutputOperand[i]);
      RETURN_FALSE_ON_ERR(err, "Add Output: [ " << i << " ] Operand Failed");
      output_op_number.emplace_back(i + input_op_number.size());
    }
    err = NeuronModel_setOperandValue(model, input_op_number.back(), dlaBuffer,
                                         length);
    RETURN_FALSE_ON_ERR(err, "Set Dla Operand Value failed");
    int32_t operationType = 0;
    const uint16_t network_operation_type_restore = RESTORE_DLA_EXTENSION_OPERATION_TYPE;
    err = NeuronModel_getExtensionOperationType(
        model, extensionRestroeCompiledNetwork, network_operation_type_restore,
        &operationType);
    RETURN_FALSE_ON_ERR(err, "get ExtensionOperationType fail");

    // Add extension operation
    err = NeuronModel_addOperation(
        model, (NeuronOperationType)operationType,
        input_op_number.size(), input_op_number.data(),
        output_op_number.size(), output_op_number.data());
    RETURN_FALSE_ON_ERR(err, "get addOperation fail");

    // Identify input and output
    err = NeuronModel_identifyInputsAndOutputs(
        model, input_op_number.size() - 1, input_op_number.data(),
        output_op_number.size(), output_op_number.data());
    RETURN_FALSE_ON_ERR(err, "get identify input and output fail");

    if (mInputSuppress) {
      err = NeuronModel_suppressInputConversion(model, true);
      RETURN_FALSE_ON_ERR(err, "Subpress input conversion failed");
    }

    if (mOutputSuppress) {
      err = NeuronModel_suppressOutputConversion(model, true);
      RETURN_FALSE_ON_ERR(err, "Subpress output conversion failed");
    }

    err = NeuronModel_finish(model);
    RETURN_FALSE_ON_ERR(err, "Restore DLA model failed");
    LOG(INFO) << "createNeuron Finish Model End Successfully";
    // ---------------------------Compilation------------------------------------
    CompilationType compilationType = COMPILATION_TYPE_NORMAL;
    if (!mUseThroughputMode) {
      LOG(INFO) << "createNeuron Compiltion Type COMPILATION_TYPE_NORMAL";
    } else {
      LOG(INFO) << "createNeuron Compiltion Type COMPILATION_TYPE_EXECUTION_THROUGHPUT_MODE";
      compilationType = COMPILATION_TYPE_EXECUTION_THROUGHPUT_MODE;
    }
    err = NeuronCompilation_createV2(model, compilationType, nullptr, &compilation);
    RETURN_FALSE_ON_ERR(err, "NeuronCompilation_create failed");
    mCompilation = std::unique_ptr<NeuronCompilation, NeuronDeleter>(compilation);
    err = NeuronCompilation_setPreference(compilation, mPreference);
    RETURN_FALSE_ON_ERR(err, "NeuronCompilation_setPreference failed");
    err = NeuronCompilation_setPriority(compilation, NEURON_PRIORITY_HIGH);
    RETURN_FALSE_ON_ERR(err, "NeuronCompilation_setPriority failed");
    err = NeuronCompilation_finish(compilation);
    RETURN_FALSE_ON_ERR(err, "Failed to restore compiled network from extension");
    LOG(INFO) << "createNeuron Compiltion Finish finish Successfully";

    free(dlaBuffer);
    LOG(INFO) << "createNeuron Free DLA Buffer";
    // ---------------------------Execution------------------------------------
    // Run the compiled model against a set of inputs
    LOG(INFO) << "createNeuron Execution Create";
    err = NeuronExecution_create(compilation, &execution);
    RETURN_FALSE_ON_ERR(err, "Create Neuron Execution failed");
    mExecution = std::unique_ptr<NeuronExecution, NeuronDeleter>(execution);
    if (mUseThroughputMode) {
      LOG(INFO) << "createNeuron Execution Set Runner Pool Size: " << mShardsNum;
      err = NeuronExecution_setRunnerPoolSize(execution, mShardsNum);
      RETURN_FALSE_ON_ERR(err, "NeuronExecution_create Set Runner Pool Size FAILED");

      LOG(INFO) << "createNeuron Execution Set Job Pool Size: " << mThreadNum;
      err = NeuronExecution_setJobPoolSize(execution, mThreadNum);
      RETURN_FALSE_ON_ERR(err, "NeuronExecution_setJobPoolSize FAILED");
    }
    if (mNeuronExecutionFlushValue != 0) {
      err = NeuronExecution_setCacheFlushHint(execution,
                                               mNeuronExecutionFlushValue);
      RETURN_FALSE_ON_ERR(err, "NeuronExecution_setCacheFlushHint Failed");
    }
    LOG(INFO) << "createNeuron End Successfully";
    return true;
  }

  bool moveToBackend(neuron_backend_ptr_t backend_ptr) {
    AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;

    backend_data->inputSizes = std::move(mInputSizes);
    backend_data->inputMemoryWrappers = std::move(mInputMemoryWrappers);
    backend_data->outputSizes = std::move(mOutputSizes);
    backend_data->outputMemoryWrappers = std::move(mOutputMemoryWrappers);

    backend_data->model = mModel.release();
    backend_data->compilation = mCompilation.release();
    backend_data->execution = mExecution.release();
    return true;
  }

  bool createMemory(int type) {
    std::vector<NeuronOperandType> *sourceOperand = &mInputOperand;
    std::vector<AhwbNeuronMemoryWrapper> *memoryWrappers = &mInputMemoryWrappers;
    bool *isSuppress = &mInputSuppress;
    std::vector<uint32_t> *targetSizes = &mInputSizes;

    if (type == TYPE_OUTPUT) {
      sourceOperand = &mOutputOperand;
      memoryWrappers = &mOutputMemoryWrappers;
      isSuppress = &mOutputSuppress;
      targetSizes = &mOutputSizes;
    }

    for (int t = 0; t < mShardsNum; t++) {
      for (int i = 0; i < sourceOperand->size(); i++) {
        auto operand = sourceOperand->at(i);
        size_t size = 0;
        if (*isSuppress) {
          if (type == TYPE_INPUT) {
            if (NeuronCompilation_getInputPaddedSize(mCompilation.get(), i, &size) != NEURON_NO_ERROR) {
              LOG(ERROR) << "Neuron Input Operand: " << i << " size failed";
            } else {
              LOG(INFO) << "Neuron Input Operand: " << i << " PaddedSize: " << size;
            }
          } else {
            if (NeuronCompilation_getOutputPaddedSize(mCompilation.get(), i, &size) != NEURON_NO_ERROR) {
              LOG(ERROR) << "Neuron Output Operand: " << i << " size failed";
            } else {
              LOG(INFO) << "Neuron Output Operand: " << i << " PaddedSize: " << size;
            }
          }
        } else {
          size = GetNeuronOperandSize(operand);
          LOG(INFO) << "NeuronOperandSize: " << size;
        }

        targetSizes->push_back(size);  // B x Input[i] == B * W * H * C
        memoryWrappers->emplace_back(size, mAhwbType);
        if (memoryWrappers->back().InitNeuronMemory() != NEURON_NO_ERROR) {
          LOG(ERROR) << "Init Neuron Memory for " << t << "th execution operand: " << i << " failed";
          return false;
        }
      }
    }
    return true;
  }

  std::vector<NeuronOperandType> mInputOperand;

  std::vector<NeuronOperandType> mOutputOperand;

  std::string mModelPath;

  std::unique_ptr<NeuronModel, NeuronDeleter> mModel;

  std::unique_ptr<NeuronCompilation, NeuronDeleter> mCompilation;

  std::unique_ptr<NeuronExecution, NeuronDeleter> mExecution;

  std::vector<AhwbNeuronMemoryWrapper> mInputMemoryWrappers;

  std::vector<AhwbNeuronMemoryWrapper> mOutputMemoryWrappers;

  std::vector<uint32_t> mInputSizes;

  std::vector<uint32_t> mOutputSizes;

  bool mUseThroughputMode = false;

  bool mInputSuppress = false;

  bool mOutputSuppress = false;

  uint32_t mThreadNum = 1;

  uint32_t mShardsNum = 1;

  std::vector<NeuronExecution *> mExecs;

  NeuronAdapterPreferenceCode mPreference = NEURON_PREFER_TURBO_BOOST;

  uint8_t mNeuronExecutionFlushValue = 0;

  uint64_t mAhwbType = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN |
                       AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
};
