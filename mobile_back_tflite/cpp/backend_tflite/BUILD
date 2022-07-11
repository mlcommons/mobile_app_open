# Copyright 2019-2021 The MLPerf Authors. All Rights Reserved.
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
load(
    "@org_tensorflow//tensorflow/lite:build_def.bzl",
    "tflite_copts",
    "tflite_jni_binary",
)

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

cc_library(
    name = "tflite_c",
    srcs = ["tflite_c.cc"],
    hdrs = [
        "tflite_settings_android.h",
        "tflite_settings_apple.h",
        "tflite_settings_windows.h",
        "thread_pool.h",
        "utils.h",
    ],
    copts = tflite_copts() + select({
        "//flutter/android/commonlibs:use_asan": [
            "-fsanitize=address",
            "-g",
            "-O1",
            "-fno-omit-frame-pointer",
        ],
        "//conditions:default": [],
    }),
    deps = [
        "//flutter/cpp/c:headers",
        "@org_tensorflow//tensorflow/lite/c:c_api",
        "@org_tensorflow//tensorflow/lite/c:common",
        "@org_tensorflow//tensorflow/core:tflite_portable_logging",
    ] + select({
        "@org_tensorflow//tensorflow:android": [
            "@org_tensorflow//tensorflow/lite/delegates/gpu:delegate",
        ],
        "@org_tensorflow//tensorflow:ios": [
            "@org_tensorflow//tensorflow/lite/delegates/coreml:coreml_delegate",
            "@org_tensorflow//tensorflow/lite/delegates/gpu:metal_delegate",
        ],
        "//conditions:default": [],
    }),
    alwayslink = 1,
)

tflite_jni_binary(
    name = "libtflitebackend.so",
    linkscript = "//flutter/cpp/c:version_script.lds",
    deps = [
        ":tflite_c",
    ],
)

cc_binary(
    name = "libtflitebackend.dll",
    linkshared = 1,
    win_def_file = "//flutter/cpp/c:dll_export.def",
    deps = [
        ":tflite_c",
    ],
)
