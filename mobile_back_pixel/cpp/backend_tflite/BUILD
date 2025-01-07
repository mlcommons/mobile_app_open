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
load("//flutter/cpp/proto:pbtxt2header.bzl", "pbtxt2header")

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

pbtxt2header(
    name = "pixel_settings",
    srcs = [
        "settings/tflite_settings_pixel6.pbtxt",
    ],
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
        "@org_tensorflow//tensorflow/lite/core/api",
        "@org_tensorflow//tensorflow/lite/kernels:cpu_backend_context",
        "@org_tensorflow//tensorflow/lite/kernels:cpu_backend_threadpool",
        "@org_tensorflow//tensorflow/lite/kernels:kernel_util",
    ],
)

cc_library(
    name = "tflite_pixel",
    srcs = [
        "pixel_single_model_pipeline.cc",
        "tflite_pixel.cc",
        "//mobile_back_tflite/cpp/backend_tflite:sd_utils.cc",
        "//mobile_back_tflite/cpp/backend_tflite:stable_diffusion_invoker.cc",
        "//mobile_back_tflite/cpp/backend_tflite:stable_diffusion_pipeline.cc",
    ],
    hdrs = [
        "tflite_settings_pixel.h",
        "thread_pool.h",
        "//mobile_back_tflite/cpp/backend_tflite:embedding_utils.h",
        "//mobile_back_tflite/cpp/backend_tflite:pipeline.h",
        "//mobile_back_tflite/cpp/backend_tflite:sd_utils.h",
        "//mobile_back_tflite/cpp/backend_tflite:single_model_pipeline.h",
        "//mobile_back_tflite/cpp/backend_tflite:stable_diffusion_invoker.h",
        "//mobile_back_tflite/cpp/backend_tflite:stable_diffusion_pipeline.h",
        "//mobile_back_tflite/cpp/backend_tflite:thread_pool.h",
        "//mobile_back_tflite/cpp/backend_tflite:utils.h",
    ],
    copts = tflite_copts() + select({
        "//flutter/android/commonlibs:use_asan": [
            "-fsanitize=address",
            "-g",
            "-O1",
            "-fno-omit-frame-pointer",
        ],
        "//conditions:default": [
            "-Imobile_back_tflite/cpp/backend_tflite",
        ],
    }),
    deps = [
        ":pixel_settings",
        ":resize_bilinear_op",
        "//flutter/cpp:utils",
        "//flutter/cpp/c:headers",
        "@org_tensorflow//tensorflow/core:tflite_portable_logging",
        "@org_tensorflow//tensorflow/lite/c:c_api",
        "@org_tensorflow//tensorflow/lite/c:c_api_experimental",
        "@org_tensorflow//tensorflow/lite/c:common",
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
    linkscript = "//flutter/cpp/c:version_script.lds",
    deps = [
        ":tflite_pixel",
    ],
)
