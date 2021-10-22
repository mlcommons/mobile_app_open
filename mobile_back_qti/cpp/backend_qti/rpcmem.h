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

#include <dlfcn.h>
#include <stdint.h>

#include <cstdlib>
#include <type_traits>

#ifndef RPCMEM_H
#define RPCMEM_H

using RpcMemAllocPtr = std::add_pointer<void *(int, uint32_t, int)>::type;
using RpcMemFreePtr = std::add_pointer<void(void *po)>::type;

class RpcMem {
 public:
  RpcMem();
  ~RpcMem();
  void *Alloc(int heapid, uint32_t flags, int nbytes);
  void *Alloc(int nbytes);
  void Free(void *data);

 private:
  void *libHandle_;
  bool isSuccess_;
  RpcMemAllocPtr rpcmemAlloc_{nullptr};
  RpcMemFreePtr rpcmemFree_{nullptr};
};

#endif  // RPCMEM_H
