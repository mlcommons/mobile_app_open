/* Copyright (c) 2020-2024 Qualcomm Innovation Center, Inc. All rights reserved.

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

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <list>
#include <map>
#include <memory>

#include "rpcmem.h"
#include "tensorflow/core/platform/logging.h"

// This allocator assumes all allocations and frees are done in order
class ChunkAllocator {
 public:
  static void *GetBuffer(size_t n, size_t chunks_per_block);

  static void ReleaseBuffer(void *p);

  static void *GetBatchPtr(void *p);

  static int GetSize(int n);

  static uint64_t GetOffset(void *p);

  static void DumpState();

  static RpcMem &getRpcMem();

 private:
  ChunkAllocator();

  ChunkAllocator(size_t chunk_size, size_t chunks_per_block)
      : chunk_size_(chunk_size), chunks_per_block_(chunks_per_block) {}

  class Block {
   public:
    Block(size_t chunk_size, int32_t num_chunks)
        : num_chunks_(num_chunks),
          free_chunks_(num_chunks),
          chunk_size_(chunk_size) {
      ptr_ = getRpcMem().Alloc(chunk_size * num_chunks);
      block_map_[ptr_] = this;
    }

    void *GetChunk() {
      if (free_chunks_) {
        free_chunks_--;
        uint8_t *p = (uint8_t *)ptr_;
        return (void *)&p[(num_chunks_ - free_chunks_ - 1) * chunk_size_];
      }
      return nullptr;
    }

    void ReleaseChunk(void *p) {
      free_chunks_++;
      if (free_chunks_ == num_chunks_) {
        getRpcMem().Free(ptr_);
        ptr_ = nullptr;
      }
      Block::block_map_.erase(p);
    }

    uint64_t GetOffset(void *p) { return (uint8_t *)p - (uint8_t *)ptr_; }

    bool IsFull() { return free_chunks_ == 0; }
    bool IsEmpty() { return free_chunks_ == num_chunks_; }

    uint32_t GetChunkSize() { return chunk_size_; }
    void DumpState() {
      printf("    Block (%p):\n", this);
      printf("      NumChunks %u\n", num_chunks_);
      printf("      FreeChunks %u\n", free_chunks_);
    }

    ~Block() {
      if (ptr_ != nullptr) getRpcMem().Free(ptr_);
    }

    // Map allocated pointers to their block
    static std::map<void *, Block *> block_map_;

    void *ptr_ = nullptr;

   private:
    uint32_t num_chunks_;
    uint32_t free_chunks_;
    uint32_t chunk_size_;
  };

  void *GetChunk() {
    if (block_list_.empty() || block_list_.back()->IsFull()) {
      Block *block = new Block(chunk_size_, chunks_per_block_);
      block_list_.push_back(block);
    }
    Block *block = block_list_.back();
    void *p = block_list_.back()->GetChunk();
    Block::block_map_[p] = block;
    return p;
  }

  uint64_t GetOffset(Block *block, void *p) { return block->GetOffset(p); }

  void ReleaseChunk(Block *block, void *p) {
    // Block::ReleaseChunk updates Block::block_map_
    block->ReleaseChunk(p);
    if (block->IsEmpty()) {
      block_list_.remove(block);
      delete block;
    }
  }

  bool IsChunkEmpty() { return (block_list_.empty()); }

  void DumpAllocatorState() {
    printf("  ChunkSize %lu\n", chunk_size_);
    printf("  ChunksPerBlock %lu\n", chunks_per_block_);
    printf("  Blocks:\n");
    for (auto it : block_list_) {
      it->DumpState();
    }
  }

  // List of all allocated blocks for this Chunk allocator instance
  // FIXME this should be a unique_map
  std::list<Block *> block_list_;
  const size_t chunk_size_;
  const size_t chunks_per_block_;
};

template <class T>
class Allocator {
 public:
  typedef T value_type;
  static bool useIonBuffer;

  Allocator() = default;

  template <class U>
  constexpr Allocator(const Allocator<U> &) noexcept {}

  [[nodiscard]] T *allocate(std::size_t n) {
    T *p;
    if (useIonBuffer) {
      p = static_cast<T *>(ChunkAllocator::getRpcMem().Alloc(n * sizeof(T)));
    } else {
      p = static_cast<T *>(std::malloc(n * sizeof(T)));
    }
    return p;
  }

  void deallocate(T *p, std::size_t n) noexcept {
    if (useIonBuffer) {
      ChunkAllocator::getRpcMem().Free(p);
    } else {
      std::free(p);
    }
  }

  static void useIonAllocator() {
    useIonBuffer = true;
    // LOG(INFO) << "Using Ion Allocator";
  }

  static void useDefaultAllocator() {
    useIonBuffer = false;
    // LOG(INFO) << "Using Default Allocator";
  }
};

template <class T>
bool Allocator<T>::useIonBuffer = true;

static void *get_buffer(size_t n, int chunkSize = 3) {
  // LOG(INFO) << chunkSize;
  void *p = ChunkAllocator::GetBuffer(n, chunkSize);
  // LOG(INFO) << "QTI backend SNPE allocator " << n << " bytes at " << p << "
  // with chunk size: " << chunkSize;
  return p;
}

static uint64_t get_offset(void *p) { return ChunkAllocator::GetOffset(p); }

static void release_buffer(void *p) {
  // LOG(INFO) << "QTI backend SNPE free " << p;
  ChunkAllocator::ReleaseBuffer(p);
}

static bool get_rpc_status() {
  return ChunkAllocator::getRpcMem().getRpcStatus();
}

#endif
