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
#include <sys/mman.h>
#include <sys/system_properties.h>

#include <fstream>
#include <future>

#include "NeuronAdapter.h"
#include "NeuronAdapterShim.h"
#include "neuron_builder.h"
#include "tensorflow/core/platform/logging.h"

#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

namespace {
int32_t CreateNeuronMemoryWithAHWB(uint32_t byte_size,
                                   uint64_t ahwb_type,
                                   NeuronMemory **memory,
                                   AHardwareBuffer **abuffer) {
  if (byte_size == 0) {
    LOG(ERROR) << "Allocate AHWB with size 0 is not allowed.";
    return NEURON_BAD_DATA;
  }
  if (*memory != nullptr || *abuffer != nullptr) return NEURON_BAD_DATA;
  AHardwareBuffer_Desc iDesc{
      .width = byte_size,
      .height = 1,
      .layers = 1,
      .format = AHARDWAREBUFFER_FORMAT_BLOB,
      .usage = ahwb_type,
      .stride = byte_size,
  };
  LOG(INFO) << "CREATE AHWB: " << byte_size;

  AHardwareBuffer_allocate(&iDesc, abuffer);
  if (*abuffer == nullptr) {
    LOG(ERROR) << "Allocate AHWB failed";
    return NEURON_UNEXPECTED_NULL;
  }

  if (int ret = NeuronMemory_createFromAHardwareBuffer(*abuffer, memory);
      ret != NEURON_NO_ERROR) {
    LOG(ERROR) << "Create Neuron Memory Failed";
    return ret;
  }
  return NEURON_NO_ERROR;
}
}  // namespace

int32_t AhwbNeuronMemoryWrapper::InitNeuronMemory() {
  if (CreateNeuronMemoryWithAHWB(size_, ahwb_type_, &memory_, &abuffer_) !=
      NEURON_NO_ERROR) {
    LOG(ERROR) << "Construct NeuronMemoryWrapper for AHWB Failed";
    return NEURON_BAD_STATE;
  }
  return NEURON_NO_ERROR;
}

int32_t AhwbNeuronMemoryWrapper::unlockAhwbData() {
  if (!data_) return NEURON_NO_ERROR;
  if (AHardwareBuffer_unlock(abuffer_, nullptr) != 0) {
    LOG(ERROR) << "AHWB unlock fail";
    return NEURON_BAD_STATE;
  }
  data_ = nullptr;
  return NEURON_NO_ERROR;
}

int32_t AhwbNeuronMemoryWrapper::lockAhwbData(void **out_data_ptr) {
  if (!out_data_ptr) {
    LOG(ERROR) << "out_data_ptr is nullptr";
    return NEURON_UNEXPECTED_NULL;
  }
  if (data_) {
    *out_data_ptr = data_;
    return NEURON_NO_ERROR;
  }
  if (!abuffer_) {
    LOG(ERROR) << "No AHWB allocated";
    return NEURON_BAD_STATE;
  }
  if (AHardwareBuffer_lock(abuffer_, ahwb_type_, -1, nullptr, out_data_ptr) != 0) {
    LOG(ERROR) << "AHWB lock fail";
    return NEURON_BAD_STATE;
  }
  data_ = *out_data_ptr;
  return NEURON_NO_ERROR;
}

void *AhwbNeuronMemoryWrapper::data() {
  if (!data_) {
    void *temp_ptr = nullptr;
    if (lockAhwbData(&temp_ptr) != NEURON_NO_ERROR) {
      LOG(ERROR) << "Lock and aquire data failed";
      data_ = nullptr;
    } else {
      data_ = temp_ptr;
    }
  }
  return data_;
}

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
  auto real_input_size = backend->inputSizes[0] / backend->real_batch_size;
  int padded_length = real_input_size / sizeof(T);
  int original_length = bytes / sizeof(T);
  T *image_data = reinterpret_cast<T *>(data);

  int padded_num = padded_length - original_length;
  assert(padded_num >= 0);

  for (int num = 0; num < padded_num; num++) {
    image_data[padded_length - 1 - num * 4] = 0;
    image_data[padded_length - 2 - num * 4] = image_data[original_length - 1 - num * 3];
    image_data[padded_length - 3 - num * 4] = image_data[original_length - 2 - num * 3];
    image_data[padded_length - 4 - num * 4] = image_data[original_length - 3 - num * 3];
  }
}

