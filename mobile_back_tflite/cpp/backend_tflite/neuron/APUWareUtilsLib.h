/*
 * Copyright (C) 2023 MediaTek Inc., this file is modified on 02/26/2021
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

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>
using namespace std;

namespace mtk::performance {

typedef enum {
  LOW_POWER_MODE = 0,       // For model execution preference
  FAST_SINGLE_ANSWER_MODE,  // For model execution preference
  SUSTAINED_SPEED_MODE,     // For model execution preference
  FAST_COMPILE_MODE,        // For model compile preference
  PERFORMANCE_MODE_MAX,
} PERFORMANCE_MODE_E;

static const std::vector<int32_t> kFastSingleAnswerParams = {
    0x00410000, 1,   0x00414000, 1,  0x0143c000, 128,   0x01000000, 0,
    0x01408300, 100, 0x0201c000, 60, 0x0201c100, 60,    0x02020000, 31,
    0x01438400, 0,   0x01438500, 0,  0x01438700, 40000, 0x01438800, 40000,
    0x01c3c100, 0,
};

//------------------------------------- -------------------------------------
#define APUWARE_LOG_D(format, ...)                                  \
  __android_log_print(ANDROID_LOG_DEBUG, "APUWARELIB", format "\n", \
                      ##__VA_ARGS__);

#define APUWARE_LOG_E(format, ...)                                  \
  __android_log_print(ANDROID_LOG_ERROR, "APUWARELIB", format "\n", \
                      ##__VA_ARGS__);

inline void* voidFunction() { return nullptr; }

// ApuWareUtils library construct
struct ApuWareUtilsLib {
  using AcquirePerformanceLockPtr =
      std::add_pointer<int32_t(int32_t, PERFORMANCE_MODE_E, uint32_t)>::type;
  using AcquirePerfParamsLockPtr =
      std::add_pointer<int32_t(int32_t, uint32_t, int32_t[], uint32_t)>::type;
  using ReleasePerformanceLockPtr = std::add_pointer<bool(int32_t)>::type;

  // Open a given library and load symbols
  bool load() {
    void* handle = nullptr;
    const std::string libraries[] = {"libapuwareutils_v2.mtk.so",
                                     "libapuwareutils.mtk.so"};
    for (const auto& lib : libraries) {
      handle = dlopen(lib.c_str(), RTLD_LAZY | RTLD_LOCAL);
      if (handle) {
        APUWARE_LOG_D("dlopen %s", lib.c_str());
        acquirePerformanceLock =
            reinterpret_cast<decltype(acquirePerformanceLock)>(
                dlsym(handle, "acquirePerformanceLockInternal"));
        acquirePerfParamsLock =
            reinterpret_cast<decltype(acquirePerfParamsLock)>(
                dlsym(handle, "acquirePerfParamsLockInternal"));
        releasePerformanceLock =
            reinterpret_cast<decltype(releasePerformanceLock)>(
                dlsym(handle, "releasePerformanceLockInternal"));
        mEnable = acquirePerformanceLock && releasePerformanceLock &&
                  acquirePerfParamsLock;
        return mEnable;
      } else {
        APUWARE_LOG_E("unable to open library %s", lib.c_str());
      }
    }
    return false;
  }

  AcquirePerformanceLockPtr acquirePerformanceLock =
      reinterpret_cast<decltype(acquirePerformanceLock)>(voidFunction);
  AcquirePerfParamsLockPtr acquirePerfParamsLock =
      reinterpret_cast<decltype(acquirePerfParamsLock)>(voidFunction);
  ReleasePerformanceLockPtr releasePerformanceLock =
      reinterpret_cast<decltype(releasePerformanceLock)>(voidFunction);

  bool mEnable = false;
};

class PerformanceLocker {
 public:
  PerformanceLocker() { mLib.load(); }

  void Start(uint32_t ms) {
    if (mLib.mEnable) {
      APUWARE_LOG_D("Powerhal Up %u ms", ms);
      mHandle = mLib.acquirePerfParamsLock(mHandle, ms,
                                           (int*)kFastSingleAnswerParams.data(),
                                           kFastSingleAnswerParams.size());
      APUWARE_LOG_D("PerformanceLocker get handle %d", mHandle);
    } else {
      APUWARE_LOG_D("Powerhal is invalid");
    }
  }
  ~PerformanceLocker() {
    if (mLib.mEnable && mHandle) {
      APUWARE_LOG_D("PerformanceLocker release handle %d", mHandle);
      mLib.releasePerformanceLock(mHandle);
    }
    APUWARE_LOG_D("PerformanceLocker destruct");
  }
  PerformanceLocker(const PerformanceLocker&) = delete;

  PerformanceLocker& operator=(const PerformanceLocker&) = delete;

 private:
  struct ApuWareUtilsLib mLib;

  int32_t mHandle = 0;
};
;

}  // namespace mtk::performance