/* Copyright (c) 2020-2021 Qualcomm Innovation Center, Inc. All rights reserved.

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

#include "allocator.h"

// Map of allocation size to Chunk allocator instance
std::map<size_t, ChunkAllocator>& getAllocator() {
  static std::map<size_t, ChunkAllocator> allocator;
  return allocator;
}

std::map<void*, ChunkAllocator::Block*> ChunkAllocator::Block::block_map_;

void* ChunkAllocator::GetBuffer(size_t n, size_t chunks_per_block) {
  // See if allocator for this allocation size exists
  if (!getAllocator().count(n)) {
    getAllocator().emplace(
        std::make_pair(n, ChunkAllocator(n, chunks_per_block)));
  }
  return getAllocator().at(n).GetChunk();
}

void ChunkAllocator::ReleaseBuffer(void* p) {
  Block* block = Block::block_map_[p];
  getAllocator().at(block->GetChunkSize()).ReleaseChunk(block, p);
}

void* ChunkAllocator::GetBatchPtr(void* p) {
  return Block::block_map_[p]->ptr_;
}

void ChunkAllocator::DumpState() {
  printf("================ DumpState ==============\n");
  for (auto it : getAllocator()) {
    printf("ChunkAllocator:\n");
    it.second.DumpAllocatorState();
  }
}

RpcMem& ChunkAllocator::getRpcMem() {
  static RpcMem rpcmem;
  return rpcmem;
}
