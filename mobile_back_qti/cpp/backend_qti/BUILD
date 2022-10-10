# Copyright (c) 2020-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
##########################################################################

#
# This file contains an application that loads and executes a neural network
# using the SNPE C++ API and saves the MLPerf output to a file.
# Inputs to and outputs from the network are conveyed in binary form as single
# precision floating point values.
#

load("@snpe_version_loader//:snpe_var_def.bzl", "SNPE_VERSION")
load("@org_tensorflow//tensorflow/lite:build_def.bzl", "tflite_copts", "tflite_jni_binary")
load("@bazel_skylib//rules:common_settings.bzl", "string_flag")

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

config_setting(
    name = "android_arm64",
    values = {"cpu": "arm64-v8a"},
)

string_flag(
    name = "with_qti",
    values = ["1","2"],
    build_setting_default = "1",
)

config_setting(
    name = "debug_option",
    flag_values = {":with_qti": "2"},
)

snpe_copts = [
    "-Imobile_back_qti/" + SNPE_VERSION + "/include/zdl",
    "-fexceptions",
    "-lc++_shared",
]

cc_library(
    name = "qti_rpcmem",
    srcs = ["rpcmem.cc"],
    hdrs = [
        "cpuctrl.h",
        "soc_utility.h",
        "rpcmem.h",
        "qti_settings.h"
    ],
    copts = tflite_copts() + snpe_copts,
    deps = ["@org_tensorflow//tensorflow/core:tflite_portable_logging"],
    alwayslink = 1,
)

cc_library(
    name = "qti_allocator",
    srcs = ["allocator.cc"],
    hdrs = ["allocator.h"],
    copts = tflite_copts() + snpe_copts,
    deps = [
        ":qti_rpcmem",
        "@org_tensorflow//tensorflow/core:tflite_portable_logging",
    ],
    alwayslink = 1,
)

cc_library(
    name = "qti_allocator_x86",
    srcs = ["allocator.cc"],
    hdrs = ["allocator.h"],
    deps = [
        ":qti_rpcmem",
        "@org_tensorflow//tensorflow/core:tflite_portable_logging",
    ],
    alwayslink = 1,
)

cc_library(
    name = "qti_c",
    srcs = select({
        "android_arm64": [
            "qti_c.cc",
            "cpuctrl.cc",
            "tflite_c.cc",
            "qti_backend_helper.cc",
            "soc_utility.cc",
        ],
    }),
    hdrs = [] + select({
        "android_arm64": [
            "qti_settings.h",
            "cpuctrl.h",
            "tflite_c.h",
            "qti_backend_helper.h",
            "mlperf_helper.h",
            "soc_utility.h"
        ],
    }),
    copts = tflite_copts() + snpe_copts,
    local_defines = ["SNPE_VERSION_STRING=" + SNPE_VERSION,] +
                    select({
                       "debug_option":
                          ["DEBUG_FLAG",],
                       "//conditions:default": [],
                    }),
    deps = [
        "//flutter/cpp/c:headers",
        ":qti_allocator",
    ] + select({
        "android_arm64": [
            "@org_tensorflow//tensorflow/lite/c:c_api",
            "@org_tensorflow//tensorflow/lite/c:common",
            "@org_tensorflow//tensorflow/lite/delegates/gpu:delegate",
            "@org_tensorflow//tensorflow/core:tflite_portable_logging",
            "//mobile_back_qti:snpe",
        ],
    }),
    alwayslink = 1,
)

tflite_jni_binary(
    name = "libqtibackend.so",
    linkscript = "//flutter/cpp/c:version_script.lds",
    deps = [
        ":qti_c",
    ],
)

cc_binary(
    name = "qti_backend_test",
    srcs = [] + select({
        "android_arm64": [
            "test.cc",
        ],
    }),
    linkopts = ["-pie"],
    linkstatic = 1,
    deps = [] + select({
        "android_arm64": [
            ":libqtibackend.so",
            "//flutter/cpp/c:headers",
            "@org_tensorflow//tensorflow/lite:shared_library",
        ],
    }),
)

cc_binary(
    name = "allocator_test",
    srcs = [
        "allocator_test.cc",
    ],
    linkstatic = 1,
    deps = [
        ":qti_allocator_x86",
    ],
)
