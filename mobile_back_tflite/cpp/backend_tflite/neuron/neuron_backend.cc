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

#if MTK_TFLITE_NEURON_BACKEND

#include "neuron_backend.h"

#include <stdlib.h>
#include <string.h>
#include <sys/system_properties.h>

#include <fstream>

#include "NeuronAdapter.h"
#include "NeuronAdapterShim.h"
#include "neuron_builder.h"
#include "tensorflow/core/platform/logging.h"

std::string GetPropertyValue(const std::string &property) {
  char value[PROP_VALUE_MAX];
  if (0 == __system_property_get(property.c_str(), value)) {
    return std::string();
  }
  return std::string(value);
}

std::string GetPlatformName() {
  static std::string kPlatform = GetPropertyValue("ro.hardware");
  return kPlatform;
}

template <typename T>
void Padding(AdapterBackendData *backend, int bytes, void *data) {
  int paddedLength = backend->inputSizes[0] / sizeof(T);
  int originalLength = bytes / sizeof(T);
  T *image_data = reinterpret_cast<T *>(data);

  int padded_num = paddedLength - originalLength;
  assert(padded_num >= 0);

  for (int num = 0; num < padded_num; num++) {
    image_data[paddedLength - 1 - num * 4] = 0;
    image_data[paddedLength - 2 - num * 4] =
        image_data[originalLength - 1 - num * 3];
    image_data[paddedLength - 3 - num * 4] =
        image_data[originalLength - 2 - num * 3];
    image_data[paddedLength - 4 - num * 4] =
        image_data[originalLength - 3 - num * 3];
  }
}

void create_mobilenet_edge_model(neuron_backend_ptr_t backend_ptr,
                                 const char *model) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;

  // Init information
  backend_data->input_nums = 1;
  backend_data->inputAhwbs.resize(1);
  backend_data->inputBuffers.resize(1);
  backend_data->inputTypes.resize(1);
  backend_data->iMemorys.resize(1);
  backend_data->inputSizes.resize(1);

  backend_data->output_nums = 1;
  backend_data->outputAhwbs.resize(1);
  backend_data->outputBuffers.resize(1);
  backend_data->outputTypes.resize(1);
  backend_data->oMemorys.resize(1);
  backend_data->outputSizes.resize(1);

  // Handle input & output data type
  {  // Input
    neuron_data_t type;
    type.type = neuron_data_t::Uint8;
    type.size = 150528;  // 1*224*224*3
    backend_data->inputTypes[0] = type;
  }

  {  // Output
    neuron_data_t type;
    type.type = neuron_data_t::Uint8;
    type.size = 1001;
    backend_data->outputTypes[0] = type;
  }

  int err = NEURON_NO_ERROR;

  // input op
  uint32_t dims_input[4] = {1, 224, 224, 3};
  NeuronOperandType tensor1x224x224x3Type{
      .type = NEURON_TENSOR_QUANT8_ASYMM,
      .scale = 0.007874015718698502f,
      .zeroPoint = 128,
      .dimensionCount = 4,
      .dimensions = dims_input,
  };

  // output op
  uint32_t dims_input2[1] = {1001};
  NeuronOperandType tensor1Type{
      .type = NEURON_TENSOR_QUANT8_ASYMM,
      .scale = 1.0f,
      .zeroPoint = 0,
      .dimensionCount = 1,
      .dimensions = dims_input2,
  };

  std::vector<NeuronOperandType> input_op = {tensor1x224x224x3Type};
  std::vector<NeuronOperandType> output_op = {tensor1Type};

  NeuronBuilder builder;

  if (builder.setInputOperand(input_op)
          .setOutputOperand(output_op)
          .setSuppressConversion(TYPE_INPUT, true)
          .setSuppressConversion(TYPE_OUTPUT, false)
          .setAhwbType(AHWB_OFTEN)
          .setExecutionFlushValue(0)
          .setModelPath(model)
          .create(backend_ptr) == false) {
    LOG(ERROR) << "create Neuron error";
    return;
  }

  // // Set input value
  err = NeuronExecution_setInputFromMemory(backend_data->execution, 0, nullptr,
                                           backend_data->iMemorys[0], 0,
                                           backend_data->inputSizes[0]);
  if (err != NEURON_NO_ERROR) {
    LOG(ERROR) << "NeuronExecution_setInputFromMemory error: " << err;
    return;
  }

  // // Set the output
  err = NeuronExecution_setOutputFromMemory(backend_data->execution, 0, nullptr,
                                            backend_data->oMemorys[0], 0,
                                            backend_data->outputSizes[0]);
  if (err != NEURON_NO_ERROR) {
    LOG(ERROR) << "NeuronExecution_setOutputFromMemory error: " << err;
    return;
  }

  backend_data->paddingFunc = &Padding<uint8_t>;
}