bool NeuronSetInputsAndOutputsFromMemory(AdapterBackendData *backend_data) {
  if (!backend_data) return false;
  LOG(INFO) << "NeuronSetInputsAndOutputsFromMemory Start";
  const uint32_t shards_num = backend_data->shards_num;
  const int input_nums = backend_data->input_nums;
  const int output_nums = backend_data->output_nums;

  int err = NEURON_NO_ERROR;
  for (int t = 0; t < shards_num; t++) {
    for (int i = 0; i < input_nums; i++) {
      err = NeuronExecution_setInputFromMemory(
          backend_data->execution, i, nullptr,
          backend_data->inputMemoryWrappers[t * input_nums + i].memory(),
          0,
          backend_data->inputSizes[t * input_nums + i]);
      RETURN_FALSE_ON_ERR(err, "NeuronExecution_setInputFromMemory[" << i << "]");
    }

    for (int i = 0; i < output_nums; i++) {
      err = NeuronExecution_setOutputFromMemory(
          backend_data->execution, i, nullptr,
          backend_data->outputMemoryWrappers[t * output_nums + i].memory(),
          0,
          backend_data->outputSizes[t * output_nums + i]);
      RETURN_FALSE_ON_ERR(err, "NeuronExecution_setOutputFromMemory[" << i << "]");
    }
    if (backend_data->use_throughput_mode) {
      err = NeuronExecution_setIODone(backend_data->execution, t);
      RETURN_FALSE_ON_ERR(err, "NeuronExecution_setIODone failed");
    }
  }

  LOG(INFO) << "NeuronSetInputsAndOutputsFromMemory End Successfully";
  return true;
}

void ResizeInputsAndOutputsBuffers(AdapterBackendData *backend_data,
                                   uint32_t input_num, uint32_t output_num) {
  uint32_t shards_num = backend_data->shards_num;
  uint32_t input_buffer_num = input_num * shards_num;
  uint32_t output_buffer_num = output_num * shards_num;
  backend_data->input_nums = input_num;
  backend_data->inputTypes.resize(input_num);

  backend_data->output_nums = output_num;
  backend_data->outputTypes.resize(output_num);
}

bool create_mobilenet_edge_model(neuron_backend_ptr_t backend_ptr,
                                 const char *model) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;

  uint32_t real_batch_size = backend_data->real_batch_size;
  ResizeInputsAndOutputsBuffers(backend_data, 1, 1);

  // Handle input & output data type
  {  // Input
    neuron_data_t type;
    type.type = neuron_data_t::Uint8;
    type.size = real_batch_size * 224 * 224 * 3;
    backend_data->inputTypes[0] = type;
  }

  {  // Output
    neuron_data_t type;
    type.type = neuron_data_t::Uint8;
    type.size = real_batch_size * 1001;
    backend_data->outputTypes[0] = type;
  }

  // input op
  std::vector<uint32_t> dims_input = {real_batch_size, 224, 224, 3};
  NeuronOperandType tensor_Bx224x224x3Type{
      .type = NEURON_TENSOR_QUANT8_ASYMM,
      .scale = 0.007874015718698502f,
      .zeroPoint = 128,
      .dimensionCount = static_cast<uint32_t>(dims_input.size()),
      .dimensions = dims_input.data(),
  };

  // output op
  std::vector<uint32_t> dim_output = {real_batch_size, 1001};
  if (real_batch_size == 1) dim_output.erase(dim_output.begin());
  NeuronOperandType tensor_outputType{
      .type = NEURON_TENSOR_QUANT8_ASYMM,
      .scale = 1.0f,
      .zeroPoint = 0,
      .dimensionCount = static_cast<uint32_t>(dim_output.size()),
      .dimensions = dim_output.data(),
  };

  std::vector<NeuronOperandType> input_op = {tensor_Bx224x224x3Type};
  std::vector<NeuronOperandType> output_op = {tensor_outputType};

  NeuronBuilder builder;

  if (builder.setInputOperand(input_op)
          .setOutputOperand(output_op)
          .setSuppressConversion(TYPE_INPUT, true)
          .setSuppressConversion(TYPE_OUTPUT, false)
          .setAhwbType(AHWB_OFTEN)
          .setExecutionFlushValue(0)
          .setModelPath(model)
          .setPreference(backend_data->preference)
          .setThreadNumber(backend_data->thread_pool_size)
          .setShardsNumber(backend_data->shards_num)
          .setUseThroughputMode(backend_data->use_throughput_mode)
          .create(backend_ptr) == false) {
    LOG(ERROR) << "create Neuron error";
    return false;
  }

  backend_data->paddingFunc = &Padding<uint8_t>;
  return NeuronSetInputsAndOutputsFromMemory(backend_data);
}

