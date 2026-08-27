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

load("@build_bazel_rules_apple//apple:apple.bzl", "apple_xcframework")

apple_xcframework(
    name = "liblitertbackend",
    bundle_id = "com.mlcommons.inference.backend-litert",
    # Not //flutter/cpp/c:exported_symbols.lds — this backend also exports the
    # LiteRt* symbols, mirroring the Android version script. See that file for
    # why they are kept even though the 2.1.5 Metal accelerator does not need
    # them.
    exported_symbols_lists = ["//mobile_back_litert/cpp/backend_litert:exported_symbols_litert.lds"],
    infoplists = ["//flutter/cpp/flutter:BackendBridgeInfo.plist"],
    ios = {
        "simulator": [
            "x86_64",
            # cpuinfo doesn't support simulator on ARM-based macs
            # "arm64",
        ],
        "device": ["arm64"],
    },
    # 13.1 like the sibling backends is not enough: litert/cc/BUILD sets
    # LITERT_MIN_IOS_VERSION = "14.0", so anything linking the LiteRT runtime
    # has to declare at least that.
    minimum_os_versions = {
        "ios": "14.0",
        "macos": "14.0",
    },
    deps = [
        "//mobile_back_litert/cpp/backend_litert:litert_c",
    ],
)