void create_mobilenet_v4_model(neuron_backend_ptr_t backend_ptr,
                               const char *model) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;

  // Init information
  backend_data->input_nums = 1;
  backend_data->inputAhwbs.resize(1);
  backend_data->inputBuffers.resize(1);
  backend_data->inputTypes.resize(1);
  backend_data->iMemorys.resize(1);
  backend_data->inputSizes.resize(1);

  backend_data->output_nums = 1;
  backend_data->outputAhwbs.resize(1);
  backend_data->outputBuffers.resize(1);
  backend_data->outputTypes.resize(1);
  backend_data->oMemorys.resize(1);
  backend_data->outputSizes.resize(1);

  // Handle input & output data type
  {  // Input
    neuron_data_t type;
    type.type = neuron_data_t::Uint8;
    type.size = 442368;  // 1*384*384*3
    backend_data->inputTypes[0] = type;
  }

  {  // Output
    neuron_data_t type;
    type.type = neuron_data_t::Uint8;
    type.size = 1000;
    backend_data->outputTypes[0] = type;
  }

  int err = NEURON_NO_ERROR;

  // input op
  uint32_t dims_input[4] = {1, 384, 384, 3};
  NeuronOperandType tensor1x384x384x3Type{
      .type = NEURON_TENSOR_QUANT8_ASYMM,
      .scale = 0.00787125,
      .zeroPoint = 128,
      .dimensionCount = 4,
      .dimensions = dims_input,
  };

  // output op
  uint32_t dims_input2[1] = {1000};
  NeuronOperandType tensor1Type{
      .type = NEURON_TENSOR_QUANT8_ASYMM,
      .scale = 1.0f,
      .zeroPoint = 0,
      .dimensionCount = 1,
      .dimensions = dims_input2,
  };

  std::vector<NeuronOperandType> input_op = {tensor1x384x384x3Type};
  std::vector<NeuronOperandType> output_op = {tensor1Type};

  NeuronBuilder builder;

  if (builder.setInputOperand(input_op)
          .setOutputOperand(output_op)
          .setSuppressConversion(TYPE_INPUT, true)
          .setSuppressConversion(TYPE_OUTPUT, false)
          .setAhwbType(AHWB_OFTEN)
          .setExecutionFlushValue(0)
          .setModelPath(model)
          .create(backend_ptr) == false) {
    LOG(ERROR) << "create Neuron error";
    return;
  }

  // // Set input value
  err = NeuronExecution_setInputFromMemory(backend_data->execution, 0, nullptr,
                                           backend_data->iMemorys[0], 0,
                                           backend_data->inputSizes[0]);
  if (err != NEURON_NO_ERROR) {
    LOG(ERROR) << "NeuronExecution_setInputFromMemory error: " << err;
    return;
  }

  // // Set the output
  err = NeuronExecution_setOutputFromMemory(backend_data->execution, 0, nullptr,
                                            backend_data->oMemorys[0], 0,
                                            backend_data->outputSizes[0]);
  if (err != NEURON_NO_ERROR) {
    LOG(ERROR) << "NeuronExecution_setOutputFromMemory error: " << err;
    return;
  }

  backend_data->paddingFunc = &Padding<uint8_t>;
}