bool create_mobilenet_v4_model(neuron_backend_ptr_t backend_ptr,
                               const char *model) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  uint32_t real_batch_size = backend_data->real_batch_size;
  ResizeInputsAndOutputsBuffers(backend_data, 1, 1);

  // Handle input & output data type
  {  // Input
    neuron_data_t type;
    type.type = neuron_data_t::Uint8;
    type.size = real_batch_size * 384 * 384 * 3;
    backend_data->inputTypes[0] = type;
  }

  {  // Output
    neuron_data_t type;
    type.type = neuron_data_t::Uint8;
    type.size = real_batch_size * 1000;
    backend_data->outputTypes[0] = type;
  }

  // input op
  std::vector<uint32_t> dims_input = {real_batch_size, 384, 384, 3};
  NeuronOperandType tensor_Bx224x224x3Type{
      .type = NEURON_TENSOR_QUANT8_ASYMM,
      .scale = 0.00787125,
      .zeroPoint = 128,
      .dimensionCount = static_cast<uint32_t>(dims_input.size()),
      .dimensions = dims_input.data(),
  };

  // output op
  std::vector<uint32_t> dim_output = {real_batch_size, 1000};
  if (real_batch_size == 1) dim_output.erase(dim_output.begin());
  NeuronOperandType tensor_outputType{
      .type = NEURON_TENSOR_QUANT8_ASYMM,
      .scale = 1.0f,
      .zeroPoint = 0,
      .dimensionCount = static_cast<uint32_t>(dim_output.size()),
      .dimensions = dim_output.data(),
  };

  std::vector<NeuronOperandType> input_op = {tensor_Bx224x224x3Type};
  std::vector<NeuronOperandType> output_op = {tensor_outputType};

  NeuronBuilder builder;

  if (builder.setInputOperand(input_op)
          .setOutputOperand(output_op)
          .setSuppressConversion(TYPE_INPUT, true)
          .setSuppressConversion(TYPE_OUTPUT, false)
          .setAhwbType(AHWB_OFTEN)
          .setExecutionFlushValue(0)
          .setModelPath(model)
          .setPreference(backend_data->preference)
          .setThreadNumber(backend_data->thread_pool_size)
          .setShardsNumber(backend_data->shards_num)
          .setUseThroughputMode(backend_data->use_throughput_mode)
          .create(backend_ptr) == false) {
    LOG(ERROR) << "create Neuron error";
    return false;
  }

  backend_data->paddingFunc = &Padding<uint8_t>;
  return NeuronSetInputsAndOutputsFromMemory(backend_data);
}

bool create_mobiledet_qat_model(neuron_backend_ptr_t backend_ptr,
                                const char *model) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  ResizeInputsAndOutputsBuffers(backend_data, 1, 4);

  // Handle input & output data type
  {  // Input
    neuron_data_t type;
    type.type = neuron_data_t::Uint8;
    type.size = 1 * 320 * 320 * 3;
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
          .setPreference(backend_data->preference)
          .setThreadNumber(backend_data->thread_pool_size)
          .setShardsNumber(backend_data->shards_num)
          .setUseThroughputMode(backend_data->use_throughput_mode)
          .create(backend_ptr) == false) {
    LOG(ERROR) << "create Neuron error";
    return false;
  }

  backend_data->paddingFunc = &Padding<uint8_t>;
  return NeuronSetInputsAndOutputsFromMemory(backend_data);
}

