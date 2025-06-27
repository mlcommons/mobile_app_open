/* Copyright (c) 2020-2025 Qualcomm Innovation Center, Inc. All rights reserved.

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

void doCopy(void* dest, size_t n) {
  printf("Verifying %p\n", dest);
  for (size_t x = 0; x < n; x++) {
    ((uint8_t*)(dest))[x] = 1;
  }
}

int main() {
  void* a1 = ChunkAllocator::GetBuffer(100, 3);
  ChunkAllocator::DumpState();
  doCopy(a1, 100);
  void* a2 = ChunkAllocator::GetBuffer(100, 3);
  ChunkAllocator::DumpState();
  doCopy(a2, 100);
  void* a3 = ChunkAllocator::GetBuffer(100, 3);
  ChunkAllocator::DumpState();
  doCopy(a3, 100);
  void* a4 = ChunkAllocator::GetBuffer(100, 3);
  doCopy(a4, 100);
  void* a5 = ChunkAllocator::GetBuffer(100, 3);
  doCopy(a5, 100);
  ChunkAllocator::DumpState();

  void* b1 = ChunkAllocator::GetBuffer(150, 4);
  doCopy(b1, 150);
  void* b2 = ChunkAllocator::GetBuffer(150, 4);
  doCopy(b2, 150);
  ChunkAllocator::DumpState();
  void* b3 = ChunkAllocator::GetBuffer(150, 4);
  doCopy(b3, 150);
  void* b4 = ChunkAllocator::GetBuffer(150, 4);
  doCopy(b4, 150);
  void* b5 = ChunkAllocator::GetBuffer(150, 4);
  doCopy(b5, 150);
  ChunkAllocator::DumpState();

  ChunkAllocator::ReleaseBuffer(a1);
  ChunkAllocator::ReleaseBuffer(a2);
  ChunkAllocator::DumpState();
  ChunkAllocator::ReleaseBuffer(a3);
  ChunkAllocator::ReleaseBuffer(a4);
  ChunkAllocator::ReleaseBuffer(a5);
  ChunkAllocator::DumpState();
  ChunkAllocator::ReleaseBuffer(b1);
  ChunkAllocator::ReleaseBuffer(b2);
  ChunkAllocator::DumpState();
  ChunkAllocator::ReleaseBuffer(b3);
  ChunkAllocator::ReleaseBuffer(b4);
  ChunkAllocator::ReleaseBuffer(b5);
  ChunkAllocator::DumpState();
}
