# Copyright 2026 The MLPerf Authors. All Rights Reserved.
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

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

pbtxt2header(
    name = "litert_settings",
    srcs = [
        "backend_settings/litert_settings_android.pbtxt",
    ],
)

cc_library(
    name = "litert_c",
    srcs = [
        "litert_c.cc",
        "llm_pipeline.cc",
    ],
    hdrs = [
        "litert_settings_android.h",
        "llm_pipeline.h",
        "pipeline.h",
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
        ":litert_settings",
        "//flutter/cpp:utils",
        "//flutter/cpp/c:headers",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/types:span",
        "@litert//litert/c:litert_environment",
        "@litert//litert/c:litert_environment_options",
        "@litert//litert/c:litert_event",
        "@litert//litert/c:litert_opaque_options",
        "@litert//litert/c:litert_tensor_buffer",
        "@litert//litert/c/internal:litert_accelerator_registration",
        "@litert//litert/c/internal:litert_external_litert_buffer_context",
        "@litert//litert/c/internal:litert_tensor_buffer_registry",
        "@litert//litert/c/options:litert_gpu_options",
        "@litert//litert/cc:litert_compiled_model",
        "@litert//litert/cc:litert_environment",
        "@litert//litert/cc:litert_tensor_buffer",
        "@litert//tflite/c:c_api",
        "@litert//tflite/c:common",
        "@org_tensorflow//tensorflow/core:tflite_portable_logging",
    ],
    alwayslink = 1,
)

tflite_jni_binary(
    name = "liblitertbackend.so",
    linkscript = "//flutter/cpp/c:version_script.lds",
    deps = [
        ":litert_c",
    ],
)