bool create_mobilebert_nnapi_model(neuron_backend_ptr_t backend_ptr,
                                   const char *model) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  ResizeInputsAndOutputsBuffers(backend_data, 3, 2);

  // Handle input & output data type
  // Input
  for (int i = 0; i < backend_data->input_nums; i++) {
    neuron_data_t type;
    type.type = neuron_data_t::Int32;
    type.size = 1 * 384;
    backend_data->inputTypes[i] = type;
  }

  // Output
  for (int i = 0; i < backend_data->output_nums; i++) {
    neuron_data_t type;
    type.type = neuron_data_t::Float32;
    type.size = 1 * 1 * 1 * 384;
    backend_data->outputTypes[i] = type;
  }

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
          .setPreference(backend_data->preference)
          .setThreadNumber(backend_data->thread_pool_size)
          .setShardsNumber(backend_data->shards_num)
          .setUseThroughputMode(backend_data->use_throughput_mode)
          .create(backend_ptr) == false) {
    LOG(ERROR) << "create Neuron error";
    return false;
  }

  return NeuronSetInputsAndOutputsFromMemory(backend_data);
}

bool create_mobile_segmenter_model(neuron_backend_ptr_t backend_ptr,
                                   const char *model) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  ResizeInputsAndOutputsBuffers(backend_data, 1, 1);

  // Handle input & output data type
  {  // Input
    neuron_data_t type;
    type.type = neuron_data_t::Uint8;
    type.size = 1 * 512 * 512 * 3;
    backend_data->inputTypes[0] = type;
  }

  {  // Output
    neuron_data_t type;
    type.type = neuron_data_t::Int32;
    type.size = 1 * 1 * 512 * 512;
    backend_data->outputTypes[0] = type;
  }

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
          .setPreference(backend_data->preference)
          .setThreadNumber(backend_data->thread_pool_size)
          .setShardsNumber(backend_data->shards_num)
          .setUseThroughputMode(backend_data->use_throughput_mode)
          .create(backend_ptr) == false) {
    LOG(ERROR) << "create Neuron error";
    return false;
  }

  backend_data->paddingFunc = &Padding<uint8_t>;
  return NeuronSetInputsAndOutputsFromMemory(backend_data);
}

bool create_edsr_model(neuron_backend_ptr_t backend_ptr, const char *model) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  ResizeInputsAndOutputsBuffers(backend_data, 1, 1);

  // Handle input & output data type
  {  // Input
    neuron_data_t type;
    type.type = neuron_data_t::Int8;
    type.size = 1 * 540 * 960 * 3;
    backend_data->inputTypes[0] = type;
  }

  {  // Output
    neuron_data_t type;
    type.type = neuron_data_t::Int8;
    type.size = 1 * 1080 * 1920 * 3;
    backend_data->outputTypes[0] = type;
  }

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
          .setPreference(backend_data->preference)
          .setThreadNumber(backend_data->thread_pool_size)
          .setShardsNumber(backend_data->shards_num)
          .setUseThroughputMode(backend_data->use_throughput_mode)
          .create(backend_ptr) == false) {
    LOG(ERROR) << "create Neuron error";
    return false;
  }

  backend_data->paddingFunc = &Padding<int8_t>;
  return NeuronSetInputsAndOutputsFromMemory(backend_data);
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

std::string InsertFolderAfterTargetFolder(const std::string &input,
                                          const std::string &target_folder,
                                          const std::string &folder) {
  size_t pos = input.find(target_folder);
  if (pos == std::string::npos) {
    // Directory not found, return the original input
    return input;
  }
  // Add the length of the directory to the position
  pos += target_folder.length();
  // Insert '/test' after the directory
  std::string output = input.substr(0, pos) + "/" + folder + input.substr(pos);
  return output;
};

