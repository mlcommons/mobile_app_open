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
#include "neuron_utils.h"
#include "tensorflow/core/platform/logging.h"

#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

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
    image_data[padded_length - 2 - num * 4] =
        image_data[original_length - 1 - num * 3];
    image_data[padded_length - 3 - num * 4] =
        image_data[original_length - 2 - num * 3];
    image_data[padded_length - 4 - num * 4] =
        image_data[original_length - 3 - num * 3];
  }
}

bool NeuronSetInputsAndOutputsFromMemory(AdapterBackendData *backend_data) {
  if (!backend_data) return false;
  LOG(INFO) << "NeuronSetInputsAndOutputsFromMemory Start";
  const uint32_t exec_num = backend_data->exec_num;
  const int input_nums = backend_data->input_nums;
  const int output_nums = backend_data->output_nums;

  int err = NEURON_NO_ERROR;
  for (int t = 0; t < exec_num; t++) {
    for (int i = 0; i < input_nums; i++) {
      err = NeuronExecution_setInputFromMemory(
          backend_data->execution, i, nullptr,
          backend_data->inputMemoryWrappers[t * input_nums + i]->memory(), 0,
          backend_data->inputSizes[t * input_nums + i]);
      RETURN_FALSE_ON_ERR(err,
                          "NeuronExecution_setInputFromMemory[" << i << "]");
    }

    for (int i = 0; i < output_nums; i++) {
      err = NeuronExecution_setOutputFromMemory(
          backend_data->execution, i, nullptr,
          backend_data->outputMemoryWrappers[t * output_nums + i]->memory(), 0,
          backend_data->outputSizes[t * output_nums + i]);
      RETURN_FALSE_ON_ERR(err,
                          "NeuronExecution_setOutputFromMemory[" << i << "]");
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
  uint32_t exec_num = backend_data->exec_num;
  uint32_t input_buffer_num = input_num * exec_num;
  uint32_t output_buffer_num = output_num * exec_num;
  backend_data->input_nums = input_num;
  backend_data->inputTypes.resize(input_num);

  backend_data->output_nums = output_num;
  backend_data->outputTypes.resize(output_num);
}

bool create_mobilenet_edge_model(neuron_backend_ptr_t backend_ptr,
                                 const char *model) {
  Tracer trace("create_mobilenet_edge_model");
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
          .setBufferType(CACHEABLE)
          .setDisableCoherentBuffer(backend_data->disableCoherentBuffer)
          .setModelPath(model)
          .setPreference(backend_data->preference)
          .setThreadNumber(backend_data->thread_pool_size)
          .setExecutorNumber(backend_data->exec_num)
          .setUseThroughputMode(backend_data->use_throughput_mode)
          .setDmaBufferAllocator(backend_data->bufferAllocator.get())
          .create(backend_ptr) == false) {
    LOG(ERROR) << "create Neuron error";
    return false;
  }

  backend_data->paddingFunc = &Padding<uint8_t>;
  return NeuronSetInputsAndOutputsFromMemory(backend_data);
}

bool create_mobilenet_v4_model(neuron_backend_ptr_t backend_ptr,
                               const char *model) {
  Tracer trace("create_mobilenet_v4_model");
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
          .setBufferType(CACHEABLE)
          .setDisableCoherentBuffer(backend_data->disableCoherentBuffer)
          .setModelPath(model)
          .setPreference(backend_data->preference)
          .setThreadNumber(backend_data->thread_pool_size)
          .setExecutorNumber(backend_data->exec_num)
          .setUseThroughputMode(backend_data->use_throughput_mode)
          .setDmaBufferAllocator(backend_data->bufferAllocator.get())
          .create(backend_ptr) == false) {
    LOG(ERROR) << "create Neuron error";
    return false;
  }

  backend_data->paddingFunc = &Padding<uint8_t>;
  return NeuronSetInputsAndOutputsFromMemory(backend_data);
}