void create_mobiledet_qat_model(neuron_backend_ptr_t backend_ptr,
                                const char *model) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;

  // Init information
  backend_data->input_nums = 1;
  backend_data->inputAhwbs.resize(1);
  backend_data->inputBuffers.resize(1);
  backend_data->inputTypes.resize(1);
  backend_data->iMemorys.resize(1);
  backend_data->inputSizes.resize(1);

  backend_data->output_nums = 4;
  backend_data->outputAhwbs.resize(4);
  backend_data->outputBuffers.resize(4);
  backend_data->outputTypes.resize(4);
  backend_data->oMemorys.resize(4);
  backend_data->outputSizes.resize(4);

  // Handle input & output data type
  {  // Input
    neuron_data_t type;
    type.type = neuron_data_t::Uint8;
    type.size = 307200;  // 1 x 320 x 320 x 3
    backend_data->inputTypes[0] = type;
  }

  {  // Output
    int sizes[4] = {40, 10, 10, 1};
    for (int i = 0; i < backend_data->output_nums; i++) {
      neuron_data_t type;
      type.type = neuron_data_t::Float32;
      type.size = sizes[i];
      backend_data->outputTypes[i] = type;
    }
  }

  int err = NEURON_NO_ERROR;
  // input 0
  uint32_t dims_input[4] = {1, 320, 320, 3};
  NeuronOperandType tensor1x320x320x3Type{
      .type = NEURON_TENSOR_QUANT8_ASYMM,
      .scale = 0.0078125f,
      .zeroPoint = 128,
      .dimensionCount = 4,
      .dimensions = dims_input,
  };

  // output 0
  uint32_t dims_input2[4] = {1, 1, 10, 4};
  NeuronOperandType tensor1x1x10x4Type{.type = NEURON_TENSOR_FLOAT32,
                                       .scale = 0,
                                       .zeroPoint = 0,
                                       .dimensionCount = 4,
                                       .dimensions = dims_input2};

  // output 1, 2
  uint32_t dims_input3[4] = {1, 1, 1, 10};
  NeuronOperandType tensor1x1x1x10Type{.type = NEURON_TENSOR_FLOAT32,
                                       .scale = 0,
                                       .zeroPoint = 0,
                                       .dimensionCount = 4,
                                       .dimensions = dims_input3};

  // output 3
  uint32_t dims_input4[4] = {1, 1, 1, 1};
  NeuronOperandType tensor1x1x1x1Type{
      .type = NEURON_TENSOR_FLOAT32,
      .scale = 0,
      .zeroPoint = 0,
      .dimensionCount = 4,
      .dimensions = dims_input4,
  };

  std::vector<NeuronOperandType> input_op = {tensor1x320x320x3Type};
  std::vector<NeuronOperandType> output_op = {
      tensor1x1x10x4Type, tensor1x1x1x10Type, tensor1x1x1x10Type,
      tensor1x1x1x1Type};

  NeuronBuilder builder;
  if (builder.setInputOperand(input_op)
          .setOutputOperand(output_op)
          .setSuppressConversion(TYPE_INPUT, true)
          .setSuppressConversion(TYPE_OUTPUT, false)
          .setAhwbType(AHWB_RARELY)
          .setExecutionFlushValue(3)
          .setModelPath(model)
          .create(backend_ptr) == false) {
    LOG(ERROR) << "create Neuron error";
    return;
  }

  // Set input value
  err = NeuronExecution_setInputFromMemory(backend_data->execution, 0, nullptr,
                                           backend_data->iMemorys[0], 0,
                                           backend_data->inputSizes[0]);
  if (err != NEURON_NO_ERROR) {
    LOG(ERROR) << "NeuronExecution_setInputFromMemory error: " << err;
    return;
  }

  // Set output value
  int sizes[4] = {160, 40, 40, 4};
  for (int i = 0; i < 4; i++) {
    err = NeuronExecution_setOutputFromMemory(
        backend_data->execution, i, nullptr, backend_data->oMemorys[i], 0,
        backend_data->outputSizes[i]);
    if (err != NEURON_NO_ERROR) {
      LOG(ERROR) << "NeuronExecution_setOutputFromMemory[" << i
                 << "] error: " << err;
      return;
    }
  }

  backend_data->paddingFunc = &Padding<uint8_t>;
}

