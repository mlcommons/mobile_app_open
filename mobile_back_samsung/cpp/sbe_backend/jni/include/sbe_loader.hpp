/* Copyright 2020-2022 Samsung Electronics Co. LTD  All Rights Reserved.

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
#ifndef SBE_LOADER_H_
#define SBE_LOADER_H_

/**
 * @file sbe_loader.hpp
 * @brief dynamic loader for samsung exynos libraries
 * @date 2022-01-04
 * @author soobong Huh (soobong.huh@samsung.com)
 */

#include <dlfcn.h>
#include "type.h"
#include "sbe_utils.hpp"

namespace sbe {
std::string sbe_core_libs[CORE_MAX] = {
  "libsbe1200_core.so",
  "libsbe2100_core.so",
  "libsbe2200_core.so",
};

void* load_symbol(void* dl_handle, const char* name) {
  auto func_pt = dlsym(dl_handle, name);
  if(func_pt==nullptr) {
    MLOGD("dlopen fail. symbol[%s]", name);
  }
  return func_pt;
}

#define link_symbol(dl_handle, symbol_name) \
  reinterpret_cast<symbol_name##_t>(    \
  load_symbol(dl_handle, #symbol_name))
}

#endif
