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

load("@org_tensorflow//tensorflow/lite:build_def.bzl", "tflite_copts")

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

cc_library(
    name = "allocator",
    srcs = [
        "allocator.cc",
    ],
    hdrs = [
        "allocator.h",
    ],
    copts = select({
        "//flutter/android/commonlibs:use_asan": [
            "-fsanitize=address",
            "-g",
            "-O1",
            "-fno-omit-frame-pointer",
        ],
        "//conditions:default": [],
    }),
    deps = [
        "//flutter/cpp:utils",
    ],
)

cc_library(
    name = "imagenet",
    srcs = [
        "imagenet.cc",
    ],
    hdrs = [
        "imagenet.h",
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
    }),
    deps = [
        ":allocator",
        "//flutter/cpp:mlperf_driver",
        "//flutter/cpp:utils",
        "//flutter/cpp/backends:external",
        "@org_tensorflow//tensorflow/lite/kernels:kernel_util",
        "@org_tensorflow//tensorflow/lite/tools/evaluation:utils",
        "@org_tensorflow//tensorflow/lite/tools/evaluation/proto:evaluation_stages_cc_proto",
        "@org_tensorflow//tensorflow/lite/tools/evaluation/stages:image_preprocessing_stage",
    ],
)

cc_library(
    name = "coco",
    srcs = [
        "coco.cc",
    ],
    hdrs = [
        "coco.h",
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
    }),
    deps = [
        ":allocator",
        "//flutter/cpp:mlperf_driver",
        "//flutter/cpp:utils",
        "//flutter/cpp/backends:external",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_protobuf//:protobuf",
        "@org_tensorflow//tensorflow/lite/tools/evaluation:utils",
        "@org_tensorflow//tensorflow/lite/tools/evaluation/proto:evaluation_stages_cc_proto",
        "@org_tensorflow//tensorflow/lite/tools/evaluation/stages:image_preprocessing_stage",
        "@org_tensorflow//tensorflow/lite/tools/evaluation/stages:object_detection_average_precision_stage",
    ],
)

cc_library(
    name = "squad",
    srcs = ["squad.cc"],
    hdrs = ["squad.h"],
    copts = select({
        "//flutter/android/commonlibs:use_asan": [
            "-fsanitize=address",
            "-g",
            "-O1",
            "-fno-omit-frame-pointer",
        ],
        "//conditions:default": [],
    }),
    deps = [
        "//flutter/cpp:mlperf_driver",
        "//flutter/cpp:utils",
        "//flutter/cpp/datasets/squad_utils",
    ] + select({
        "@org_tensorflow//tensorflow:android": [
            "@org_tensorflow//tensorflow/core:portable_tensorflow_lib_lite",
        ],
        "//conditions:default": [
            "@org_tensorflow//tensorflow/core:framework",
            "@org_tensorflow//tensorflow/core:lib",
            "@org_tensorflow//tensorflow/core:protos_all_cc",
        ],
    }),
)

cc_library(
    name = "ade20k",
    srcs = ["ade20k.cc"],
    hdrs = [
        "ade20k.h",
        "utils.h",
    ],
    deps = [
        ":allocator",
        "//flutter/cpp:mlperf_driver",
        "//flutter/cpp:utils",
        "//flutter/cpp/backends:external",
        "@org_tensorflow//tensorflow/lite/kernels:kernel_util",
        "@org_tensorflow//tensorflow/lite/tools/evaluation:utils",
        "@org_tensorflow//tensorflow/lite/tools/evaluation/proto:evaluation_stages_cc_proto",
        "@org_tensorflow//tensorflow/lite/tools/evaluation/stages:image_preprocessing_stage",
    ],
)

cc_library(
    name = "snu_sr",
    srcs = ["snu_sr.cc"],
    hdrs = [
        "snu_sr.h",
        "utils.h",
    ],
    deps = [
        ":allocator",
        "//flutter/cpp:mlperf_driver",
        "//flutter/cpp:utils",
        "//flutter/cpp/backends:external",
        "@org_tensorflow//tensorflow/lite/kernels:kernel_util",
        "@org_tensorflow//tensorflow/lite/tools/evaluation:utils",
        "@org_tensorflow//tensorflow/lite/tools/evaluation/proto:evaluation_stages_cc_proto",
        "@org_tensorflow//tensorflow/lite/tools/evaluation/stages:image_preprocessing_stage",
    ],
)

cc_library(
    name = "coco_gen",
    srcs = [
        "coco_gen.cc",
    ],
    hdrs = [
        "coco_gen.h",
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
    }),
    deps = [
        ":allocator",
        "//flutter/cpp:mlperf_driver",
        "//flutter/cpp:utils",
        "//flutter/cpp/backends:external",
        "//flutter/cpp/datasets/coco_gen_utils",
        "//flutter/cpp/datasets/squad_utils",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_protobuf//:protobuf",
        "@org_tensorflow//tensorflow/lite/tools/evaluation:utils",
        "@org_tensorflow//tensorflow/lite/tools/evaluation/proto:evaluation_stages_cc_proto",
        "@org_tensorflow//tensorflow/lite/tools/evaluation/stages:image_preprocessing_stage",
        "@org_tensorflow//tensorflow/lite/tools/evaluation/stages:object_detection_average_precision_stage",
    ],
)

cc_library(
    name = "mmlu_gen",
    srcs = [
        "mmlu_gen.cc",
    ],
    hdrs = [
        "mmlu_gen.h",
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
    }),
    deps = [
        ":allocator",
        "//flutter/cpp:mlperf_driver",
        "//flutter/cpp:utils",
        "//flutter/cpp/backends:external",
        "//flutter/cpp/datasets/squad_utils",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_protobuf//:protobuf",
        "@org_tensorflow//tensorflow/lite/tools/evaluation:utils",
        "@org_tensorflow//tensorflow/lite/tools/evaluation/proto:evaluation_stages_cc_proto",
    ],
)
