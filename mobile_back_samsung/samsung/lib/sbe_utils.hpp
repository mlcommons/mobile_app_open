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
#ifndef SBE_UTILS_H_
#define SBE_UTILS_H_

/**
 * @file sbe_utils.hpp
 * @brief utility for samsung exynos backend
 * @date 2022-01-04
 * @author soobong Huh (soobong.huh@samsung.com)
 */

#include <android/log.h>

namespace sbe {
enum DEVICE_ID {
  CORE_INVALID = -1,
  CORE_2100 = 0,
  CORE_1200,
  CORE_2200,
  CORE_MAX
};

#ifndef LOG_TAG
#define LOG_TAG "sbe"
#endif

#define LOG_ENABLE
#undef LOG_ENABLE

#define __SHARP_X(x) #x
#define __STR(x) __SHARP_X(x)
#define _MLOG(_loglevel, fmt, ...)                                          \
  __android_log_print(_loglevel, "MLPerf",                                  \
                      "[Backend][" LOG_TAG "] %s:" __STR(__LINE__) ": " fmt \
                                                                   "\n",    \
                      __FUNCTION__, ##__VA_ARGS__)

#ifdef LOG_ENABLE
#define MLOGV(fmt, ...) _MLOG(ANDROID_LOG_VERBOSE, fmt, ##__VA_ARGS__)
#define MLOGD(fmt, ...) _MLOG(ANDROID_LOG_DEBUG, fmt, ##__VA_ARGS__)
#define MLOGE(fmt, ...) _MLOG(ANDROID_LOG_ERROR, fmt, ##__VA_ARGS__)
#else
#define MLOGV(fmt, ...) _MLOG(ANDROID_LOG_VERBOSE, fmt, ##__VA_ARGS__)
#define MLOGD(fmt, ...)
#define MLOGE(fmt, ...)
#endif
}  // namespace sbe
#endif