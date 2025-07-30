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

#include "flutter/cpp/c/type.h"

#include <chrono>
#include <cstdint>
#include <tuple>
#include <unordered_map>

#include "APUWareUtilsLib.h"
#include "NeuronAdapter.h"
#include "NeuronAdapterShim.h"
#include "mobile_back_tflite/cpp/backend_tflite/thread_pool.h"

#define RETURN_FALSE_ON_ERR(ret, msg)               \
  do {                                              \
    if ((ret) != NEURON_NO_ERROR) {                 \
      LOG(ERROR) << msg << " with error: " << (ret); \
      return false;                                 \
    }                                               \
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

class Allocator {
 public:
  Allocator(size_t size, size_t batch) {
    // LOG(INFO) << "Allocator for size: " << size << " created";
    mBlockSize = size * batch;
    mBatchSize = batch;
    mDataSize = size;
  }

  void *AddCapacity() {
    auto usage = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN |
                 AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
    AHardwareBuffer_Desc iDesc{
        .width = static_cast<uint32_t>(mBlockSize),
        .height = 1,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_BLOB,
        .usage = usage,
        .stride = static_cast<uint32_t>(mBlockSize),
    };
    AHardwareBuffer *Abuffer = nullptr;
    AHardwareBuffer_allocate(&iDesc, &Abuffer);
    if (Abuffer == nullptr) {
      LOG(ERROR) << "AHWB allocate fail";
      return nullptr;
    }
    extraAhwbs.push_back(Abuffer);

    void *buffer = nullptr;
    AHardwareBuffer_lock(Abuffer, usage, -1, nullptr, &buffer);
    if (buffer == nullptr) {
      LOG(ERROR) << "AHardwareBuffer_lock fail";
      return nullptr;
    }
    extraBuffers.push_back(buffer);

    NeuronMemory *memory = nullptr;
    NeuronMemory_createFromAHardwareBuffer(Abuffer, &memory);
    if (memory == nullptr) {
      LOG(ERROR) << "NeuronMemory create fail";
      return nullptr;
    }
    extraMemorys.push_back(memory);
    return buffer;
  }

  std::tuple<void *, size_t, NeuronMemory *> GetBuffer() {
    if (mDataNum % mBatchSize == 0) {
      if (AddCapacity() == nullptr) {
        LOG(ERROR) << "AddCapacity failed";
      }
    }
    void *base = extraBuffers[mDataNum / mBatchSize];
    size_t offsets = mDataSize * (mDataNum % mBatchSize);
    NeuronMemory *memory = extraMemorys[mDataNum / mBatchSize];
    mDataNum++;
    // LOG(INFO) << "mDataNum : " << mDataNum;
    return std::make_tuple(base, offsets, memory);
  }

  ~Allocator() {
    for (auto &memory : extraMemorys) {
      if (memory != nullptr) {
        NeuronMemory_free(memory);
      }
    }
    for (auto &ahwb : extraAhwbs) {
      if (ahwb != nullptr) {
        AHardwareBuffer_unlock(ahwb, nullptr);
        AHardwareBuffer_release(ahwb);
      }
    }
    extraMemorys.clear();
    extraAhwbs.clear();
    extraBuffers.clear();
    LOG(INFO) << "Allocator for size: " << mDataSize
              << " with data number: " << mDataNum << " deleted";
  }

  std::pair<size_t, std::vector<NeuronMemory *>> GetAllocatedNeuronMemory() {
    return {mBlockSize, extraMemorys};
  }

 private:
  std::vector<void *> extraBuffers{};
  std::vector<AHardwareBuffer *> extraAhwbs{};
  std::vector<NeuronMemory *> extraMemorys{};
  size_t mDataNum = 0;
  size_t mBlockSize = 0;
  size_t mDataSize = 0;
  size_t mBatchSize = 0;
};

class AhwbNeuronMemoryWrapper {
 public:
  explicit AhwbNeuronMemoryWrapper(
      uint32_t byte_size,
      uint64_t ahwb_type = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN |
                           AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN) : ahwb_type_(ahwb_type),
                                                                    size_(byte_size) {}
  ~AhwbNeuronMemoryWrapper() {
    if (data_) unlockAhwbData();
    if (memory_) NeuronMemory_free(memory_);
    if (abuffer_) AHardwareBuffer_release(abuffer_);
  }
  AhwbNeuronMemoryWrapper(AhwbNeuronMemoryWrapper &&other) { *this = std::move(other); };
  AhwbNeuronMemoryWrapper &operator=(AhwbNeuronMemoryWrapper &&other) {
    if (this != &other) {
      AhwbNeuronMemoryWrapper::~AhwbNeuronMemoryWrapper();
      this->data_ = other.data_;
      this->ahwb_type_ = other.ahwb_type_;
      this->abuffer_ = other.abuffer_;
      this->size_ = other.size_;
      this->memory_ = other.memory_;
      other.data_ = nullptr;
      other.abuffer_ = nullptr;
      other.memory_ = nullptr;
      other.size_ = 0;
    }
    return *this;
  }

  NeuronMemory *memory() { return memory_; }
  uint32_t size() { return size_; }
  void *data();
  bool Available() { return memory_ != nullptr; }
  int32_t InitNeuronMemory();
  int32_t lockAhwbData(void **data_ptr);
  int32_t unlockAhwbData();

 private:
  uint64_t ahwb_type_ = 0;
  void *data_ = 0;
  AHardwareBuffer *abuffer_ = nullptr;
  NeuronMemory *memory_ = nullptr;
  uint32_t size_ = 0;

  AhwbNeuronMemoryWrapper &operator=(const AhwbNeuronMemoryWrapper &) = delete;
  AhwbNeuronMemoryWrapper(const AhwbNeuronMemoryWrapper &) = delete;
};

class NeuronAllocator {
 public:
  NeuronAllocator() { LOG(INFO) << "Neuron Allocator created"; }
  void *GetBuffer(size_t n) {
    n *= 2;

    mEnable = true;
    if (!allocatorMapper.count(n)) {
      LOG(INFO) << "Neuron Allocator create Allocator :" << n;
      allocatorMapper[n] = allocators.size();
      allocators.emplace_back(n, 300);
    } else {
      LOG(INFO) << "Neuron Allocator reuse Allocator :" << n;
    }
    Allocator &aloct = allocators[allocatorMapper.at(n)];

    auto [base, offset, memory] = aloct.GetBuffer();
    // LOG(INFO) << "[data] base: " << base << " offset :" << offset;
    // For MLPerf, they only needs the address
    // but for neuron, setIO from neuron Memory needs offset,
    void *addr = (char *)base + offset;
    memoryMapper[addr] = {base, offset, memory};
    return addr;
  }
  std::tuple<void *, size_t, NeuronMemory *> GetCachedMemory(void *addr) {
    if (memoryMapper.count(addr)) {
      return memoryMapper[addr];
    }
    return {nullptr, 0, nullptr};
  }

  std::vector<std::pair<size_t, std::vector<NeuronMemory *>>>
  GetAllocatedNeuronMemory() {
    std::vector<std::pair<size_t, std::vector<NeuronMemory *>>> res;
    for (auto &aloct : allocators) {
      auto [size, memorys] = aloct.GetAllocatedNeuronMemory();
      res.emplace_back(size, memorys);
    }
    return res;
  }

  void SetSizeHint(int size) { mSizeHint = size; }

  bool IsEnable() const { return mEnable; }

 private:
  std::unordered_map<size_t, size_t> allocatorMapper;
  std::unordered_map<void *, std::tuple<void *, size_t, NeuronMemory *>>
      memoryMapper;
  std::vector<Allocator> allocators{};
  bool mEnable = false;
  int mSizeHint = 0;
};

struct ParallelExecution {
  ParallelExecution(NeuronExecution* exec):exec(exec){}
  NeuronExecution* exec;
  std::unordered_map<int, std::vector<std::future<void*>>> seq_input_futures;
};

struct AdapterBackendData {
  const char *name = "Adapter";
  const char *vendor = "Mediatek";
  NeuronModel *model{nullptr};
  NeuronCompilation *compilation{nullptr};
  NeuronExecution *execution{nullptr};
  std::vector<AhwbNeuronMemoryWrapper> inputMemoryWrappers;
  std::vector<AhwbNeuronMemoryWrapper> outputMemoryWrappers;
  std::vector<neuron_data_t> inputTypes{};
  std::vector<neuron_data_t> outputTypes{};
  std::vector<uint32_t> inputSizes{};
  std::vector<uint32_t> outputSizes{};
  int32_t input_nums = 0;
  int32_t output_nums = 0;
  int32_t shards_num = 1;
  uint32_t real_batch_size = 1;
  bool useNeuronBackend = false;
  bool useSuppress = false;
  bool use_throughput_mode = false;
  mtk::performance::PerformanceLocker locker;
  //--------------Allocator----------------
  NeuronAllocator neuronAllocator;
  bool hasPreset = false;
  //---------------------------------------
  std::function<void(AdapterBackendData *, int, void *)> paddingFunc = nullptr;
  //-------Input data thread pool----------
  int thread_pool_size = 4;
  //------------- Preference ------------------
  NeuronAdapterPreferenceCode preference = NEURON_PREFER_TURBO_BOOST;
};

bool need_neuron_backend(const char *model_path);

bool create_neuron_backend(neuron_backend_ptr_t backend_ptr,
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

#endif
