load("@build_bazel_rules_apple//apple:apple.bzl", "apple_xcframework")

apple_xcframework(
    name = "libtflitebackend",
    bundle_id = "com.mlcommons.inference.backend-tflite",
    exported_symbols_lists = ["//flutter/cpp/c:exported_symbols.lds"],
    infoplists = ["//flutter/cpp/flutter:BackendBridgeInfo.plist"],
    ios = {
        "simulator": [
            "x86_64",
            # cpuinfo doesn't support simulator on ARM-based macs
            # "ios_sim_arm64",
        ],
        "device": ["arm64"],
    },
    minimum_os_versions = {
        "ios": "13.1",
        "macos": "12.1",
    },
    deps = [
        "//mobile_back_tflite/cpp/backend_tflite:tflite_c",
    ],
)