void create_mobilebert_nnapi_model(neuron_backend_ptr_t backend_ptr,
                                   const char *model) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;

  // Init information
  backend_data->input_nums = 3;
  backend_data->inputAhwbs.resize(3);
  backend_data->inputBuffers.resize(3);
  backend_data->inputTypes.resize(3);
  backend_data->iMemorys.resize(3);
  backend_data->inputSizes.resize(3);

  backend_data->output_nums = 2;
  backend_data->outputAhwbs.resize(2);
  backend_data->outputBuffers.resize(2);
  backend_data->outputTypes.resize(2);
  backend_data->oMemorys.resize(2);
  backend_data->outputSizes.resize(2);

  // Handle input & output data type
  // Input
  for (int i = 0; i < backend_data->input_nums; i++) {
    neuron_data_t type;
    type.type = neuron_data_t::Int32;
    type.size = 384;  // 1x 384
    backend_data->inputTypes[i] = type;
  }

  // Output
  for (int i = 0; i < backend_data->output_nums; i++) {
    neuron_data_t type;
    type.type = neuron_data_t::Float32;
    type.size = 384;  // 1 x 1 x 1x 384
    backend_data->outputTypes[i] = type;
  }

  int err = NEURON_NO_ERROR;
  // input 0, 1, 2
  uint32_t dims_input[2] = {1, 384};
  NeuronOperandType tensor1x384Type{
      .type = NEURON_TENSOR_INT32,
      .scale = 0.0f,
      .zeroPoint = 0,
      .dimensionCount = 2,
      .dimensions = dims_input,
  };

  // output 0, 1
  uint32_t dims_input2[4] = {1, 1, 1, 384};
  NeuronOperandType tensor1x1x1x384Type{
      .type = NEURON_TENSOR_FLOAT32,
      .scale = 0,
      .zeroPoint = 0,
      .dimensionCount = 4,
      .dimensions = dims_input2,
  };

  std::vector<NeuronOperandType> input_op = {tensor1x384Type, tensor1x384Type,
                                             tensor1x384Type};
  std::vector<NeuronOperandType> output_op = {tensor1x1x1x384Type,
                                              tensor1x1x1x384Type};

  NeuronBuilder builder;
  if (builder.setInputOperand(input_op)
          .setOutputOperand(output_op)
          .setSuppressConversion(TYPE_INPUT, false)
          .setSuppressConversion(TYPE_OUTPUT, false)
          .setAhwbType(AHWB_OFTEN)
          .setExecutionFlushValue(0)
          .setModelPath(model)
          .create(backend_ptr) == false) {
    LOG(ERROR) << "create Neuron error";
    return;
  }

  for (int i = 0; i < backend_data->input_nums; i++) {
    // Set input value
    err = NeuronExecution_setInputFromMemory(backend_data->execution, i,
                                             nullptr, backend_data->iMemorys[i],
                                             0, backend_data->inputSizes[i]);
    if (err != NEURON_NO_ERROR) {
      LOG(ERROR) << "NeuronExecution_setInputFromMemory[" << i
                 << "] error: " << err;
      return;
    }
  }

  for (int i = 0; i < backend_data->output_nums; i++) {
    err = NeuronExecution_setOutputFromMemory(
        backend_data->execution, i, nullptr, backend_data->oMemorys[i], 0,
        backend_data->outputSizes[i]);
    if (err != NEURON_NO_ERROR) {
      LOG(ERROR) << "NeuronExecution_setOutputFromMemory[" << i
                 << "] error: " << err;
      return;
    }
  }
}

