/* Copyright 2021 The MLPerf Authors. All Rights Reserved.

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

#include <memory>

static void* GetBuffer_(size_t n) { return std::malloc(n); }
static void ReleaseBuffer_(void* p) { std::free(p); }

namespace mlperf {
namespace mobile {

AllocatorMgr::GetBufferFn AllocatorMgr::get_buffer_ = GetBuffer_;
AllocatorMgr::ReleaseBufferFn AllocatorMgr::release_buffer_ = ReleaseBuffer_;

void AllocatorMgr::UseBackendAllocator(GetBufferFn& get_buffer,
                                       ReleaseBufferFn& release_buffer) {
  get_buffer_ = get_buffer;
  release_buffer_ = release_buffer;
}

void AllocatorMgr::UseDefaultAllocator() {
  get_buffer_ = GetBuffer_;
  release_buffer_ = ReleaseBuffer_;
}

}  // namespace mobile
}  // namespace mlperf
