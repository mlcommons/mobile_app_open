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
#ifndef MLPERF_DATASETS_ALLOCATOR_H_
#define MLPERF_DATASETS_ALLOCATOR_H_

#include <cstdlib>
#include <new>

#include "cpp/utils.h"

namespace mlperf {
namespace mobile {

class AllocatorMgr {
 public:
  using GetBufferFn = std::add_pointer<void *(size_t)>::type;
  using ReleaseBufferFn = std::add_pointer<void(void *)>::type;

  static void UseBackendAllocator(GetBufferFn &get_buffer,
                                  ReleaseBufferFn &release_buffer);
  static void UseDefaultAllocator();

  static void *GetBuffer(size_t n) { return get_buffer_(n); }
  static void ReleaseBuffer(void *p) { release_buffer_(p); }

 private:
  AllocatorMgr();
  ~AllocatorMgr();

  static GetBufferFn get_buffer_;
  static ReleaseBufferFn release_buffer_;
};

template <class T>
struct BackendAllocator {
  typedef T value_type;

  // BackendAllocator() { allocatorFunctions_ = GetBackendAllocatorFunctions();
  // }
  BackendAllocator() = default;

  template <class U>
  constexpr BackendAllocator(const BackendAllocator<U> &) noexcept {}

  [[nodiscard]] T *allocate(std::size_t n) {
    if (auto p = static_cast<T *>(AllocatorMgr::GetBuffer(n * sizeof(T)))) {
      return p;
    }

    LOG(FATAL) << "Failed to get buffer from backend";
  }

  void deallocate(T *p, std::size_t n) noexcept {
    AllocatorMgr::ReleaseBuffer(p);
  }
};

template <class T, class U>
bool operator==(const BackendAllocator<T> &, const BackendAllocator<U> &) {
  return true;
}
template <class T, class U>
bool operator!=(const BackendAllocator<T> &, const BackendAllocator<U> &) {
  return false;
}

}  // namespace mobile
}  // namespace mlperf

#endif
