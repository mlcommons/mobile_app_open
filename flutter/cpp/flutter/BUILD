load("@build_bazel_rules_apple//apple:ios.bzl", "ios_static_framework")
load("@org_tensorflow//tensorflow/lite:build_def.bzl", "tflite_jni_binary")

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

config_setting(
    name = "arm_build",
    values = {"cpu": "ios_arm64"},
)

cc_library(
    name = "bridge",
    srcs = [
        "dart_backend_match.cc",
        "dart_mlperf_config.cc",
        "dart_run_benchmark.cc",
    ],
    hdrs = [
        "dart_backend_match.h",
        "dart_mlperf_config.h",
        "dart_run_benchmark.h",
    ],
    deps = [
        "//flutter/cpp:mlperf_driver",
        "//flutter/cpp/backends:external",
        "//flutter/cpp/datasets:ade20k",
        "//flutter/cpp/datasets:coco",
        "//flutter/cpp/datasets:imagenet",
        "//flutter/cpp/datasets:squad",
        "//flutter/cpp/proto:mlperf_task_cc_proto",
    ],
    alwayslink = 1,
)

cc_library(
    name = "ios_backend",
    srcs = ["ios_linker_workaround.cc"],
    hdrs = ["ios_linker_workaround.h"],
    deps = [
        "//flutter/cpp/flutter:bridge",
        "//mobile_back_tflite/cpp/backend_tflite:tflite_c",
    ],
)

ios_static_framework(
    name = "ios_backend_fw_static",
    families = [
        "iphone",
        "ipad",
    ],
    ipa_post_processor = "cp_framework_to_tmp.sh",
    minimum_os_version = "11.0",
    deps = [
        "//flutter/cpp/flutter:ios_backend",
    ],
)

cc_binary(
    name = "backend_bridge.dll",
    linkshared = 1,
    win_def_file = "dll_export.def",
    deps = [
        "//flutter/cpp/flutter:bridge",
    ],
)

tflite_jni_binary(
    name = "libbackendbridge.so",
    exported_symbols = "//flutter/cpp/flutter:exported_symbols.lds",
    linkscript = "//flutter/cpp/flutter:version_script.lds",
    deps = [
        "//flutter/cpp/flutter:bridge",
    ],
)