void create_mobile_segmenter_model(neuron_backend_ptr_t backend_ptr,
                                   const char *model) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;

  // Init information
  backend_data->input_nums = 1;
  backend_data->inputAhwbs.resize(1);
  backend_data->inputBuffers.resize(1);
  backend_data->inputTypes.resize(1);
  backend_data->iMemorys.resize(1);
  backend_data->inputSizes.resize(1);

  backend_data->output_nums = 1;
  backend_data->outputAhwbs.resize(1);
  backend_data->outputBuffers.resize(1);
  backend_data->outputTypes.resize(1);
  backend_data->oMemorys.resize(1);
  backend_data->outputSizes.resize(1);

  // Handle input & output data type
  {  // Input
    neuron_data_t type;
    type.type = neuron_data_t::Uint8;
    type.size = 786432;  // 1*512*512*3
    backend_data->inputTypes[0] = type;
  }

  {  // Output
    neuron_data_t type;
    type.type = neuron_data_t::Int32;
    type.size = 512 * 512;  // 1 * 1 * 512 * 512
    backend_data->outputTypes[0] = type;
  }

  int err = NEURON_NO_ERROR;

  // input 0
  uint32_t dims_input[4] = {1, 512, 512, 3};
  NeuronOperandType tensor1x512x512x3Type{.type = NEURON_TENSOR_QUANT8_ASYMM,
                                          .scale = 0.007843137718737125f,
                                          .zeroPoint = 128,
                                          .dimensionCount = 4,
                                          .dimensions = dims_input};

  // output 0
  uint32_t dims_input2[4] = {1, 1, 512, 512};
  NeuronOperandType tensor1x1x512x512Type{.type = NEURON_TENSOR_INT32,
                                          .scale = 0,
                                          .zeroPoint = 0,
                                          .dimensionCount = 4,
                                          .dimensions = dims_input2};

  std::vector<NeuronOperandType> input_op = {tensor1x512x512x3Type};
  std::vector<NeuronOperandType> output_op = {tensor1x1x512x512Type};

  NeuronBuilder builder;
  if (builder.setInputOperand(input_op)
          .setOutputOperand(output_op)
          .setSuppressConversion(TYPE_INPUT, true)
          .setSuppressConversion(TYPE_OUTPUT, false)
          .setAhwbType(AHWB_RARELY)
          .setExecutionFlushValue(3)
          .setModelPath(model)
          .create(backend_ptr) == false) {
    LOG(ERROR) << "create Neuron error";
    return;
  }

  // Set the input
  err = NeuronExecution_setInputFromMemory(backend_data->execution, 0, nullptr,
                                           backend_data->iMemorys[0], 0,
                                           backend_data->inputSizes[0]);
  if (err != NEURON_NO_ERROR) {
    LOG(ERROR) << "NeuronExecution_setInputFromMemory error: " << err;
    return;
  }

  // Set the output
  err = NeuronExecution_setOutputFromMemory(backend_data->execution, 0, nullptr,
                                            backend_data->oMemorys[0], 0,
                                            backend_data->outputSizes[0]);
  if (err != NEURON_NO_ERROR) {
    LOG(ERROR) << "NeuronExecution_setOutputFromMemory error: " << err;
    return;
  }

  backend_data->paddingFunc = &Padding<uint8_t>;
}

void create_edsr_model(neuron_backend_ptr_t backend_ptr, const char *model) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;

  // Init information
  backend_data->input_nums = 1;
  backend_data->inputAhwbs.resize(1);
  backend_data->inputBuffers.resize(1);
  backend_data->inputTypes.resize(1);
  backend_data->iMemorys.resize(1);
  backend_data->inputSizes.resize(1);

  backend_data->output_nums = 1;
  backend_data->outputAhwbs.resize(1);
  backend_data->outputBuffers.resize(1);
  backend_data->outputTypes.resize(1);
  backend_data->oMemorys.resize(1);
  backend_data->outputSizes.resize(1);

  // Handle input & output data type
  {  // Input
    neuron_data_t type;
    type.type = neuron_data_t::Int8;
    type.size = 1555200;  // 1*540*960*3
    backend_data->inputTypes[0] = type;
  }

  {  // Output
    neuron_data_t type;
    type.type = neuron_data_t::Int8;
    type.size = 6220800;  // 1*1080*1920*3
    backend_data->outputTypes[0] = type;
  }

  int err = NEURON_NO_ERROR;

  // input op
  uint32_t dims_input[4] = {1, 540, 960, 3};
  NeuronOperandType tensor1x540x960x3Type{
      .type = NEURON_TENSOR_QUANT8_ASYMM,
      .scale = 1.0f,
      .zeroPoint = -128,
      .dimensionCount = 4,
      .dimensions = dims_input,
  };

  // output op
  uint32_t dims_input2[4] = {1, 1080, 1920, 3};
  NeuronOperandType tensor1Type{
      .type = NEURON_TENSOR_QUANT8_ASYMM,
      .scale = 1.0f,
      .zeroPoint = -128,
      .dimensionCount = 4,
      .dimensions = dims_input2,
  };

  std::vector<NeuronOperandType> input_op = {tensor1x540x960x3Type};
  std::vector<NeuronOperandType> output_op = {tensor1Type};

  NeuronBuilder builder;

  if (builder.setInputOperand(input_op)
          .setOutputOperand(output_op)
          .setSuppressConversion(TYPE_INPUT, true)
          .setSuppressConversion(TYPE_OUTPUT, false)
          .setAhwbType(AHWB_OFTEN)
          .setExecutionFlushValue(0)
          .setModelPath(model)
          .create(backend_ptr) == false) {
    LOG(ERROR) << "create Neuron error";
    return;
  }

  // // Set input value
  err = NeuronExecution_setInputFromMemory(backend_data->execution, 0, nullptr,
                                           backend_data->iMemorys[0], 0,
                                           backend_data->inputSizes[0]);
  if (err != NEURON_NO_ERROR) {
    LOG(ERROR) << "NeuronExecution_setInputFromMemory error: " << err;
    return;
  }

  // // Set the output
  err = NeuronExecution_setOutputFromMemory(backend_data->execution, 0, nullptr,
                                            backend_data->oMemorys[0], 0,
                                            backend_data->outputSizes[0]);
  if (err != NEURON_NO_ERROR) {
    LOG(ERROR) << "NeuronExecution_setOutputFromMemory error: " << err;
    return;
  }

  backend_data->paddingFunc = &Padding<int8_t>;
}

