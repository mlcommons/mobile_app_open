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

load("@org_tensorflow//tensorflow/lite:build_def.bzl", "tflite_copts", "tflite_jni_binary")

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

config_setting(
    name = "android_arm64",
    values = {"cpu": "arm64-v8a"},
)

cc_library(
    name = "qti_mock",
    srcs = ["qti_mock_c.cc"],
    copts = tflite_copts(),
    deps = [
        "//flutter/cpp/c:headers",
    ],
    alwayslink = 1,
)

tflite_jni_binary(
    name = "libqtibackend.so",
    linkscript = "//flutter/cpp/c:version_script.lds",
    deps = [":qti_mock"],
)

cc_binary(
    name = "libqtibackend.dll",
    linkshared = 1,
    win_def_file = "//android/cpp/c:dll_export.def",
    deps = [
        ":qti_mock",
    ],
)