void ReadOfflineProperties(AdapterBackendData *backend_data,
                           uint32_t model_batch_size, uint32_t shrads_num,
                           uint32_t thread_pool_size) {
#if defined(__ANDROID__)
  char debug_mlperf_offline_shrads_num[PROP_VALUE_MAX + 1];
  if (__system_property_get("debug.mlperf.offline_shrads_num",
                            debug_mlperf_offline_shrads_num)) {
    shrads_num = atoi(debug_mlperf_offline_shrads_num);
  }
  char debug_mlperf_offline_thread_pool_size[PROP_VALUE_MAX + 1];
  if (__system_property_get("debug.mlperf.offline_thread_pool_size",
                            debug_mlperf_offline_thread_pool_size)) {
    thread_pool_size = atoi(debug_mlperf_offline_thread_pool_size);
  }
  char debug_mlperf_offline_model_batch_size[PROP_VALUE_MAX + 1];
  if (__system_property_get("debug.mlperf.offline_model_batch_size",
                            debug_mlperf_offline_model_batch_size)) {
    model_batch_size = atoi(debug_mlperf_offline_model_batch_size);
  }
  auto preference = NEURON_PREFER_FAST_SINGLE_ANSWER;
  char debug_mlperf_offline_preference[PROP_VALUE_MAX + 1];
  if (__system_property_get("debug.mlperf.offline_preference",
                            debug_mlperf_offline_preference)) {
    preference = static_cast<NeuronAdapterPreferenceCode>(
        max(0, min(atoi(debug_mlperf_offline_preference), 3)));
  }
  backend_data->preference = preference;
  backend_data->thread_pool_size = thread_pool_size;
  backend_data->shards_num = shrads_num;
  // Originally, real_batch_size is the whole batch size passed in from mlperf
  // Now, it's model_batch_size
  backend_data->real_batch_size = model_batch_size;
  backend_data->use_throughput_mode = true;
#endif
}

bool CreateNeuronModel(neuron_backend_ptr_t backend_ptr,
                       const std::string &model) {
  LOG(INFO) << "create_neuron_backend Create model: " << model << " Start";
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;

  bool create_result = false;
  if (model.find("mobilenet_edgetpu_224_1.0_uint8.dla") != std::string::npos) {
    create_result = create_mobilenet_edge_model(backend_ptr, model.c_str());
  } else if (model.find("MobileNetV4-Conv-Large-int8-ptq.dla") != std::string::npos) {
    create_result = create_mobilenet_v4_model(backend_ptr, model.c_str());
  } else if (model.find("mobilenet_edgetpu_224_1.0_uint8_offline.dla") != std::string::npos) {
    ReadOfflineProperties(backend_data, /*model_batch_size*/ 50,
                          /*shrads_num*/ 8, /*thread_pool_size*/ 16);
    create_result = create_mobilenet_edge_model(backend_ptr, model.c_str());
  } else if (model.find("MobileNetV4-Conv-Large-int8-ptq_offline.dla") != std::string::npos) {
    ReadOfflineProperties(backend_data, /*model_batch_size*/ 4,
                          /*shrads_num*/ 8, /*thread_pool_size*/ 16);
    create_result = create_mobilenet_v4_model(backend_ptr, model.c_str());
  } else if (model.find("mobiledet_qat.dla") != std::string::npos) {
    create_result = create_mobiledet_qat_model(backend_ptr, model.c_str());
  } else if (model.find("mobile_segmenter_r4_quant_argmax_uint8.dla") != std::string::npos) {
    create_result = create_mobile_segmenter_model(backend_ptr, model.c_str());
  } else if (model.find("mobilebert_int8_384_nnapi.dla") != std::string::npos) {
    create_result = create_mobilebert_nnapi_model(backend_ptr, model.c_str());
  } else if (model.find("edsr_f32b5_full_qint8.dla") != std::string::npos) {
    create_result = create_edsr_model(backend_ptr, model.c_str());
  } else {
    LOG(ERROR) << "Not a supported dla";
    return false;
  }
  LOG(INFO) << "create_neuron_backend Create model: " << model << " End, "
            << "shrads_num: " << backend_data->shards_num
            << ", real_batch_size(model batch size): "
            << backend_data->real_batch_size;
  return create_result;
}