bool need_neuron_backend(const char *model_path) {
  auto model = std::string(model_path);
  std::string fileExtension = model.substr(model.find_last_of(".") + 1);
  if (fileExtension.compare("dla") == 0) {
    return true;
  }
  LOG(INFO) << "Not supported model: " << std::string(model_path);
  return false;
}

bool create_neuron_backend(neuron_backend_ptr_t backend_ptr,
                           const char *model_path) {
  auto insertTestAfterDirectory = [](std::string input,
                                     std::string directory) -> std::string {
    size_t pos = input.find(directory);
    if (pos == std::string::npos) {
      // Directory not found, return the original input
      return input;
    }
    // Add the length of the directory to the position
    pos += directory.length();
    // Insert '/test' after the directory
    std::string output = input.substr(0, pos) + "/test" + input.substr(pos);
    return output;
  };

  LOG(INFO) << "create_neuron_backend Start.";
  std::string model = std::string(model_path);
  model = insertTestAfterDirectory(model, GetPlatformName());
  LOG(INFO) << "model: " << model;
  if (model.find("mobilenet_edgetpu_224_1.0_uint8.dla") != std::string::npos) {
    create_mobilenet_edge_model(backend_ptr, model_path);
  } else if (model.find("MobileNetV4-Conv-Large-int8-ptq.dla") !=
             std::string::npos) {
    create_mobilenet_v4_model(backend_ptr, model_path);
  } else if (model.find("mobiledet_qat.dla") != std::string::npos) {
    create_mobiledet_qat_model(backend_ptr, model_path);
  } else if (model.find("mobile_segmenter_r4_quant_argmax_uint8.dla") !=
             std::string::npos) {
    create_mobile_segmenter_model(backend_ptr, model_path);
  } else if (model.find("mobilebert_int8_384_nnapi.dla") != std::string::npos) {
    create_mobilebert_nnapi_model(backend_ptr, model_path);
  } else if (model.find("edsr_f32b5_full_qint8.dla") != std::string::npos) {
    create_edsr_model(backend_ptr, model_path);
  } else {
    LOG(ERROR) << "Not supported dla";
    return false;
  }
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  backend_data->useNeuronBackend = true;

  // PERFORMANCE_ON;
  backend_data->locker.Start(0);  // Powerhal infinite times

  LOG(INFO) << "create_neuron_backend success";
  return true;
}

bool delete_neuron_backend(neuron_backend_ptr_t backend_ptr) {
  LOG(INFO) << "delete_neuron_backend Start";

  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;

  // PERFORMANCE_OFF;

  if (backend_data->useNeuronBackend) {
    for (int i = 0; i < backend_data->input_nums; i++) {
      NeuronMemory_free(backend_data->iMemorys[i]);
      AHardwareBuffer_unlock(backend_data->inputAhwbs[i], nullptr);
      AHardwareBuffer_release(backend_data->inputAhwbs[i]);
    }

    for (int i = 0; i < backend_data->output_nums; i++) {
      NeuronMemory_free(backend_data->oMemorys[i]);
      AHardwareBuffer_unlock(backend_data->outputAhwbs[i], nullptr);
      AHardwareBuffer_release(backend_data->outputAhwbs[i]);
    }
    NeuronExecution_free(backend_data->execution);
    NeuronCompilation_free(backend_data->compilation);
    NeuronModel_free(backend_data->model);

    backend_data->inputAhwbs.clear();
    backend_data->inputBuffers.clear();
    backend_data->inputTypes.clear();
    backend_data->iMemorys.clear();

    backend_data->outputAhwbs.clear();
    backend_data->outputBuffers.clear();
    backend_data->outputTypes.clear();
    backend_data->oMemorys.clear();

    LOG(INFO) << "delete_neuron_backend End";
    return true;
  }

  return false;
}

