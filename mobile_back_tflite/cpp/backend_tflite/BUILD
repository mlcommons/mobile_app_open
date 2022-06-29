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
        "thread_pool.h",
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
        "//android/cpp/c:headers",
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

cc_library(
    name = "neuron_delegate",
    srcs = select({
        "@org_tensorflow//tensorflow:android": [
            "@neuron_delegate//neuron/java:libtensorflowlite_neuron_jni.so",
        ],
    }),
    hdrs = select({
        "@org_tensorflow//tensorflow:android": [
            "@neuron_delegate//neuron:APUWareUtilsApi.h",
            "@neuron_delegate//neuron:neuron_delegate.h",
        ],
    }),
)

cc_library(
    name = "tflite_neuron_c",
    srcs = ["tflite_c.cc"],
    hdrs = [
        "neuron/tflite_settings_mtk.h",
        "thread_pool.h",
        "utils.h",
    ],
    copts = tflite_copts() + [
        "-Iexternal/neuron_delegate",
    ] + select({
        "//android/commonlibs:use_asan": [
            "-fsanitize=address",
            "-g",
            "-O1",
            "-fno-omit-frame-pointer",
        ],
        "//conditions:default": [],
    }),
    local_defines = ["MTK_TFLITE_NEURON_BACKEND"],
    deps = [
        "//android/cpp/c:headers",
        "@org_tensorflow//tensorflow/lite/c:c_api",
        "@org_tensorflow//tensorflow/lite/c:common",
        "@org_tensorflow//tensorflow/core:tflite_portable_logging",
    ] + select({
        "@org_tensorflow//tensorflow:android": [
            ":neuron_delegate",
            "@org_tensorflow//tensorflow/lite/delegates/gpu:delegate",
        ],
        "//conditions:default": [],
    }),
    alwayslink = 1,
)

tflite_jni_binary(
    name = "libtflitebackend.so",
    exported_symbols = "//android/cpp/c:exported_symbols.lds",
    linkscript = "//android/cpp/c:version_script.lds",
    deps = [
        ":tflite_c",
    ],
)

tflite_jni_binary(
    name = "libtfliteneuronbackend.so",
    exported_symbols = "//android/cpp/c:exported_symbols.lds",
    linkscript = "//android/cpp/c:version_script.lds",
    deps = [
        ":tflite_neuron_c",
    ],
)

aar_import(
    name = "tflite_neuron_aar",
    aar = "@neuron_delegate//neuron/java:tensorflow-lite-neuron.aar",
)

cc_binary(
    name = "libtflitebackend.dll",
    linkshared = 1,
    win_def_file = "//android/cpp/c:dll_export.def",
    deps = [
        ":tflite_c",
    ],
)