bool create_mobiledet_qat_model(neuron_backend_ptr_t backend_ptr,
                                const char *model) {
  Tracer trace("create_mobiledet_qat_model");
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
          .setBufferType(UNCACHED)
          .setDisableCoherentBuffer(backend_data->disableCoherentBuffer)
          .setModelPath(model)
          .setPreference(backend_data->preference)
          .setThreadNumber(backend_data->thread_pool_size)
          .setExecutorNumber(backend_data->exec_num)
          .setUseThroughputMode(backend_data->use_throughput_mode)
          .setDmaBufferAllocator(backend_data->bufferAllocator.get())
          .create(backend_ptr) == false) {
    LOG(ERROR) << "create Neuron error";
    return false;
  }

  backend_data->paddingFunc = &Padding<uint8_t>;
  return NeuronSetInputsAndOutputsFromMemory(backend_data);
}

bool create_mobilebert_nnapi_model(neuron_backend_ptr_t backend_ptr,
                                   const char *model) {
  Tracer trace("create_mobilebert_nnapi_model");
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
          .setBufferType(CACHEABLE)
          .setDisableCoherentBuffer(backend_data->disableCoherentBuffer)
          .setModelPath(model)
          .setPreference(backend_data->preference)
          .setThreadNumber(backend_data->thread_pool_size)
          .setExecutorNumber(backend_data->exec_num)
          .setUseThroughputMode(backend_data->use_throughput_mode)
          .setDmaBufferAllocator(backend_data->bufferAllocator.get())
          .create(backend_ptr) == false) {
    LOG(ERROR) << "create Neuron error";
    return false;
  }

  return NeuronSetInputsAndOutputsFromMemory(backend_data);
}

bool create_mobile_segmenter_model(neuron_backend_ptr_t backend_ptr,
                                   const char *model) {
  Tracer trace("create_mobile_segmenter_model");
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
          .setBufferType(UNCACHED)
          .setDisableCoherentBuffer(backend_data->disableCoherentBuffer)
          .setModelPath(model)
          .setPreference(backend_data->preference)
          .setThreadNumber(backend_data->thread_pool_size)
          .setExecutorNumber(backend_data->exec_num)
          .setUseThroughputMode(backend_data->use_throughput_mode)
          .setDmaBufferAllocator(backend_data->bufferAllocator.get())
          .create(backend_ptr) == false) {
    LOG(ERROR) << "create Neuron error";
    return false;
  }

  backend_data->paddingFunc = &Padding<uint8_t>;
  return NeuronSetInputsAndOutputsFromMemory(backend_data);
}