bool neuron_get_in_out_count(neuron_backend_ptr_t backend_ptr, bool isIn,
                             int32_t *count) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  if (backend_data->useNeuronBackend) {
    *count = isIn ? backend_data->input_nums : backend_data->output_nums;
    return true;
  }
  return false;
}

bool neuron_get_in_out_datatype(neuron_backend_ptr_t backend_ptr, int32_t i,
                                bool isIn, neuron_data_t *type) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  if (backend_data->useNeuronBackend) {
    *type = isIn ? backend_data->inputTypes[i] : backend_data->outputTypes[i];
    return true;
  }
  return false;
}

bool neuron_set_input(neuron_backend_ptr_t backend_ptr, int32_t i, void *data) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  if (!backend_data->useNeuronBackend) {
    return false;
  }
#ifdef SET_INPUT_FROM_MEMORY
  // If the input data is cached, use the cached NeuronMemory first.
  if (backend_data->neuronAllocator.IsEnable()) {
    auto [base, offset, memory] =
        backend_data->neuronAllocator.GetCachedMemory(data);
    if (memory != nullptr) {
      LOG(INFO) << "cached hit";
      int res = 0;
      {
        TRACER("NeuronExecution_setInputFromMemory");
        res = NeuronExecution_setInputFromMemory(backend_data->execution, i,
                                                 nullptr, memory, offset,
                                                 backend_data->inputSizes[i]);
      }
      if (res == NEURON_NO_ERROR) {
        return true;
      } else {
        LOG(ERROR) << "Set cached Neuron Memory Failed. fallback";
        abort();
      }
    }
  }
#endif

  // TRACER("memcpy");
  memcpy(backend_data->inputBuffers[i], data, backend_data->inputSizes[i]);
  return true;
}

bool neuron_get_output(neuron_backend_ptr_t backend_ptr, int32_t i,
                       void **data) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  if (backend_data->useNeuronBackend) {
    *data = backend_data->outputBuffers[i];
    return true;
  }
  return false;
}

bool neuron_issue_query(neuron_backend_ptr_t backend_ptr) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  if (backend_data->useNeuronBackend) {
    NeuronExecution_compute(backend_data->execution);
    return true;
  }
  return false;
}

bool neuron_convert_input(neuron_backend_ptr_t backend_ptr, int bytes,
                          void *data) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  if (!backend_data->useNeuronBackend) {
    return false;
  }

  if (backend_data->paddingFunc) {
    backend_data->paddingFunc(backend_data, bytes, data);
  }

#ifdef SET_INPUT_FROM_MEMORY
  static int cnt = 0;
  cnt++;
  LOG(INFO) << "neuron_convert_input :" << cnt;
  if (cnt % 300 != 0) {
    return true;
  }
  LOG(INFO) << "neuron_convert_input should set";
  // We hint the "importForvered" APUSysEngine by preset the NeuronMemory.
  // Due to the first-time setInputFromMemory would take longer time,
  // we preset the input here.
  if (!backend_data->hasPreset) {
    TRACER("Preset_NeuronExecution");
    backend_data->hasPreset = true;
    auto neuronMemorys =
        backend_data->neuronAllocator.GetAllocatedNeuronMemory();
    for (auto [size, memorys] : neuronMemorys) {
      for (auto memory : memorys) {
        int res = NeuronExecution_setInputFromMemory(backend_data->execution, 0,
                                                     nullptr, memory, 0, size);
        if (res != NEURON_NO_ERROR) {
          LOG(ERROR) << "Preset NeuronExecution fail";
        }
      }
    }
  }
#endif  // PRE_HINT_ALL_NEURONMEMORY
  return true;
}
#endif
