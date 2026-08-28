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
#ifndef LITERT_ENV_H_
#define LITERT_ENV_H_

#include <string>
#include <vector>

#include "absl/log/log.h"
#include "litert/cc/litert_environment.h"
#include "litert/cc/litert_environment_options.h"
#include "litert/cc/litert_expected.h"

#if defined(__APPLE__)
#include "apple_support.h"
#endif

// Log the remaining process memory budget at a phase boundary. Only Apple
// enforces a per-process limit tight enough to matter here, so this is a no-op
// everywhere else and costs nothing.
#if defined(__APPLE__)
#define LITERT_LOG_MEM(stage) ::litert_apple::LogAvailableMemory(stage)
#else
#define LITERT_LOG_MEM(stage) ((void)0)
#endif

// Creates the LiteRT environment the compiled models are built in. Both
// pipelines share it so the platform-conditional setup stays in one place.
//
// On Apple the GPU (Metal) accelerator is dlopened from the runtime library
// directory, which has to be passed explicitly; see apple_support.h. Everywhere
// else LiteRT uses its default library search, so no options are needed.
inline litert::Expected<litert::Environment> CreateLiteRtEnvironment() {
#if defined(__APPLE__)
  const std::string runtime_library_dir = GetAppleRuntimeLibraryDir();
  if (!runtime_library_dir.empty()) {
    const std::vector<litert::EnvironmentOptions::Option> env_options = {
        {litert::EnvironmentOptions::Tag::kRuntimeLibraryDir,
         runtime_library_dir.c_str()}};
    auto env = litert::Environment::Create(litert::EnvironmentOptions(
        litert::Span<const litert::EnvironmentOptions::Option>(
            env_options.data(), env_options.size())));
    if (env) return env;
    // The directory is only a hint for the accelerator loader, so never fail
    // the backend over it: without it the compile falls back to CPU.
    LOG(WARNING) << "Environment::Create with a runtime library dir failed; "
                 << "retrying without it";
  }
#endif
  return litert::Environment::Create({});
}

#endif  // LITERT_ENV_H_
