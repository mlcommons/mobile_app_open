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

#include "rpcmem.h"
#include "cpuctrl.h"

#include "tensorflow/core/platform/logging.h"

RpcMem::RpcMem() {
  if (CpuCtrl::getSocId() != SDM865) {
    libHandle_ = dlopen("libcdsprpc.so", RTLD_NOW);
  } else {
    libHandle_ = nullptr;
  }

  if (libHandle_ == nullptr) {
    LOG(ERROR) << "Can't open rpc lib";
    isSuccess_ = false;
  } else {
    rpcmemAlloc_ =
        reinterpret_cast<RpcMemAllocPtr>(dlsym(libHandle_, "rpcmem_alloc"));
    rpcmemFree_ =
        reinterpret_cast<RpcMemFreePtr>(dlsym(libHandle_, "rpcmem_free"));

    if (rpcmemAlloc_ && rpcmemFree_) {
      isSuccess_ = true;
    } else {
      isSuccess_ = false;
      LOG(ERROR) << "Unable to dlsym rpcmem functions";
    }
  }
}

RpcMem::~RpcMem() {
  isSuccess_ = false;
  dlclose(libHandle_);
}

void *RpcMem::Alloc(int id, uint32_t flags, int size) {
  if (isSuccess_) {
    return rpcmemAlloc_(id, flags, size);
  } else {
    return std::malloc(size);
  }
}

void *RpcMem::Alloc(int size) { return Alloc(25, 1, size); }

void RpcMem::Free(void *data) {
  if (isSuccess_) {
    return rpcmemFree_(data);
  } else {
    return std::free(data);
  }
}
