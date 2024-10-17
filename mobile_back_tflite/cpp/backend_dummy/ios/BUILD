# Copyright 2022 The MLPerf Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================

load("@build_bazel_rules_apple//apple:apple.bzl", "apple_xcframework")

# This target is required to have the same name
# as the true tflite backend.
# Otherwise xcode will fail to build the app.
apple_xcframework(
    name = "libtflitebackend",
    bundle_id = "com.mlcommons.inference.backend-tflite-dummy",
    exported_symbols_lists = ["//flutter/cpp/c:exported_symbols.lds"],
    infoplists = ["//flutter/cpp/flutter:BackendBridgeInfo.plist"],
    ios = {
        "simulator": ["x86_64"],
        "device": ["arm64"],
    },
    minimum_os_versions = {
        "ios": "13.1",
        "macos": "13.1",
    },
    deps = [
        "//mobile_back_tflite/cpp/backend_dummy:dummy_backend",
    ],
)

# This target is required to have the same name
# as the true coreml backend.
# Otherwise xcode will fail to build the app.
apple_xcframework(
    name = "libcoremlbackend",
    bundle_id = "com.mlcommons.inference.backend-coreml-dummy",
    exported_symbols_lists = ["//flutter/cpp/c:exported_symbols.lds"],
    infoplists = ["//flutter/cpp/flutter:BackendBridgeInfo.plist"],
    ios = {
        "simulator": ["x86_64"],
        "device": ["arm64"],
    },
    minimum_os_versions = {
        "ios": "13.1",
        "macos": "13.1",
    },
    deps = [
        "//mobile_back_tflite/cpp/backend_dummy:dummy_backend",
    ],
)
