/*
 * Copyright (C) 2021 MediaTek Inc., this file is modified on 02/26/2021
 * by MediaTek Inc. based on MIT License .
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the ""Software""), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED ""AS IS"", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <android/log.h>
#include <dlfcn.h>
#include <cstdlib>

#include <memory>
#include <utility>

using namespace std;

typedef enum {
  LOW_POWER_MODE = 0,       // For model execution preference
  FAST_SINGLE_ANSWER_MODE,  // For model execution preference
  SUSTAINED_SPEED_MODE,     // For model execution preference
  FAST_COMPILE_MODE,        // For model compile preference
  PERFORMANCE_MODE_MAX,
} PERFORMANCE_MODE_E;

//------------------------------------- -------------------------------------
#define APUWARE_LOG_D(format, ...)                                  \
  __android_log_print(ANDROID_LOG_DEBUG, "APUWARELIB", format "\n", \
                      ##__VA_ARGS__);

#define APUWARE_LOG_E(format, ...)                                  \
  __android_log_print(ANDROID_LOG_ERROR, "APUWARELIB", format "\n", \
                      ##__VA_ARGS__);

#define LOAD_APUWARE_UTILS_FUNCTION(name) \
  static name##_fn fn =                   \
      reinterpret_cast<name##_fn>(loadApuWareUtilsFunction(#name));

#define EXECUTE_APUWARE_UTILS_FUNCTION(...) \
  if (fn != NULL) {                         \
    fn(__VA_ARGS__);                        \
  }

#define EXECUTE_APUWARE_UTILS_FUNCTION_RETURN(...) return fn(__VA_ARGS__);

static void* sAPUWareUtilsLibHandle;

inline void* loadApuWareUtilsLibrary(const char* name) {
  sAPUWareUtilsLibHandle = dlopen(name, RTLD_LAZY | RTLD_LOCAL);

  if (sAPUWareUtilsLibHandle == nullptr) {
    char* error = dlerror();
    if (error != nullptr) {
      APUWARE_LOG_E("unable to open library %s, with error %s", name, error);
    } else {
      APUWARE_LOG_E("unable to open library %s", name);
    }
#ifdef ABORT_ON_DLOPEN_ERROR
    abort();
#endif
    return nullptr;
  } else {
    APUWARE_LOG_D("ApuWare : open library %s", name);
  }
  return sAPUWareUtilsLibHandle;
}

inline void* getApuWareUtilsLibraryHandle() {
  if (sAPUWareUtilsLibHandle == nullptr) {
    sAPUWareUtilsLibHandle =
        loadApuWareUtilsLibrary("libapuwareutils_v2.mtk.so");
    if (sAPUWareUtilsLibHandle == nullptr) {
      sAPUWareUtilsLibHandle =
          loadApuWareUtilsLibrary("libapuwareutils.mtk.so");
    }
  }
  return sAPUWareUtilsLibHandle;
}

inline void* loadApuWareUtilsFunction(const char* name) {
  void* fn = nullptr;
  char* error = nullptr;
  if (getApuWareUtilsLibraryHandle() != nullptr) {
    fn = dlsym(getApuWareUtilsLibraryHandle(), name);
    if (fn == nullptr) {
      if ((error = dlerror()) != NULL) {
        APUWARE_LOG_E("unable to open function %s, with error %s", name, error);
        return nullptr;
      }
    }
  }
  return fn;
}

typedef int32_t (*acquirePerformanceLockInternal_fn)(
    int32_t hdl, PERFORMANCE_MODE_E perfMode, uint32_t duration);
typedef bool (*releasePerformanceLockInternal_fn)(int32_t hdl);
typedef int32_t (*acquirePerfParamsLockInternal_fn)(int32_t hdl,
                                                    uint32_t duration,
                                                    int32_t boostList[],
                                                    uint32_t numParams);

#ifdef __cplusplus
extern "C" {
#endif

inline int32_t acquirePerformanceLock(int32_t hdl, PERFORMANCE_MODE_E perfMode,
                                      uint32_t duration) {
  LOAD_APUWARE_UTILS_FUNCTION(acquirePerformanceLockInternal);
  EXECUTE_APUWARE_UTILS_FUNCTION_RETURN(hdl, perfMode, duration);
}

inline bool releasePerformanceLock(int32_t hdl) {
  LOAD_APUWARE_UTILS_FUNCTION(releasePerformanceLockInternal);
  EXECUTE_APUWARE_UTILS_FUNCTION_RETURN(hdl);
}

inline int32_t acquirePerfParamsLock(int32_t hdl, uint32_t duration,
                                     int32_t boostList[], uint32_t numParams) {
  LOAD_APUWARE_UTILS_FUNCTION(acquirePerfParamsLockInternal);
  EXECUTE_APUWARE_UTILS_FUNCTION_RETURN(hdl, duration, boostList, numParams);
}

#ifdef __cplusplus
}  // extern "C"
#endif
