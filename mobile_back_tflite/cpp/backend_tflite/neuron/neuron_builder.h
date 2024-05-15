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

  LOG(INFO) << "Reading " << length << " characters... ";
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
  ~NeuronBuilder() {
    mInputAhwb.clear();
    mOutputAhwb.clear();

    mInputNeuronMemory.clear();
    mOutputNeuronMemory.clear();

    mExecution.reset();
    mCompilation.reset();
    mModel.reset();
  }

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

  NeuronBuilder &setModelPath(const char *dla_path) {
    mModelPath = dla_path;
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

    moveToBackend(backend_ptr);
    return true;
  }

 private:
  bool createNeuron() {
    NeuronModel *model = nullptr;
    NeuronCompilation *compilation = nullptr;
    NeuronExecution *execution = nullptr;

    uint8_t *dlaBuffer = nullptr;
    int length = load_file(mModelPath.c_str(), &dlaBuffer);
    if (length == 0) {
      return false;
    }
    // ---------------------------Model------------------------------------
    int err = NEURON_NO_ERROR;
    err |= NeuronModel_create(&model);
    if (err != NEURON_NO_ERROR) {
      LOG(ERROR) << "NeuronModel_create error";
      return false;
    }
    mModel = std::unique_ptr<NeuronModel, NeuronDeleter>(model);

    std::vector<uint32_t> input_op_number;
    for (int i = 0; i < mInputOperand.size(); i++) {
      err |= NeuronModel_addOperand(model, &mInputOperand[i]);
      input_op_number.emplace_back(i);
    }

    int32_t operandType = 0;
    const uint16_t network_operand_restore_data =
        RESTORE_DLA_EXTENSION_OPERAND_TYPE;
    const char *extensionRestroeCompiledNetwork = RESTORE_DLA_EXTENSION_NAME;
    err |= NeuronModel_getExtensionOperandType(
        model, extensionRestroeCompiledNetwork, network_operand_restore_data,
        &operandType);

    NeuronOperandType extenOperandType{
        .type = operandType,
        .scale = 0.0f,
        .zeroPoint = 0,
        .dimensionCount = 0,
    };

    err |= NeuronModel_addOperand(model, &extenOperandType);
    input_op_number.emplace_back(input_op_number.size());

    std::vector<uint32_t> output_op_number;
    for (int i = 0; i < mOutputOperand.size(); i++) {
      err |= NeuronModel_addOperand(model, &mOutputOperand[i]);
      output_op_number.emplace_back(i + input_op_number.size());
    }

    if (err != NEURON_NO_ERROR) {
      LOG(ERROR) << "addOperand fail";
      return false;
    }
    err |= NeuronModel_setOperandValue(model, input_op_number.back(), dlaBuffer,
                                       length);

    int32_t operationType = 0;
    const uint16_t network_operation_type_restore =
        RESTORE_DLA_EXTENSION_OPERATION_TYPE;
    err |= NeuronModel_getExtensionOperationType(
        model, extensionRestroeCompiledNetwork, network_operation_type_restore,
        &operationType);

    if (err != NEURON_NO_ERROR) {
      LOG(ERROR) << "get ExtensionOperationType fail";
      return false;
    }

    // Add extension operation
    err |= NeuronModel_addOperation(
        model, (NeuronOperationType)operationType, input_op_number.size(),
        input_op_number.data(), output_op_number.size(),
        output_op_number.data());

    if (err != NEURON_NO_ERROR) {
      LOG(ERROR) << "get addOperation fail";
      return false;
    }

    // Identify input and output
    err |= NeuronModel_identifyInputsAndOutputs(
        model, input_op_number.size() - 1, input_op_number.data(),
        output_op_number.size(), output_op_number.data());
    if (err != NEURON_NO_ERROR) {
      LOG(ERROR) << "get identify input and output fail";
      return false;
    }

    if (mInputSuppress) {
      err |= NeuronModel_suppressInputConversion(model, true);
    }

    if (mOutputSuppress) {
      err |= NeuronModel_suppressOutputConversion(model, true);
    }
    if (err != NEURON_NO_ERROR) {
      LOG(ERROR) << "NeuronModel suppress fail";
      return false;
    }

    err |= NeuronModel_finish(model);
    if (err != NEURON_NO_ERROR) {
      LOG(ERROR) << "get model_finish fail";
      return false;
    }
    // ---------------------------Compilation------------------------------------
    if (NeuronCompilation_create(model, &compilation) != NEURON_NO_ERROR) {
      LOG(ERROR) << "NeuronCompilation_create fail";
      return false;
    };
    mCompilation =
        std::unique_ptr<NeuronCompilation, NeuronDeleter>(compilation);

    err |=
        NeuronCompilation_setPreference(compilation, NEURON_PREFER_TURBO_BOOST);
    err |= NeuronCompilation_setPriority(compilation, NEURON_PRIORITY_HIGH);
    if (err != NEURON_NO_ERROR) {
      LOG(ERROR) << "NeuronCompilation_setPreference or setPriority error";
      return false;
    }
    err = NeuronCompilation_finish(compilation);
    if (err != NEURON_NO_ERROR) {
      LOG(ERROR) << "Failed to restore compiled network from extension: "
                 << err;
      return false;
    }

    free(dlaBuffer);
    // ---------------------------Execution------------------------------------
    // Run the compiled model against a set of inputs
    err = NeuronExecution_create(compilation, &execution);
    if (err != NEURON_NO_ERROR) {
      LOG(ERROR) << "NeuronExecution_create error: " << err;
      return false;
    }
    mExecution = std::unique_ptr<NeuronExecution, NeuronDeleter>(execution);

    if (mNeuronExecutionFlushValue != 0) {
      err = NeuronExecution_setCacheFlushHint(execution,
                                              mNeuronExecutionFlushValue);
      if (err != NEURON_NO_ERROR) {
        LOG(ERROR) << "NeuronExecution_setCacheFlushHint error";
      }
    }

    return true;
  }

  bool moveToBackend(neuron_backend_ptr_t backend_ptr) {
    AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;

    for (int i = 0; i < mInputOperand.size(); i++) {
      backend_data->iMemorys[i] = mInputNeuronMemory[i].release();
      backend_data->inputAhwbs[i] = mInputAhwb[i].release();
      backend_data->inputBuffers[i] = mInputBuffer[i];
      backend_data->inputSizes[i] = mInputSizes[i];
    }
    for (int i = 0; i < mOutputOperand.size(); i++) {
      backend_data->oMemorys[i] = mOutputNeuronMemory[i].release();
      backend_data->outputAhwbs[i] = mOutputAhwb[i].release();
      backend_data->outputBuffers[i] = mOutputBuffer[i];
      backend_data->outputSizes[i] = mOutputSizes[i];
    }
    backend_data->model = mModel.release();
    backend_data->compilation = mCompilation.release();
    backend_data->execution = mExecution.release();
    return true;
  }

  bool createMemory(int type) {
    std::vector<NeuronOperandType> *sourceOperand = &mInputOperand;
    std::vector<std::unique_ptr<NeuronMemory, NeuronDeleter>> *targetMemory =
        &mInputNeuronMemory;
    std::vector<std::unique_ptr<AHardwareBuffer, BufferDeleter>>
        *targetABuffer = &mInputAhwb;
    std::vector<void *> *targetBuffer = &mInputBuffer;
    bool *isSuppress = &mInputSuppress;
    std::vector<uint32_t> *targetSizes = &mInputSizes;

    if (type == TYPE_OUTPUT) {
      sourceOperand = &mOutputOperand;
      targetMemory = &mOutputNeuronMemory;
      targetABuffer = &mOutputAhwb;
      targetBuffer = &mOutputBuffer;
      isSuppress = &mOutputSuppress;
      targetSizes = &mOutputSizes;
    }

    for (int i = 0; i < sourceOperand->size(); i++) {
      auto operand = sourceOperand->at(i);
      size_t size = 0;
      if (*isSuppress) {
        if (type == TYPE_INPUT) {
          NeuronCompilation_getInputPaddedSize(mCompilation.get(), i, &size);
        } else {
          NeuronCompilation_getOutputPaddedSize(mCompilation.get(), i, &size);
        }
        LOG(INFO) << "NeuronOperandPaddedSize: " << size;
      } else {
        size = GetNeuronOperandSize(operand);
        LOG(INFO) << "NeuronOperandSize: " << size;
      }

      targetSizes->push_back(size);
      AHardwareBuffer_Desc iDesc{
          .width = static_cast<uint32_t>(size),
          .height = 1,
          .layers = 1,
          .format = AHARDWAREBUFFER_FORMAT_BLOB,
          .usage = mAhwbType,
          .stride = static_cast<uint32_t>(size),
      };
      AHardwareBuffer *Abuffer = nullptr;
      AHardwareBuffer_allocate(&iDesc, &Abuffer);
      if (Abuffer == nullptr) {
        LOG(ERROR) << "AHWB allocate fail";
        return false;
      }
      targetABuffer->push_back(
          std::unique_ptr<AHardwareBuffer, BufferDeleter>(Abuffer));

      NeuronMemory *memory = nullptr;
      NeuronMemory_createFromAHardwareBuffer(Abuffer, &memory);
      if (memory == nullptr) {
        LOG(ERROR) << "NeuronMemory create fail";
        return false;
      }
      targetMemory->push_back(
          std::unique_ptr<NeuronMemory, NeuronDeleter>(memory));

      void *buffer = nullptr;
      AHardwareBuffer_lock(Abuffer, mAhwbType, -1, nullptr, &buffer);
      if (buffer == nullptr) {
        LOG(ERROR) << "AHWB lock fail";
        return false;
      }
      targetBuffer->push_back(buffer);
    }
    return true;
  }

  std::vector<NeuronOperandType> mInputOperand;

  std::vector<NeuronOperandType> mOutputOperand;

  std::string mModelPath;

  std::unique_ptr<NeuronModel, NeuronDeleter> mModel;

  std::unique_ptr<NeuronCompilation, NeuronDeleter> mCompilation;

  std::unique_ptr<NeuronExecution, NeuronDeleter> mExecution;

  std::vector<std::unique_ptr<NeuronMemory, NeuronDeleter>> mInputNeuronMemory;

  std::vector<std::unique_ptr<NeuronMemory, NeuronDeleter>> mOutputNeuronMemory;

  std::vector<std::unique_ptr<AHardwareBuffer, BufferDeleter>> mInputAhwb;

  std::vector<std::unique_ptr<AHardwareBuffer, BufferDeleter>> mOutputAhwb;

  std::vector<void *> mInputBuffer;

  std::vector<void *> mOutputBuffer;

  std::vector<uint32_t> mInputSizes;

  std::vector<uint32_t> mOutputSizes;

  bool mInputSuppress = false;

  bool mOutputSuppress = false;

  uint8_t mNeuronExecutionFlushValue = 0;

  uint64_t mAhwbType = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN |
                       AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
};
