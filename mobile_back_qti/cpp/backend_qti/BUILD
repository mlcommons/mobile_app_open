# Copyright (c) 2020-2025 Qualcomm Innovation Center, Inc. All rights reserved.
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

load("@bazel_skylib//rules:common_settings.bzl", "string_flag")
load("@bazel_skylib//lib:selects.bzl", "selects")
load("@org_tensorflow//tensorflow/lite:build_def.bzl", "tflite_copts", "tflite_jni_binary")
load("@snpe_version_loader//:snpe_var_def.bzl", "SNPE_VERSION")
load("//flutter/cpp/proto:pbtxt2header.bzl", "pbtxt2header")

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

config_setting(
    name = "android_arm64",
    values = {"cpu": "arm64-v8a"},
)

config_setting(
    name = "windows_arm64",
    values = {"cpu": "x64_arm64_windows"},
)

string_flag(
    name = "with_qti",
    build_setting_default = "1",
    values = [
        "1",
        "2",
    ],
)

string_flag(
    name = "with_stablediffusion",
    build_setting_default = "2",
    values = [
        "1",
        "2",
    ],
)

config_setting(
    name = "stablediffusion_option",
    flag_values = {":with_stablediffusion": "1"},
)

config_setting(
    name = "nostablediffusion_option",
    flag_values = {":with_stablediffusion": "2"},
)

selects.config_setting_group(
    name = "android_with_stablediffusion",
    match_all = [
        "android_arm64",
        "stablediffusion_option",
    ],
)

selects.config_setting_group(
    name = "android_without_stablediffusion",
    match_all = [
        "android_arm64",
        "nostablediffusion_option",
    ],
)

config_setting(
    name = "debug_option",
    flag_values = {":with_qti": "2"},
)

string_flag(
    name = "external_config",
    build_setting_default = "2",
    values = [
        "1",
        "2",
    ],
)

config_setting(
    name = "config_option",
    flag_values = {":external_config": "1"},
)

snpe_copts = [
    "-Imobile_back_qti/" + "qairt/" + SNPE_VERSION + "/include/SNPE",
    "-Imobile_back_qti/" + "qairt/" + SNPE_VERSION + "/include/QNN",
    "-Imobile_back_qti/" + "qairt/" + SNPE_VERSION + "/include/QNN/HTP",
    "-fcxx-exceptions",
    "-lc++_shared",
]

pbtxt2header(
    name = "qti_settings",
    srcs = [
        "settings/qti_settings_default_cpu.pbtxt",
        "settings/qti_settings_default_dsp.pbtxt",
        "settings/qti_settings_default_gpu.pbtxt",
        "settings/qti_settings_gpufp16.pbtxt",
        "settings/qti_settings_sd4g2.pbtxt",
        "settings/qti_settings_sd7cxg3.pbtxt",
        "settings/qti_settings_sd7g3.pbtxt",
        "settings/qti_settings_sd7sg3.pbtxt",
        "settings/qti_settings_sd8cxg3.pbtxt",
        "settings/qti_settings_sd8elite.pbtxt",
        "settings/qti_settings_sd8g3.pbtxt",
        "settings/qti_settings_sd8sg3.pbtxt",
        "settings/qti_settings_stablediffusion.pbtxt",
        "settings/qti_settings_stablediffusion_v75.pbtxt",
        "settings/qti_settings_stablediffusion_v79.pbtxt",
    ],
)

cc_library(
    name = "qti_rpcmem",
    srcs = ["rpcmem.cc"],
    hdrs = [
        "cpuctrl.h",
        "qti_settings.h",
        "rpcmem.h",
        "soc_utility.h",
    ],
    copts = tflite_copts() + snpe_copts,
    deps = [
        ":qti_settings",
        "@org_tensorflow//tensorflow/core:tflite_portable_logging",
    ],
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
    srcs = [
        "cpuctrl.cc",
        "qti_backend_helper.cc",
        "qti_c.cc",
        "soc_utility.cc",
        "tflite_c.cc",
    ],
    hdrs = [
        "cpuctrl.h",
        "mlperf_helper.h",
        "qti_backend_helper.h",
        "qti_settings.h",
        "soc_utility.h",
        "tflite_c.h",
    ] + select({
        "android_arm64": [],
        "windows_arm64": [
            "acpitabl.h",
        ],
        "//conditions:default": [],
    }),
    copts = tflite_copts() + snpe_copts,
    local_defines = ["SNPE_VERSION_STRING=" + SNPE_VERSION] +
                    select({
                        "debug_option": ["DEBUG_FLAG"],
                        "//conditions:default": [],
                        "config_option": ["EXTERNAL_CONFIG"],
                    }) +
                    select({
                        "stablediffusion_option": ["STABLEDIFFUSION_FLAG"],
                        "//conditions:default": [],
                    }),
    deps = [
        "//flutter/cpp/c:headers",
        ":qti_allocator",
        ":qti_settings",
        "@org_tensorflow//tensorflow/core:tflite_portable_logging",
    ] + select({
        ":android_without_stablediffusion": [
            "@org_tensorflow//tensorflow/lite/delegates/gpu:delegate",
            "@org_tensorflow//tensorflow/lite/c:c_api",
            "@org_tensorflow//tensorflow/lite/c:common",
            "//mobile_back_qti:snpe",
        ],
        ":android_with_stablediffusion": [
            "@org_tensorflow//tensorflow/lite/delegates/gpu:delegate",
            "@org_tensorflow//tensorflow/lite/c:c_api",
            "@org_tensorflow//tensorflow/lite/c:common",
            "//mobile_back_qti:snpe",
            "//mobile_back_qti/cpp/backend_qti/StableDiffusionShared:StableDiffusionShared",
        ],
        "windows_arm64": [
            "//mobile_back_qti:snpewindowslib",
        ],
        "//conditions:default": [],
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
    name = "libqtibackend.dll",
    linkshared = 1,
    win_def_file = "//flutter/cpp/c:dll_export.def",
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
