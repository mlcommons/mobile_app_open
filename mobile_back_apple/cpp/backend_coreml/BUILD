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

# we borrow tflite_jni_binary even for non-tflite one
load("@org_tensorflow//tensorflow/lite:build_def.bzl", "tflite_jni_binary")
load("@build_bazel_rules_apple//apple:apple.bzl", "apple_xcframework")
load("@build_bazel_rules_swift//swift:swift.bzl", "swift_library")

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

apple_xcframework(
    name = "libcoremlbackend",
    bundle_id = "com.mlcommons.inference.backend-coreml",
    exported_symbols_lists = ["//flutter/cpp/c:exported_symbols.lds"],
    infoplists = ["//flutter/cpp/flutter:BackendBridgeInfo.plist"],
    ios = {
        "simulator": [
            "x86_64",
            # cpuinfo doesn't support simulator on ARM-based macs
            # "ios_sim_arm64",
        ],
        "device": ["arm64"],
    },
    minimum_os_versions = {
        "ios": "12.0",
    },
    deps = [
        "//mobile_back_apple/cpp/backend_coreml:coreml_c",
    ],
)

objc_library(
    name = "apple_frameworks",
    sdk_frameworks = ["CoreML"],
)

swift_library(
    name = "coreml_util",
    srcs = [
        "coreml_util.swift",
    ],
    deps = [
        ":apple_frameworks",
    ],
)

cc_library(
    name = "coreml_c",
    srcs = [
        "coreml_settings.h",
        "coreml_util.h",
        "main.cc",
    ],
    deps = [
        ":apple_frameworks",
        ":coreml_util",
        "//flutter/cpp:utils",
        "//flutter/cpp/c:headers",
    ],
    alwayslink = 1,
)

tflite_jni_binary(
    name = "libcoremlbackend.so",
    exported_symbols = "//flutter/cpp/c:exported_symbols.lds",
    linkscript = "//flutter/cpp/c:version_script.lds",
    deps = [
        ":coreml_c",
    ],
)
