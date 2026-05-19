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
 * @file neuron_backend.h
 */

#pragma once

#if MTK_TFLITE_NEURON_BACKEND

#define RESTORE_DLA_EXTENSION_OPERAND_TYPE 0x0100
#define RESTORE_DLA_EXTENSION_OPERATION_TYPE 0x0000
#define RESTORE_DLA_EXTENSION_NAME "com.mediatek.compiled_network"

#include <chrono>
#include <cstdint>
#include <tuple>
#include <unordered_map>

#include "APUWareUtilsLib.h"
#include "NeuronAdapter.h"
#include "NeuronAdapterShim.h"
#include "flutter/cpp/c/type.h"
#include "neuron_memory.h"

#define RETURN_FALSE_ON_ERR(ret, msg)            \
  do {                                           \
    if (auto r = (ret); r != NEURON_NO_ERROR) {  \
      LOG(ERROR) << msg << " with error: " << r; \
      return false;                              \
    }                                            \
  } while (false)

typedef void *neuron_backend_ptr_t;

// mapping to mlperf_data_t
typedef struct {
  enum Type {
    Float32 = mlperf_data_t::Float32,
    Uint8 = mlperf_data_t::Uint8,
    Int8 = mlperf_data_t::Int8,
    Float16 = mlperf_data_t::Float16,
    Int32 = mlperf_data_t::Int32,
    Int64 = mlperf_data_t::Int64,
  };

  enum Type type;
  int64_t size;
} neuron_data_t;

struct AdapterBackendData {
  const char *name = "Adapter";
  const char *vendor = "Mediatek";
  NeuronModel *model{nullptr};
  NeuronCompilation *compilation{nullptr};
  NeuronExecution *execution{nullptr};
  std::vector<std::unique_ptr<NeuronMemoryWrapper>> inputMemoryWrappers;
  std::vector<std::unique_ptr<NeuronMemoryWrapper>> outputMemoryWrappers;
  std::vector<neuron_data_t> inputTypes{};
  std::vector<neuron_data_t> outputTypes{};
  std::vector<uint32_t> inputSizes{};
  std::vector<uint32_t> outputSizes{};
  int32_t input_nums = 0;
  int32_t output_nums = 0;
  bool useNeuronBackend = false;
  bool disableCoherentBuffer = true;
  mtk::performance::PerformanceLocker locker;
  //--------------Allocator----------------
  std::unique_ptr<DmaBufferAllocatorWrapper> bufferAllocator = nullptr;
  //---------------------------------------
  std::function<void(AdapterBackendData *, int, void *)> paddingFunc = nullptr;
  //------------- Throughput Mode ---------------
  uint32_t real_batch_size = 1;
  uint32_t exec_num = 1;
  bool use_throughput_mode = false;
  uint32_t thread_pool_size = 4;
  //------------- Preference ------------------
  NeuronAdapterPreferenceCode preference = NEURON_PREFER_TURBO_BOOST;
};

bool need_neuron_backend(const char *model_path);

bool create_neuron_backend(neuron_backend_ptr_t backend_ptr,
                           mlperf_backend_configuration_t *configs,
                           const char *model_path);

bool delete_neuron_backend(neuron_backend_ptr_t backend_ptr);

bool neuron_get_in_out_count(neuron_backend_ptr_t backend_ptr, bool isIn,
                             int32_t *count);

bool neuron_get_in_out_datatype(neuron_backend_ptr_t backend_ptr, int32_t i,
                                bool isIn, neuron_data_t *type);

bool neuron_set_input(neuron_backend_ptr_t backend_ptr, int batch_index,
                      int32_t i, void *data);

bool neuron_get_output(neuron_backend_ptr_t backend_ptr, int batch_index,
                       int32_t i, void **data);

bool neuron_flush_queries(neuron_backend_ptr_t backend_ptr);

bool neuron_issue_query(neuron_backend_ptr_t backend_ptr);

bool neuron_convert_input(neuron_backend_ptr_t backend_ptr, int bytes,
                          void *data);

std::string GetPlatformName();

#endif
