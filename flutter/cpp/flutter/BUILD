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
        "main.cc",
    ],
    hdrs = ["main.h"],
    deps = [
        "//cpp:mlperf_driver",
        "//cpp/backends:external",
        "//cpp/datasets:ade20k",
        "//cpp/datasets:coco",
        "//cpp/datasets:imagenet",
        "//cpp/datasets:squad",
        "//cpp/proto:mlperf_task_cc_proto",
    ],
    alwayslink = 1,
)

ios_static_framework(
    name = "ios_backend_fw_static",
    hdrs = ["main.h"],
    families = [
        "iphone",
        "ipad",
    ],
    minimum_os_version = "11.0",
    deps = [
        "//cpp/backend_tflite:tflite_c",
        "//cpp/flutter:bridge",
    ],
)

cc_binary(
    name = "backend_bridge.dll",
    linkshared = 1,
    win_def_file = "dll_export.def",
    deps = [
        "//cpp/flutter:bridge",
    ],
)

tflite_jni_binary(
    name = "libbackendbridge.so",
    exported_symbols = "//cpp/flutter:exported_symbols.lds",
    linkscript = "//cpp/flutter:version_script.lds",
    deps = [
        "//cpp/flutter:bridge",
    ],
)
