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
 * @file neuron_memory.cc
 */

#include "neuron_memory.h"

#include <fcntl.h>
#include <linux/dma-heap.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "NeuronAdapterShim.h"
#include "neuron_utils.h"

namespace {
int32_t CreateNeuronMemoryWithDMA(uint32_t byte_size, uint32_t offset,
                                  int dma_buffer_fd, NeuronMemory** memory) {
  auto ret = NeuronMemory_createFromFd(byte_size, PROT_READ | PROT_WRITE,
                                       dma_buffer_fd, offset, memory);
  if (ret != NEURON_NO_ERROR) {
    LOG(ERROR) << "Failed to create neuron memory with error: " << ret;
  }
  return ret;
}

int32_t CreateNeuronMemoryWithAHWB(uint32_t byte_size, uint64_t ahwb_type,
                                   NeuronMemory** memory,
                                   AHardwareBuffer** abuffer) {
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

int DmabufSetName(unsigned int dmabuf_fd, const std::string& name) {
  /*
   * Truncate the name here to avoid failure if the length exceeds the limit.
   * length() does not count the '\0' character at the end of the string,
   * but the kernel does, ioctl() would also fail if len == DMA_BUF_NAME_LEN.
   * So we limit the maximum length of the name to 'DMA_BUF_NAME_LEN - 1'.
   */
  const std::string truncated_name = name.substr(0, DMA_BUF_NAME_LEN - 1);
  return TEMP_FAILURE_RETRY(
      ioctl(dmabuf_fd, DMA_BUF_SET_NAME_B, truncated_name.c_str()));
}

inline const char* GetHeapNameByType(BufferType type) {
  switch (type) {
    case UNCACHED: {
      return "mtk_mm-uncached";
    }
    case CACHEABLE: {
      return "mtk_mm";
    }
    case COHERENT: {
      return "mtk_mm-coherent";
    }
    default: {
      LOG(ERROR) << "Failed to get dma heap name of unknow type: " << type;
    }
  }
  return "unknown";
}

}  // namespace

AhwbNeuronMemoryWrapper::~AhwbNeuronMemoryWrapper() {
  if (data_) unlockAhwbData();
  if (memory_) NeuronMemory_free(memory_);
  memory_ = nullptr;
  if (abuffer_) AHardwareBuffer_release(abuffer_);
}

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

int32_t AhwbNeuronMemoryWrapper::lockAhwbData(void** out_data_ptr) {
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
  if (AHardwareBuffer_lock(abuffer_, ahwb_type_, -1, nullptr, out_data_ptr) !=
      0) {
    LOG(ERROR) << "AHWB lock fail";
    return NEURON_BAD_STATE;
  }
  data_ = *out_data_ptr;
  return NEURON_NO_ERROR;
}

void* AhwbNeuronMemoryWrapper::data() {
  void* temp_ptr = nullptr;
  if (lockAhwbData(&temp_ptr) != NEURON_NO_ERROR) {
    LOG(ERROR) << "Lock and aquire data failed";
    return nullptr;
  }
  return temp_ptr;
}

int32_t DmaNeuronMemoryWrapper::InitNeuronMemory() {
  if (!buffer_.Available()) {
    LOG(ERROR)
        << "Failed to initialized NeuronMemory with an invalid dma buffer.";
    return NEURON_BAD_STATE;
  }
  return CreateNeuronMemoryWithDMA(size_, 0, buffer_.fd(), &memory_);
}

void* DmaNeuronMemoryWrapper::data() { return buffer_.data(); }

DmaBufferAllocatorWrapper::~DmaBufferAllocatorWrapper() {
  for (auto& [_, fd] : opened_heap_fds_) {
    if (fd >= 0) {
      close(fd);
    }
  }
}

int DmaBufferAllocatorWrapper::GetHeadFd(BufferType type) {
  if (auto it = opened_heap_fds_.find(type); it != opened_heap_fds_.end()) {
    return it->second;
  }
  int heap_fd = -1;
  switch (type) {
    case UNCACHED:
    case CACHEABLE:
    case COHERENT: {
      const std::string heap_path =
          "/dev/dma_heap/" + std::string(GetHeapNameByType(type));
      Tracer trace(heap_path);
      heap_fd =
          TEMP_FAILURE_RETRY(open(heap_path.c_str(), O_RDONLY | O_CLOEXEC));
    }
  }
  if (heap_fd < 0) {
    LOG(ERROR) << "Failed to open dma Buffer heap of type: " << type;
  }
  return opened_heap_fds_[type] = heap_fd;
}

bool DmaBufferAllocatorWrapper::Allocate(BufferType type, uint32_t size,
                                         std::unique_ptr<DmaBuffer>& buffer) {
  if (buffer != nullptr && buffer->Available()) {
    LOG(ERROR) << "Can't allocate dma buffer on an available buffer.";
    return false;
  }
  if (!Available(type)) {
    LOG(ERROR) << "Can't allocate: [ " << type << " ] type dma buffer";
    return false;
  }
  int heap_fd = GetHeadFd(type);
  struct dma_heap_allocation_data heapInfo = {
      .len = size,
      .fd_flags = O_RDWR | O_CLOEXEC,
  };
  if (TEMP_FAILURE_RETRY(ioctl(heap_fd, DMA_HEAP_IOCTL_ALLOC, &heapInfo)) < 0) {
    LOG(ERROR) << "Failed to allocate DMA buffer";
    return false;
  }
  if (heapInfo.fd >= 0) {
    const char* heap_name = GetHeapNameByType(type);
    if (DmabufSetName(heapInfo.fd, heap_name) != 0) {
      LOG(WARNING) << "Unable to name DMA buffer for: " << heap_name;
    }
  }
  buffer = std::make_unique<DmaBuffer>(type, size, heapInfo.fd);
  if (!buffer->Available()) {
    LOG(ERROR) << "The allocated dma buffer is not available.";
    buffer.reset();
    return false;
  }
  Tracer trace(std::string() + GetHeapNameByType(type) +
               "_size:" + std::to_string(size));
  LOG(INFO) << "Allocated DMA buffer of type: " << type
            << " size: " << buffer->size() << " fd: " << buffer->fd();
  return true;
}

bool DoSync(bool start, unsigned int dmabuf_fd, SyncType sync_type) {
  struct dma_buf_sync sync = {
      .flags = (start ? DMA_BUF_SYNC_START : DMA_BUF_SYNC_END) |
               static_cast<uint64_t>(sync_type),
  };
  return TEMP_FAILURE_RETRY(ioctl(dmabuf_fd, DMA_BUF_IOCTL_SYNC, &sync)) == 0;
}

bool DmaBufferAllocatorWrapper::CpuSyncStart(const DmaBuffer& buffer,
                                             SyncType sync_type) {
  if (buffer.type() != CACHEABLE) return true;

  if (!DoSync(true, buffer.fd(), sync_type)) {
    LOG(ERROR) << "CpuSyncStart() failure";
    return false;
  }
  return true;
}

bool DmaBufferAllocatorWrapper::CpuSyncEnd(const DmaBuffer& buffer,
                                           SyncType sync_type) {
  if (buffer.type() != CACHEABLE) return true;

  if (!DoSync(false, buffer.fd(), sync_type)) {
    LOG(ERROR) << "CpuSyncEnd() failure";
    return false;
  }
  return true;
}

DmaBuffer::~DmaBuffer() {
  if (data_ != nullptr) munmap(data_, size_);
  if (fd_ >= 0) close(fd_);
}

void* DmaBuffer::data() {
  if (fd_ < 0) return data_ = nullptr;
  if (data_ != nullptr) return data_;

  LOG(INFO) << "Try to open Dma Buffer fd: " << fd_;
  auto mapped_addr =
      ::mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
  if (mapped_addr == MAP_FAILED) {
    LOG(ERROR) << "Failed to mmap the the given Dma Buffer fd: " << fd_;
    mapped_addr = nullptr;
  }
  return data_ = mapped_addr;
}
