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
    "@litert//tflite:build_def.bzl",
    "tflite_copts",
    "tflite_jni_binary",
)
load("//flutter/cpp/proto:pbtxt2header.bzl", "pbtxt2header")
load("@litert//litert/build_common:special_rule.bzl", "litert_gpu_accelerator_prebuilts")

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

pbtxt2header(
    name = "tflite_settings",
    srcs = [
        "backend_settings/tflite_settings_android.pbtxt",
        "backend_settings/tflite_settings_apple_iphone11.pbtxt",
        "backend_settings/tflite_settings_apple_iphone12.pbtxt",
        "backend_settings/tflite_settings_apple_iphoneX.pbtxt",
        "backend_settings/tflite_settings_apple_main.pbtxt",
        "backend_settings/tflite_settings_windows.pbtxt",
    ],
)

cc_library(
    name = "embedding_utils",
    srcs = ["embedding_utils.cc"],
    hdrs = ["embedding_utils.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@org_tensorflow//tensorflow/core:tflite_portable_logging",
    ],
)

cc_library(
    name = "tflite_c",
    srcs = [
        "embedding_utils.cc",
        "llm_pipeline.cc",
        "sd_utils.cc",
        "single_model_pipeline.cc",
        "stable_diffusion_invoker.cc",
        "stable_diffusion_pipeline.cc",
        "tflite_c.cc",
    ],
    hdrs = [
        "embedding_utils.h",
        "llm_pipeline.h",
        "pipeline.h",
        "sd_utils.h",
        "single_model_pipeline.h",
        "stable_diffusion_invoker.h",
        "stable_diffusion_pipeline.h",
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
        # fpermissive required due to bug in LiteRT (as of version 2.1.2)
    }) + ["-UTFLITE_USE_OPAQUE_DELEGATE", "-fpermissive"],
    deps = [
        ":embedding_utils",
        ":tflite_settings",
        "//flutter/cpp:utils",
        "//flutter/cpp/c:headers",
        "@org_tensorflow//tensorflow/core:tflite_portable_logging",
        "@litert//tflite:framework",
        "@litert//tflite:util",
        "@litert//tflite/c:c_api",
        "@litert//tflite/c:c_api_experimental",
        "@litert//tflite/c:common",
        "@litert//litert/cc:litert_compiled_model",
        "@litert//litert/cc:litert_environment",
        "@litert//litert/cc:litert_tensor_buffer",
    ]
    + select({
        "@org_tensorflow//tensorflow:android": [
            "@litert//tflite/delegates/gpu:delegate",
            "@litert//tflite/delegates/xnnpack:xnnpack_delegate",
        ],
        "@org_tensorflow//tensorflow:ios": [
            "@litert//tflite/delegates/coreml:coreml_delegate",
            "@litert//tflite/delegates/gpu:metal_delegate",
        ],
        "//conditions:default": [],
    }),
    alwayslink = 1,
)

tflite_jni_binary(
    name = "libtflitebackend.so",
    # macOS uses exported_symbols, while Windows uses linkscript
    exported_symbols = "//flutter/cpp/c:exported_symbols.lds",
    linkscript = "//flutter/cpp/c:version_script.lds",
    deps = [
        ":tflite_c",
    ],
    linkopts = [
        "-Wl,--export-dynamic",
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
