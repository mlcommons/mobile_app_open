/* Copyright 2020-2023 Samsung Electronics Co. LTD  All Rights Reserved.

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
#ifndef MBE_LOADER_H_
#define MBE_LOADER_H_

/**
 * @file mbe_loader.hpp
 * @brief dynamic loader for exynos libraries
 * @date 2022-01-04
 *       2022-08-30 (update module name)
 * @author soobong Huh (soobong.huh@samsung.com)
 */

#include <dlfcn.h>

#include "mbe_utils.hpp"
#include "type.h"

namespace mbe {
std::string mbe_core_libs[CORE_MAX] = {
    "libmbe1200_core.so",
    "libmbe2100_core.so",
    "libmbe2200_core.so",
    "libmbe2300_core.so",
};

void* load_symbol(void* dl_handle, const char* name) {
  auto func_pt = dlsym(dl_handle, name);
  if (func_pt == nullptr) {
    MLOGE("dlopen fail. symbol[%s]", name);
  }
  return func_pt;
}

#define link_symbol(dl_handle, symbol_name) \
  reinterpret_cast<symbol_name##_t>(load_symbol(dl_handle, #symbol_name))
}  // namespace mbe

#endif
