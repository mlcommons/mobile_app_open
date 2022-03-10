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
load("@org_tensorflow//tensorflow/lite:build_def.bzl", "tflite_linkopts")

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

proto_library(
    name = "mlperf_task_proto",
    srcs = [
        "backend_setting.proto",
        "mlperf_task.proto",
    ],
    visibility = ["//visibility:public"],
)

cc_proto_library(
    name = "mlperf_task_cc_proto",
    deps = [":mlperf_task_proto"],
)

java_proto_library(
    name = "mlperf_task_java_proto",
    deps = [":mlperf_task_proto"],
)

common_linkopts = tflite_linkopts() + select({
    "//conditions:default": [],
    "@org_tensorflow//tensorflow:android": [
        "-llog",
        "-static-libgcc",
        "-pie",
    ],
})

cc_binary(
    name = "proto_test",
    srcs = [
        "test.cc",
    ],
    linkopts = common_linkopts,
    deps = [
        "//flutter/cpp:mlperf_driver",
        "//flutter/cpp:utils",
        "//flutter/cpp/proto:mlperf_task_cc_proto",
        "@com_google_protobuf//:protobuf",
        "@org_tensorflow//tensorflow/lite/tools:command_line_flags",
    ],
)
