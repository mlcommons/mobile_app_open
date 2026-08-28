/* Copyright 2026 The MLPerf Authors. All Rights Reserved.

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
#ifndef LITERT_APPLE_SUPPORT_H_
#define LITERT_APPLE_SUPPORT_H_

#if defined(__APPLE__)

#include <dlfcn.h>
#include <os/proc.h>
#include <sys/stat.h>

#include <string>

#include "absl/log/log.h"

// LiteRT dlopens its GPU accelerator (libLiteRtMetalAccelerator.dylib on Apple)
// by joining the kLiteRtEnvOptionTagRuntimeLibraryDir environment option with
// the library filename. That option is mandatory here: iOS has no dlopen
// directory search, and the app passes an empty native_lib_path to
// mlperf_backend_create (device_info.dart returns '' on iOS), so the directory
// has to be recovered from this binary's own location instead.
namespace litert_apple {

// How many bytes this process can still allocate before iOS kills it. This is
// the budget EXC_RESOURCE enforces (3376 MB on an 8 GB device), and it is the
// only thing that decides which benchmarks this backend can claim, so log it
// around the phases that allocate rather than reasoning about it from model
// sizes. Cheap: a counter read, not a scan.
inline void LogAvailableMemory(const char* stage) {
  const size_t available = os_proc_available_memory();
  // 0 means the call is unavailable (it needs an app context), not that the
  // process is out of memory -- do not report that as an imminent kill.
  if (available == 0) return;
  LOG(INFO) << "[mem] " << stage << ": " << (available / (1024 * 1024))
            << " MiB left before the iOS limit";
}

inline bool FileExists(const std::string& path) {
  struct stat info;
  return stat(path.c_str(), &info) == 0;
}

// Returns the parent directory of `path`, or an empty string if there is none.
inline std::string DirName(const std::string& path) {
  const size_t slash = path.rfind('/');
  if (slash == std::string::npos) return "";
  if (slash == 0) return "/";
  return path.substr(0, slash);
}

}  // namespace litert_apple

// Returns the directory holding libLiteRtMetalAccelerator.dylib inside the app
// bundle, or an empty string when it cannot be found (the caller then runs on
// CPU). dladdr gives this binary's path, e.g.
// .../MyApp.app/Frameworks/liblitertbackend.framework/liblitertbackend, and the
// accelerator sits either next to it or one level up in
// .../MyApp.app/Frameworks.
inline std::string GetAppleRuntimeLibraryDir() {
  static constexpr char kAcceleratorName[] = "libLiteRtMetalAccelerator.dylib";

  Dl_info info;
  if (dladdr(reinterpret_cast<const void*>(&GetAppleRuntimeLibraryDir),
             &info) == 0 ||
      info.dli_fname == nullptr) {
    LOG(WARNING) << "dladdr failed to locate the LiteRT backend binary; the "
                    "Metal accelerator will not be loaded";
    return "";
  }

  std::string dir = litert_apple::DirName(std::string(info.dli_fname));
  for (int level = 0; level < 2 && !dir.empty(); ++level) {
    if (litert_apple::FileExists(dir + "/" + kAcceleratorName)) {
      LOG(INFO) << "LiteRT runtime library dir: " << dir;
      return dir;
    }
    dir = litert_apple::DirName(dir);
  }

  LOG(WARNING) << kAcceleratorName << " not found near " << info.dli_fname
               << "; the Metal accelerator is unavailable and the pipeline "
                  "falls back to CPU";
  return "";
}

#endif  // defined(__APPLE__)

#endif  // LITERT_APPLE_SUPPORT_H_
