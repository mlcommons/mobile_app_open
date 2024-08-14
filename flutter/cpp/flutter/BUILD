load("@build_bazel_rules_apple//apple:apple.bzl", "apple_xcframework")
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
        "dart_cpuinfo.cc",
        "dart_mlperf_config.cc",
        "dart_run_benchmark.cc",
    ],
    hdrs = [
        "dart_backend_match.h",
        "dart_cpuinfo.h",
        "dart_mlperf_config.h",
        "dart_run_benchmark.h",
    ],
    deps = [
        "//flutter/cpp:mlperf_driver",
        "//flutter/cpp/backends:external",
        "//flutter/cpp/datasets:ade20k",
        "//flutter/cpp/datasets:coco",
        "//flutter/cpp/datasets:imagenet",
        "//flutter/cpp/datasets:snu_sr",
        "//flutter/cpp/datasets:squad",
        "//flutter/cpp/datasets:coco_gen",
        "//flutter/cpp/proto:mlperf_task_cc_proto",
        "@cpuinfo",
    ],
    alwayslink = 1,
)

apple_xcframework(
    name = "backend_bridge_fw",
    bundle_id = "com.mlcommons.inference.backend-bridge",
    exported_symbols_lists = ["//flutter/cpp/flutter:exported_symbols.lds"],
    infoplists = ["//flutter/cpp/flutter:BackendBridgeInfo.plist"],
    ios = {
        "simulator": [
            "x86_64",
            # cpuinfo does not support simulator on ARM-based macs
            # "ios_sim_arm64",
        ],
        "device": ["arm64"],
    },
    minimum_os_versions = {
        "ios": "12.0",
    },
    deps = [
        "//flutter/cpp/flutter:bridge",
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
    linkscript = "//flutter/cpp/flutter:version_script.lds",
    deps = [
        "//flutter/cpp/flutter:bridge",
    ],
)
