# Copyright 2019 The MLPerf Authors. All Rights Reserved.
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

load("@org_tensorflow//tensorflow/lite:build_def.bzl", "tflite_copts", "tflite_linkopts")

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

common_linkopts = tflite_linkopts() + select({
    "//conditions:default": [],
    "@org_tensorflow//tensorflow:android": [
        "-pie",
    ],
}) + select({
    "//android/commonlibs:use_asan": [
        "-fsanitize=address",
        "-g",
        "-O1",
        "-fno-omit-frame-pointer",
    ],
    "//conditions:default": [],
})

cc_library(
    name = "mlperf_driver",
    srcs = ["mlperf_driver.cc"],
    hdrs = [
        "backend.h",
        "dataset.h",
        "mlperf_driver.h",
    ],
    copts = tflite_copts() + select({
        "//android/commonlibs:use_asan": [
            "-fsanitize=address",
            "-g",
            "-O1",
            "-fno-omit-frame-pointer",
        ],
        "//conditions:default": [],
    }),
    deps = [
        ":utils",
        "//flutter/cpp/proto:mlperf_task_cc_proto",
        "@org_mlperf_inference//:loadgen",
    ],
)

cc_library(
    name = "utils",
    srcs = ["utils.cc"],
    hdrs = [
        "utils.h",
    ],
    copts = tflite_copts() + select({
        "//android/commonlibs:use_asan": [
            "-fsanitize=address",
            "-g",
            "-O1",
            "-fno-omit-frame-pointer",
        ],
        "//conditions:default": [],
    }),
    deps = [
        "//flutter/cpp/c:headers",
        "//flutter/cpp/proto:mlperf_task_cc_proto",
        "@org_mlperf_inference//:loadgen",
        "@org_tensorflow//tensorflow/core:tflite_portable_logging",
        "@org_tensorflow//tensorflow/lite/tools/evaluation:utils",
    ],
)

cc_test(
    name = "utils_test",
    srcs = ["utils_test.cc"],
    linkopts = common_linkopts,
    linkstatic = 1,
    deps = [
        ":utils",
        "@com_google_googletest//:gtest",
    ],
)
