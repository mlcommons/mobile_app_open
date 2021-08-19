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
load("@build_bazel_rules_android//android:rules.bzl", "android_binary", "android_library")
load("@org_tensorflow//tensorflow/lite:build_def.bzl", "tflite_jni_binary")
load("//android/cpp/proto:gen_binary_proto.bzl", "basename", "text_to_bin")
load("@bazel_skylib//rules:common_settings.bzl", "string_flag")

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

string_flag(
    name = "with_qti",
    build_setting_default = "0",
)

config_setting(
    name = "use_qti",
    flag_values = {
        ":with_qti": "1",
    },
)

string_flag(
    name = "with_samsung",
    build_setting_default = "0",
)

config_setting(
    name = "use_samsung",
    flag_values = {
        ":with_samsung": "1",
    },
)

string_flag(
    name = "with_mediatek",
    build_setting_default = "0",
)

config_setting(
    name = "use_mediatek",
    flag_values = {
        ":with_mediatek": "1",
    },
)

java_lite_proto_library(
    name = "mlperf_task_java_proto_lite",
    deps = ["//android/cpp/proto:mlperf_task_proto"],
)

genrule(
    name = "supported_backends",
    srcs = [
        "Backends.java.in",
    ],
    outs = [
        "Backends.java",
    ],
    cmd = "cat $(location Backends.java.in) " +
          select({
              ":use_qti": "| sed -e 's/QTI/\"qti\",/' ",
              "//conditions:default": "| sed -e 's/QTI//' ",
          }) + select({
        ":use_samsung": "| sed -e 's/SAMSUNG/\"samsung\",/' ",
        "//conditions:default": "| sed -e 's/SAMSUNG//'",
    }) + select({
        ":use_mediatek": "| sed -e 's/MEDIATEK/\"tfliteneuron\",/' ",
        "//conditions:default": "| sed -e 's/MEDIATEK//' ",
    }) + " > $(@)",
)

android_library(
    name = "evaluation_app_lib",
    srcs = [
        "AppConstants.java",
        "Benchmark.java",
        "BuildTimeConstants.java",
        "MLCtx.java",
        "MLPerfDriverWrapper.java",
        "MLPerfTasks.java",
        "MiddleInterface.java",
        "NativeEvaluation.java",
        "ResultHolder.java",
        "ResultsAdapter.java",
        "RunMLPerfWorker.java",
        "Util.java",
        "ConfigPrefsUtil.java",
        "supported_backends",
    ] + glob([
        "activities/*.java",
    ]) + glob([
        "adapters/*.java",
    ]) + glob([
        "exceptions/*.java",
    ]) + glob([
        "models/*.java",
    ]) + glob([
        "ui/*.java",
    ]) + glob([
        "activities/settings/*.java",
    ]),
    manifest = "AndroidManifest.xml",
    resource_files = [] + glob(["res/**"]),
    deps = [
        ":evaluation_native",
        ":mlperf_task_java_proto_lite",
        "//android/third_party:androidx_annotation",
        "//android/third_party:androidx_appcompat",
        "//android/third_party:androidx_constraint_layout",
        "//android/third_party:androidx_core",
        "//android/third_party:androidx_material",
        "//android/third_party:androidx_preference",
        "//android/third_party:androidx_recyclerview",
        "//android/third_party:androidx_test_espresso_idling_resource",
    ] + select({
        ":use_mediatek": ["//mobile_back_tflite/cpp/backend_tflite:tflite_neuron_aar"],
        "//conditions:default": [],
    }),
)

android_binary(
    name = "mlperf_app",
    assets = glob([
        "assets/*.tflite",
        "assets/*.txt",
        "assets/*.pbtxt",
        "assets/*.tfrecord",
    ]),
    assets_dir = "assets",
    manifest = "AndroidManifest.xml",
    multidex = "native",
    tags = [
        "instrumentation",
        "no_cuda_on_cpu_tap",
    ],
    deps = [
        ":evaluation_app_lib",
        "//android/commonlibs:commonlibs",
    ] + select({
        ":use_qti": ["//mobile_back_qti:qtibackend"],
        "//conditions:default": [],
    }) + select({
        ":use_samsung": ["//mobile_back_samsung:samsungbackend"],
        "//conditions:default": [],
    }) + select({
        ":use_mediatek": ["//mobile_back_tflite:tfliteneuronbackend"],
        "//conditions:default": [],
    }),
)

filegroup(
    name = "all_files",
    srcs = glob(
        ["**/*"],
        exclude = [
            "**/METADATA",
            "**/OWNERS",
        ],
    ),
)

cc_library(
    name = "evaluation_native",
    srcs = [
        "libmlperf_jni.so",
    ],
    deps = [
        "//mobile_back_tflite:tflitebackend",
    ] + select({
        ":use_samsung": [
            "//mobile_back_samsung:samsungbackend",
        ],
        "//conditions:default": [],
    }) + select({
        ":use_mediatek": ["//mobile_back_tflite:tfliteneuronbackend"],
        "//conditions:default": [],
    }),
)

tflite_jni_binary(
    name = "libmlperf_jni.so",
    deps = [
        "//android/java/org/mlperf/inference/jni:mlperf_jni",
    ],
)