bool create_edsr_model(neuron_backend_ptr_t backend_ptr, const char *model) {
  Tracer trace("create_edsr_model");
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
          .setBufferType(CACHEABLE)
          .setDisableCoherentBuffer(backend_data->disableCoherentBuffer)
          .setModelPath(model)
          .setPreference(backend_data->preference)
          .setThreadNumber(backend_data->thread_pool_size)
          .setExecutorNumber(backend_data->exec_num)
          .setUseThroughputMode(backend_data->use_throughput_mode)
          .setDmaBufferAllocator(backend_data->bufferAllocator.get())
          .setPerfParam(mtk::performance::kEdsrParams)
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

void ReadOfflineProperties(AdapterBackendData *backend_data) {
#if defined(__ANDROID__)
  char debug_mlperf_offline_exec_num[PROP_VALUE_MAX + 1];
  if (__system_property_get("debug.mlperf.offline_exec_num",
                            debug_mlperf_offline_exec_num)) {
    backend_data->exec_num = atoi(debug_mlperf_offline_exec_num);
  }
  char debug_mlperf_offline_thread_pool_size[PROP_VALUE_MAX + 1];
  if (__system_property_get("debug.mlperf.offline_thread_pool_size",
                            debug_mlperf_offline_thread_pool_size)) {
    backend_data->thread_pool_size =
        atoi(debug_mlperf_offline_thread_pool_size);
  }
  char debug_mlperf_offline_model_batch_size[PROP_VALUE_MAX + 1];
  if (__system_property_get("debug.mlperf.offline_model_batch_size",
                            debug_mlperf_offline_model_batch_size)) {
    backend_data->real_batch_size = atoi(debug_mlperf_offline_model_batch_size);
  }
  auto preference = NEURON_PREFER_FAST_SINGLE_ANSWER;
  char debug_mlperf_offline_preference[PROP_VALUE_MAX + 1];
  if (__system_property_get("debug.mlperf.offline_preference",
                            debug_mlperf_offline_preference)) {
    preference = static_cast<NeuronAdapterPreferenceCode>(
        max(0, min(atoi(debug_mlperf_offline_preference), 3)));
  }
  backend_data->preference = preference;
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
  } else if (model.find("MobileNetV4-Conv-Large-int8-ptq.dla") !=
             std::string::npos) {
    create_result = create_mobilenet_v4_model(backend_ptr, model.c_str());
  } else if (model.find("mobilenet_edgetpu_224_1.0_uint8_offline.dla") !=
             std::string::npos) {
    ReadOfflineProperties(backend_data);
    create_result = create_mobilenet_edge_model(backend_ptr, model.c_str());
  } else if (model.find("MobileNetV4-Conv-Large-int8-ptq_offline.dla") !=
             std::string::npos) {
    ReadOfflineProperties(backend_data);
    create_result = create_mobilenet_v4_model(backend_ptr, model.c_str());
  } else if (model.find("mobiledet_qat.dla") != std::string::npos) {
    create_result = create_mobiledet_qat_model(backend_ptr, model.c_str());
  } else if (model.find("mobile_segmenter_r4_quant_argmax_uint8.dla") !=
             std::string::npos) {
    create_result = create_mobile_segmenter_model(backend_ptr, model.c_str());
  } else if (model.find("mobilebert_int8_384_nnapi.dla") != std::string::npos) {
    create_result = create_mobilebert_nnapi_model(backend_ptr, model.c_str());
  } else if (model.find("edsr_f32b5_full_qint8.dla") != std::string::npos) {
    create_result = create_edsr_model(backend_ptr, model.c_str());
  } else {
    LOG(ERROR) << "Not a supported dla";
    return false;
  }
  if (!create_result) {
    LOG(ERROR) << "Failed to create neuron model";
  } else {
    LOG(INFO) << "create_neuron_backend Create model: " << model << " End";
  }
  LOG(INFO) << "Neuron Backend Configs: "
            << "\n| exec_num: " << backend_data->exec_num
            << "\n| real_batch_size(model batch size): "
            << backend_data->real_batch_size
            << "\n| thread_pool_size: " << backend_data->thread_pool_size
            << "\n| preference: " << backend_data->preference
            << "\n| input_nums: " << backend_data->input_nums
            << "\n| output_nums: " << backend_data->output_nums
            << "\n| use_throughput_mode: " << backend_data->use_throughput_mode
            << "\n| disableCoherentBuffer: "
            << backend_data->disableCoherentBuffer;
  return create_result;
}

bool ReadU32ValueFromStr(const char *str, uint32_t &output) {
  char *end;
  uint32_t value = std::strtoul(str, &end, 10);
  if (*end != '\0') return false;
  output = value;
  return true;
}

bool ReadNeuronBackendConfigs(AdapterBackendData *backend_data,
                              mlperf_backend_configuration_t *configs) {
  for (size_t i = 0; i < configs->count; ++i) {
    const char *k = configs->keys[i];
    const char *v = configs->values[i];

    if (strcmp(k, "thread_pool_size") == 0) {
      if (!ReadU32ValueFromStr(v, backend_data->thread_pool_size)) {
        LOG(ERROR) << "Failed to read neuron thread_pool_size value";
        return false;
      }
    } else if (strcmp(k, "model_batch_size") == 0) {
      // Originally, real_batch_size is the whole batch size passed in from
      // mlperf Now, it's model_batch_size
      if (!ReadU32ValueFromStr(v, backend_data->real_batch_size)) {
        LOG(ERROR) << "Failed to read neuron model_batch_size value";
        return false;
      }
    } else if (strcmp(k, "exec_num") == 0) {
      if (!ReadU32ValueFromStr(v, backend_data->exec_num)) {
        LOG(ERROR) << "Failed to read neuron exec_num value";
        return false;
      }
    } else if (strcmp(k, "disable_coherent") == 0) {
      if (strcmp(v, "true") == 0) {
        backend_data->disableCoherentBuffer = true;
      } else {
        backend_data->disableCoherentBuffer = false;
      }
    }
  }
  return true;
}

