/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** @file log.h
    @brief NPU log header
*/
#ifndef COMMON_LOG_H_
#define COMMON_LOG_H_

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "EDEN"
#define EDEN_VERSION "v1.6.6-4"
#define VERSION_MAJOR 1
#define VERSION_MINOR 6
#define VERSION_BUILD 6

enum _eden_log_flags {
  EDEN_LOG_FORCE = 0,
  EDEN_LOG_NN = 1,
  EDEN_LOG_RT = 2,
  EDEN_LOG_UD = 3,
  EDEN_LOG_LINK = 4,
  EDEN_LOG_EMA = 5,
  EDEN_LOG_GTEST = 6,
  EDEN_LOG_HIDL = 7,
  EDEN_LOG_RT_STUB = 8,
  EDEN_LOG_DRIVER = 9,
  EDEN_LOG_CL = 10,
  EDEN_LOG_TOOL = 11,
  EDEN_LOG_MAX_FLAG
};

#define EDEN_FORCE (1 << EDEN_LOG_FORCE)
#define EDEN_NN (1 << EDEN_LOG_NN)
#define EDEN_RT (1 << EDEN_LOG_RT)
#define EDEN_UD (1 << EDEN_LOG_UD)
#define EDEN_LINK (1 << EDEN_LOG_LINK)
#define EDEN_EMA (1 << EDEN_LOG_EMA)
#define EDEN_GTEST (1 << EDEN_LOG_GTEST)
#define EDEN_HIDL (1 << EDEN_LOG_HIDL)
#define EDEN_RT_STUB (1 << EDEN_LOG_RT_STUB)
#define EDEN_DRIVER (1 << EDEN_LOG_DRIVER)
#define EDEN_CL (1 << EDEN_LOG_CL)
#define EDEN_TOOL (1 << EDEN_LOG_TOOL)
#define EDEN_MAX_FLAG (1 << EDEN_LOG_MAX_FLAG)

/*
 * comment out to mask
 */
static int glive_log = EDEN_LOG_FORCE | EDEN_NN | EDEN_RT | EDEN_UD |
                       EDEN_LINK | EDEN_EMA | EDEN_GTEST | EDEN_HIDL |
                       EDEN_RT_STUB | EDEN_DRIVER | EDEN_CL | EDEN_TOOL;

#define IS_LOG_ON(FLAG) ((glive_log & (FLAG)) != 0)

#define __SHARP_X(x) #x
#define __STR(x) __SHARP_X(x)

#if defined(LINUX_LOG)
#include <stdio.h>
#define LL_DBG 'D'
#define LL_INF 'I'
#define LL_WRN 'W'
#define LL_ERR 'E'
#ifdef EDEN_DEBUG
#define __LOGX(flag, log_level, fmt, ...)               \
  if (IS_LOG_ON(flag)) {                                \
    fprintf(stdout,                                     \
            "[Exynos][EDEN][" EDEN_VERSION "][" LOG_TAG \
            "][%c] %s:" __STR(__LINE__) ": " fmt "\n",  \
            log_level, __FUNCTION__, ##__VA_ARGS__);    \
    fflush(stdout);                                     \
  }
#else
#define __LOGX(flag, log_level, fmt, ...)               \
  if (IS_LOG_ON(flag)) {                                \
    fprintf(stdout,                                     \
            "[Exynos][EDEN][" EDEN_VERSION "][" LOG_TAG \
            "][%c] %s:" __STR(__LINE__) ": " fmt "\n",  \
            log_level, __FUNCTION__, ##__VA_ARGS__);    \
  }
#endif
#elif defined(ANDROID_LOG)
#include <android/log.h>
#define LL_DBG ANDROID_LOG_DEBUG
#define LL_INF ANDROID_LOG_INFO
#define LL_WRN ANDROID_LOG_WARN
#define LL_ERR ANDROID_LOG_ERROR
#define __LOGX(flag, log_level, fmt, ...)                           \
  if (IS_LOG_ON(flag)) {                                            \
    __android_log_print(log_level, "EDEN",                          \
                        "[Exynos][EDEN][" EDEN_VERSION "][" LOG_TAG \
                        "] %s:" __STR(__LINE__) ": " fmt,           \
                        __FUNCTION__, ##__VA_ARGS__);               \
  }
#else
#error "Either LINUX_LOG or ANDROID_LOG should be applied."
#endif

#define LOGD(flag, fmt, ...) __LOGX(flag, LL_DBG, fmt, ##__VA_ARGS__)
#define LOGI(flag, fmt, ...) __LOGX(flag, LL_INF, fmt, ##__VA_ARGS__)
#define LOGW(flag, fmt, ...) __LOGX(flag, LL_WRN, fmt, ##__VA_ARGS__)
#define LOGE(flag, fmt, ...) __LOGX(flag, LL_ERR, fmt, ##__VA_ARGS__)

/*
 * override LOGD only if EDEN_DEBUG is not applied
 */
#ifndef EDEN_DEBUG
#undef LOGD
#define LOGD(FLAG, ...) \
  {}
#endif  // !EDEN_DEBUG

/*
 * override all if REMOVE_LOG is applied
 */
#ifdef REMOVE_LOG
#undef __LOGX
#define __LOGX(flag, ...) \
  {}
#endif  // REMOVE_LOG

#endif  // COMMON_LOG_H_
