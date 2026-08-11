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
/* MediaTek Inc. (C) 2025. All rights reserved.
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
 * @file neuron_memory.h
 */

#pragma once

#include <linux/dma-buf.h>

#include <string>
#include <unordered_map>
#include <utility>

#include "NeuronAdapter.h"

class NeuronMemoryWrapper {
 public:
  NeuronMemoryWrapper(uint32_t byte_size)
      : size_(byte_size), memory_(nullptr) {}
  virtual ~NeuronMemoryWrapper() {
    if (memory_) NeuronMemory_free(memory_);
  }
  NeuronMemoryWrapper(NeuronMemoryWrapper &&other) {
    size_ = other.size_;
    memory_ = other.memory_;
    other.memory_ = nullptr;
    other.size_ = 0;
  }
  NeuronMemoryWrapper &operator=(NeuronMemoryWrapper &&other) {
    if (this != &other) {
      NeuronMemoryWrapper::~NeuronMemoryWrapper();
      size_ = other.size_;
      memory_ = other.memory_;
      other.memory_ = nullptr;
      other.size_ = 0;
    }
    return *this;
  }

  virtual void *data() = 0;
  virtual int32_t InitNeuronMemory() = 0;

  NeuronMemory *memory() { return memory_; }
  uint32_t size() { return size_; }
  bool Available() { return memory_ != nullptr; }

 protected:
  NeuronMemory *memory_ = nullptr;
  uint32_t size_ = 0;

 private:
  NeuronMemoryWrapper &operator=(const NeuronMemoryWrapper &) = delete;
  NeuronMemoryWrapper(const NeuronMemoryWrapper &) = delete;
};

typedef enum {
  kSyncRead = DMA_BUF_SYNC_READ,
  kSyncWrite = DMA_BUF_SYNC_WRITE,
  kSyncReadWrite = DMA_BUF_SYNC_RW,
} SyncType;

enum BufferType {
  UNKNOWN,
  UNCACHED,
  CACHEABLE,
  COHERENT,
};

class BufferAllocator;
class DmaBuffer;
class DmaBufferAllocatorWrapper {
 public:
  DmaBufferAllocatorWrapper() : opened_heap_fds_() {}
  ~DmaBufferAllocatorWrapper();
  DmaBufferAllocatorWrapper(DmaBufferAllocatorWrapper &&other) {
    *this = std::move(other);
  };
  DmaBufferAllocatorWrapper &operator=(DmaBufferAllocatorWrapper &&other) {
    if (this != &other) {
      opened_heap_fds_ = std::move(other.opened_heap_fds_);
    }
    return *this;
  }

  bool Available(BufferType type) { return GetHeadFd(type) >= 0; }
  int GetHeadFd(BufferType type);
  bool Allocate(BufferType type, uint32_t size,
                std::unique_ptr<DmaBuffer> &buffer);
  bool CpuSyncStart(const DmaBuffer &buffer, SyncType sync_type);
  bool CpuSyncEnd(const DmaBuffer &buffer, SyncType sync_type);

 private:
  std::unordered_map<BufferType, int> opened_heap_fds_;

  DmaBufferAllocatorWrapper &operator=(const DmaBufferAllocatorWrapper &) =
      delete;
  DmaBufferAllocatorWrapper(const DmaBufferAllocatorWrapper &) = delete;
};

class DmaBuffer {
 public:
  DmaBuffer(BufferType type, uint32_t byte_size, int fd)
      : type_(type), size_(byte_size), fd_(fd) {}
  ~DmaBuffer();
  DmaBuffer(DmaBuffer &&other) {
    size_ = other.size_;
    fd_ = other.fd_;
    data_ = other.data_;
    type_ = other.type_;
    other.size_ = 0;
    other.fd_ = -1;
    other.data_ = nullptr;
    other.type_ = UNKNOWN;
  }
  DmaBuffer &operator=(DmaBuffer &&other) {
    if (this != &other) {
      DmaBuffer::~DmaBuffer();
      size_ = other.size_;
      fd_ = other.fd_;
      data_ = other.data_;
      type_ = other.type_;
      other.size_ = 0;
      other.fd_ = -1;
      other.data_ = nullptr;
      other.type_ = UNKNOWN;
    }
    return *this;
  }

  void *data();
  uint32_t size() const { return size_; }
  int fd() const { return fd_; }
  BufferType type() const { return type_; }
  bool Available() { return data() != nullptr; }

 private:
  int fd_ = -1;
  uint32_t size_ = 0;
  void *data_ = nullptr;
  BufferType type_ = UNKNOWN;

  DmaBuffer &operator=(const DmaBuffer &) = delete;
  DmaBuffer(const DmaBuffer &) = delete;
};

class DmaNeuronMemoryWrapper : public NeuronMemoryWrapper {
 public:
  explicit DmaNeuronMemoryWrapper(DmaBuffer &&buffer)
      : NeuronMemoryWrapper(buffer.size()), buffer_(std::move(buffer)) {}
  int32_t InitNeuronMemory() override;
  DmaNeuronMemoryWrapper(DmaNeuronMemoryWrapper &&other)
      : NeuronMemoryWrapper(std::move(other)),
        buffer_(std::move(other.buffer_)) {}
  void *data() override;

 private:
  DmaBuffer buffer_;
  DmaNeuronMemoryWrapper &operator=(const DmaNeuronMemoryWrapper &) = delete;
  DmaNeuronMemoryWrapper(const DmaNeuronMemoryWrapper &) = delete;
};

class AhwbNeuronMemoryWrapper : public NeuronMemoryWrapper {
 public:
  explicit AhwbNeuronMemoryWrapper(
      uint32_t byte_size,
      uint64_t ahwb_type = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN |
                           AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN)
      : NeuronMemoryWrapper(byte_size), ahwb_type_(ahwb_type) {}
  ~AhwbNeuronMemoryWrapper() override;
  AhwbNeuronMemoryWrapper(AhwbNeuronMemoryWrapper &&other)
      : NeuronMemoryWrapper(std::move(other)) {
    this->data_ = other.data_;
    this->ahwb_type_ = other.ahwb_type_;
    this->abuffer_ = other.abuffer_;
    other.data_ = nullptr;
    other.abuffer_ = nullptr;
  };
  AhwbNeuronMemoryWrapper &operator=(AhwbNeuronMemoryWrapper &&other) {
    if (this != &other) {
      AhwbNeuronMemoryWrapper::~AhwbNeuronMemoryWrapper();
      this->data_ = other.data_;
      this->ahwb_type_ = other.ahwb_type_;
      this->abuffer_ = other.abuffer_;
      other.data_ = nullptr;
      other.abuffer_ = nullptr;
    }
    return *this;
  }

  int32_t InitNeuronMemory() override;

  void *data() override;
  int32_t lockAhwbData(void **data_ptr);
  int32_t unlockAhwbData();

 private:
  uint64_t ahwb_type_ = 0;
  void *data_ = nullptr;
  AHardwareBuffer *abuffer_ = nullptr;

  AhwbNeuronMemoryWrapper &operator=(const AhwbNeuronMemoryWrapper &) = delete;
  AhwbNeuronMemoryWrapper(const AhwbNeuronMemoryWrapper &) = delete;
};