bool create_neuron_backend(neuron_backend_ptr_t backend_ptr,
                           mlperf_backend_configuration_t *configs,
                           const char *model_path) {
  LOG(INFO) << "create_neuron_backend Start.";
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  backend_data->useNeuronBackend = true;
  if (!ReadNeuronBackendConfigs(backend_data, configs)) {
    LOG(ERROR) << "Failed to read neuron backend configs";
    return false;
  }
  backend_data->bufferAllocator = std::make_unique<DmaBufferAllocatorWrapper>();

  if (!CreateNeuronModel(backend_ptr, model_path)) {
    LOG(ERROR) << "create_neuron_backend create model FAILED";
    return false;
  }
  // PERFORMANCE_ON;
  if (!Config::GetDisablePerformanceLocker()) {
    LOG(INFO) << "PerformanceLocker started";
    backend_data->locker.Start(0);  // Powerhal infinite times
  }

  LOG(INFO) << "create_neuron_backend End Successfully";
  return true;
}

bool delete_neuron_backend(neuron_backend_ptr_t backend_ptr) {
  Tracer trace("delete_neuron_backend");
  LOG(INFO) << "delete_neuron_backend Start";
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;

  if (!backend_data->useNeuronBackend) return false;

  // Delete executin before releasing memory in case of inferencing on invalid
  // buffer
  NeuronExecution_free(backend_data->execution);
  NeuronCompilation_free(backend_data->compilation);
  NeuronModel_free(backend_data->model);

  // released memory
  backend_data->inputMemoryWrappers.clear();
  backend_data->outputMemoryWrappers.clear();
  backend_data->inputTypes.clear();
  backend_data->outputTypes.clear();
  backend_data->bufferAllocator.reset();

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
  Tracer trace("neuron_set_input");
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  if (unlikely(!backend_data->useNeuronBackend)) return false;
  const int model_batch_size = backend_data->real_batch_size;  // 50
  const int one_input_size = backend_data->inputSizes[i] / model_batch_size;

  if (backend_data->use_throughput_mode) {
    return NeuronExecution_submitInput(backend_data->execution, i, data_src,
                                       one_input_size) == NEURON_NO_ERROR;
  } else {
    Tracer trace("SetInput:memcpy");
    memcpy(backend_data->inputMemoryWrappers[i]->data(), data_src,
           one_input_size);
  }
  return true;
}

bool neuron_flush_queries(neuron_backend_ptr_t backend_ptr) {
  AdapterBackendData *const backend_data = (AdapterBackendData *)backend_ptr;
  if (backend_data->use_throughput_mode) {
    if (NeuronExecution_resetParallelResources(backend_data->execution) !=
        NEURON_NO_ERROR) {
      return false;
    }
  }
  return true;
}

bool neuron_get_output(neuron_backend_ptr_t backend_ptr, int32_t batch_index,
                       int32_t i, void **data) {
  Tracer trace("neuron_get_output");
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  if (unlikely(!backend_data->useNeuronBackend)) return false;

  if (backend_data->use_throughput_mode) {
    return NeuronExecution_queryOutputPointer(backend_data->execution, i,
                                              data) == NEURON_NO_ERROR;
  } else {
    *data = backend_data->outputMemoryWrappers[i]->data();
  }
  return true;
}

bool neuron_issue_query(neuron_backend_ptr_t backend_ptr) {
  Tracer trace("neuron_issue_query");
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  if (unlikely(!backend_data->useNeuronBackend)) return false;

  if (backend_data->use_throughput_mode) {
    return NeuronExecution_flushExecution(backend_data->execution) ==
           NEURON_NO_ERROR;
  } else {
    return NeuronExecution_compute(backend_data->execution) == NEURON_NO_ERROR;
  }
}

bool neuron_convert_input(neuron_backend_ptr_t backend_ptr, int bytes,
                          void *data) {
  Tracer trace("neuron_convert_input");
  AdapterBackendData *backend_data = (AdapterBackendData *)backend_ptr;
  if (!backend_data->useNeuronBackend) return false;

  if (backend_data->paddingFunc) {
    backend_data->paddingFunc(backend_data, bytes, data);
  }

  return true;
}
#endif