bool create_neuron_backend(neuron_backend_ptr_t backend_ptr,
                           const char *model_path) {
  LOG(INFO) << "create_neuron_backend Start.";
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  backend_data->useNeuronBackend = true;

  std::string modified_model_path = InsertFolderAfterTargetFolder(
      std::string(model_path), GetPlatformName(), "test");
  if (!CreateNeuronModel(backend_ptr, modified_model_path)) {
    LOG(ERROR) << "create_neuron_backend create model FAILED";
    return false;
  }
  // PERFORMANCE_ON;
  backend_data->locker.Start(0);  // Powerhal infinite times

  LOG(INFO) << "create_neuron_backend End Successfully";
  return true;
}

bool delete_neuron_backend(neuron_backend_ptr_t backend_ptr) {
  LOG(INFO) << "delete_neuron_backend Start";
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;

  if (!backend_data->useNeuronBackend) return false;

  // Delete executin before releasing memory in case of inferencing on invalid buffer
  NeuronExecution_free(backend_data->execution);
  NeuronCompilation_free(backend_data->compilation);
  NeuronModel_free(backend_data->model);

  // released memory
  backend_data->inputMemoryWrappers.clear();
  backend_data->outputMemoryWrappers.clear();
  backend_data->inputTypes.clear();
  backend_data->outputTypes.clear();

  LOG(INFO) << "delete_neuron_backend End";
  return true;
}

bool neuron_get_in_out_count(neuron_backend_ptr_t backend_ptr, bool isIn,
                             int32_t *count) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  if (!backend_data->useNeuronBackend) return false;
  *count = isIn ? backend_data->input_nums : backend_data->output_nums;
  return true;
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

bool neuron_set_input(neuron_backend_ptr_t backend_ptr, int32_t batch_index,
                      int32_t i, void *data_src) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  if (unlikely(!backend_data->useNeuronBackend)) return false;
#ifdef SET_INPUT_FROM_MEMORY
  // If the input data is cached, use the cached NeuronMemory first.
  if (backend_data->neuronAllocator.IsEnable()) {
    auto [base, offset, memory] =
        backend_data->neuronAllocator.GetCachedMemory(data_src);
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
  const int model_batch_size = backend_data->real_batch_size;  // 50
  const int one_input_size = backend_data->inputSizes[i] / model_batch_size;

  // TRACER("memcpy");
  if (backend_data->use_throughput_mode) {
    return NeuronExecution_submitInput(backend_data->execution, i, data_src, one_input_size) == NEURON_NO_ERROR;
  } else {
    memcpy(backend_data->inputMemoryWrappers[i].data(), data_src, one_input_size);
  }
  return true;
}

bool neuron_flush_queries(neuron_backend_ptr_t backend_ptr) {
  AdapterBackendData *const backend_data = (AdapterBackendData *)backend_ptr;
  if (backend_data->use_throughput_mode) {
    if (NeuronExecution_resetParallelResources(backend_data->execution) != NEURON_NO_ERROR) {
      return false;
    }
  }
  return true;
}

bool neuron_get_output(neuron_backend_ptr_t backend_ptr, int32_t batch_index,
                       int32_t i, void **data) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  if (unlikely(!backend_data->useNeuronBackend)) return false;

  if (backend_data->use_throughput_mode) {
    return NeuronExecution_queryOutputPointer(backend_data->execution, i, data) == NEURON_NO_ERROR;
  } else {
    *data = backend_data->outputMemoryWrappers[i].data();
  }
  return true;
}

bool neuron_issue_query(neuron_backend_ptr_t backend_ptr) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  if (unlikely(!backend_data->useNeuronBackend)) return false;

  if (backend_data->use_throughput_mode) {
    return NeuronExecution_flushExecution(backend_data->execution) == NEURON_NO_ERROR;
  } else {
    return NeuronExecution_compute(backend_data->execution) == NEURON_NO_ERROR;
  }
}

bool neuron_convert_input(neuron_backend_ptr_t backend_ptr, int bytes,
                          void *data) {
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  if (!backend_data->useNeuronBackend) return false;

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
