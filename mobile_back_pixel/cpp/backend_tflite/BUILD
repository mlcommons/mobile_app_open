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
    name = "resize_bilinear_op",
    srcs = [
        "resize_argmax_op.cc",
    ],
    hdrs = [
        "resize_argmax_op.h",
    ],
    deps = [
        "@org_tensorflow//tensorflow/lite/c:common",
        "@org_tensorflow//tensorflow/lite/kernels:kernel_util",
        "@org_tensorflow//tensorflow/lite/core/api",
        "@org_tensorflow//tensorflow/lite/kernels:cpu_backend_context",
        "@org_tensorflow//tensorflow/lite/kernels:cpu_backend_threadpool",
    ],
)

cc_library(
    name = "tflite_pixel",
    srcs = [
        "tflite_pixel.cc",
    ],
    hdrs = [
        "tflite_settings_pixel.h",
        "thread_pool.h",
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
        ":resize_bilinear_op",
        "//android/cpp/c:headers",
        "@org_tensorflow//tensorflow/lite/c:c_api",
        "@org_tensorflow//tensorflow/lite/c:c_api_experimental",
        "@org_tensorflow//tensorflow/lite/c:common",
        "@org_tensorflow//tensorflow/core:tflite_portable_logging",
    ] + select({
        "@org_tensorflow//tensorflow:android": [
            "@org_tensorflow//tensorflow/lite/delegates/gpu:delegate",
        ],
        "//conditions:default": [],
    }),
    alwayslink = 1,
)

tflite_jni_binary(
    name = "libtflitepixelbackend.so",
    exported_symbols = "//android/cpp/c:exported_symbols.lds",
    linkscript = "//android/cpp/c:version_script.lds",
    deps = [
        ":tflite_pixel",
    ],
)
