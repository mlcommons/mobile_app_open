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

#include <TargetConditionals.h>
#include <dlfcn.h>
#include <malloc/malloc.h>
#include <os/log.h>
#include <sys/stat.h>

#include <string>

#if TARGET_OS_IPHONE
#include <os/proc.h>
#else
// os_proc_available_memory() is explicitly unavailable on macOS: there is no
// per-process high-watermark limit there to be available against. The dev-utils
// CLI (mobile_back_apple/dev-utils) runs this backend on the host, so the file
// still has to compile; report phys_footprint instead, which is the same
// quantity EXC_RESOURCE measures on device and is what makes host runs
// comparable to device ones.
#include <mach/mach.h>
#include <mach/task.h>
#endif  // TARGET_OS_IPHONE

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
#if TARGET_OS_IPHONE
  const size_t available = os_proc_available_memory();
  // 0 means the call is unavailable (it needs an app context), not that the
  // process is out of memory -- do not report that as an imminent kill.
  if (available == 0) return;
  const size_t mib = available / (1024 * 1024);
  const char* const unit = "MiB left before the iOS limit";
#else
  // macOS: report what the process is holding rather than what is left, since
  // nothing is enforcing a ceiling here. phys_footprint is the same counter
  // EXC_RESOURCE compares against on device, so a host measurement of "this
  // delegate costs N MiB" transfers directly to the device budget.
  task_vm_info_data_t info = {};
  mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
  if (task_info(mach_task_self(), TASK_VM_INFO,
                reinterpret_cast<task_info_t>(&info), &count) != KERN_SUCCESS) {
    return;
  }
  const size_t mib = static_cast<size_t>(info.phys_footprint) / (1024 * 1024);
  const char* const unit = "MiB phys_footprint";
#endif  // TARGET_OS_IPHONE
  LOG(INFO) << "[mem] " << stage << ": " << mib << " " << unit;
  // Also to os_log, which is the only one of the two that reaches a
  // BrowserStack device-log artifact: that capture carries os_log entries
  // (Dart's print arrives that way) but no native stderr, so without this a CI
  // memory failure gives pass/fail and nothing to diagnose it with.
  os_log(OS_LOG_DEFAULT, "[mem] %{public}s: %zu %{public}s", stage, mib, unit);
}

// Hand pages the allocator is holding back to the OS.
//
// Destroying a compiled model frees its weights and arenas, but free() does
// not necessarily shrink the process footprint: libmalloc keeps the pages in
// its free list and stays ready to reuse them. phys_footprint still counts
// them, and phys_footprint is exactly what EXC_RESOURCE measures -- so a
// release that looks correct can leave the budget unchanged. A null zone means
// every zone, and a goal of 0 means reclaim as much as possible.
inline void ReturnFreeMemoryToOS() { malloc_zone_pressure_relief(nullptr, 0); }

// Log a diagnostic line to both sinks, for the same reason LogAvailableMemory
// does: absl goes to native stderr, which a local `flutter run` shows but a
// BrowserStack device-log artifact drops entirely.
inline void LogNote(const std::string& text) {
  LOG(INFO) << text;
  os_log(OS_LOG_DEFAULT, "%{public}s", text.c_str());
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
// .../MyApp.app/Frameworks/liblitertbackend.framework/liblitertbackend, so the
// search starts there and walks up to .../MyApp.app/Frameworks.
//
// The accelerator ships wrapped in LiteRtMetalAccelerator.framework, because an
// iOS app bundle may only embed bundles and App Store Connect rejects a bare
// Mach-O under Frameworks/. The framework's CFBundleExecutable is the dylib's
// original filename, so the path this returns still joins with that name the
// way LiteRT expects. The loose layout is still accepted at each level: it is
// what a non-bundle host (a macOS command-line harness) produces.
inline std::string GetAppleRuntimeLibraryDir() {
  static constexpr char kAcceleratorName[] = "libLiteRtMetalAccelerator.dylib";
  static constexpr char kAcceleratorFramework[] =
      "LiteRtMetalAccelerator.framework";

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
    const std::string candidates[] = {dir, dir + "/" + kAcceleratorFramework};
    for (const std::string& candidate : candidates) {
      if (litert_apple::FileExists(candidate + "/" + kAcceleratorName)) {
        LOG(INFO) << "LiteRT runtime library dir: " << candidate;
        return candidate;
      }
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
