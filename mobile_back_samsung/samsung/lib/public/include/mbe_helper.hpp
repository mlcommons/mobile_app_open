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
#ifndef MBE_HELPER_H_
#define MBE_HELPER_H_

/**
 * @file mbe_helper.hpp
 * @brief helper class of samsung backend core for samsung exynos
 * @date 2022-01-04
 * @author soobong Huh (soobong.huh@samsung.com)
 */

#include <dlfcn.h>
#include <stdint.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "mbe_config.hpp"
#include "mbe_utils.hpp"

namespace mbe {
static int core_id;
#define DECO(x) #x
#define MAJOR DECO(2)
#define MINOR DECO(4)
#define PATCH DECO(3)
#define VERSION(a, b, c) a "." b "." c

class core_ctrl {
 public:
  static int support_mbe(const char *, const char *);
  static const char *get_benchmark_config(int);
  static int get_core_id();
};
}  // namespace mbe

#endif
