load("@build_bazel_rules_apple//apple:apple.bzl", "apple_xcframework")

apple_xcframework(
    name = "liblitertbackend",
    bundle_id = "com.mlcommons.inference.backend-litert",
    exported_symbols_lists = ["//flutter/cpp/c:exported_symbols.lds"],
    infoplists = ["//flutter/cpp/flutter:BackendBridgeInfo.plist"],
    ios = {
        "simulator": [
            "x86_64",
            # cpuinfo doesn't support simulator on ARM-based macs
            # "arm64",
        ],
        "device": ["arm64"],
    },
    minimum_os_versions = {
        "ios": "13.1",
        "macos": "13.1",
    },
    deps = [
        "//mobile_back_litert/cpp/backend_litert:litert_c",
    ],
)
