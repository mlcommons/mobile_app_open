# Copyright (c) 2020-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
##########################################################################

load("@snpe_version_loader//:snpe_var_def.bzl", "SNPE_VERSION")

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

config_setting(
    name = "android_arm64",
    values = {"cpu": "arm64-v8a"},
)

cc_import(
    name = "snpewindowslib",
    hdrs = glob([
        SNPE_VERSION + "/include/SNPE/**/*.hpp",
        SNPE_VERSION + "/include/SNPE/**/*.h",
    ]),
    interface_library = SNPE_VERSION + "/lib/aarch64-windows-msvc/SNPE.lib",
    shared_library = SNPE_VERSION + "/lib/aarch64-windows-msvc/SNPE.dll",
)

cc_library(
    name = "snpe_depsdll",
    srcs = [
        SNPE_VERSION + "/lib/aarch64-windows-msvc/SNPE.dll",
        SNPE_VERSION + "/lib/aarch64-windows-msvc/SnpeHtpV68Stub.dll",
        SNPE_VERSION + "/lib/hexagon-v68/unsigned/libSnpeHtpV68Skel.so",
    ],
    linkstatic = 0,
    deps = [":snpewindowslib"],
)

cc_library(
    name = "snpe",
    srcs = [
        SNPE_VERSION + "/lib/aarch64-android/libSNPE.so",
    ],
    hdrs = glob([
        SNPE_VERSION + "/include/SNPE/**/*.hpp",
        SNPE_VERSION + "/include/SNPE/**/*.h",
    ]),
    copts = ["-I" + SNPE_VERSION + "/include/SNPE"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "snpe_deps",
    srcs = [
        SNPE_VERSION + "/lib/aarch64-android/libhta.so",
        SNPE_VERSION + "/lib/aarch64-android/libSnpeHtpV75Stub.so",
        SNPE_VERSION + "/lib/aarch64-android/libSnpeHtpV73Stub.so",
        SNPE_VERSION + "/lib/aarch64-android/libSnpeHtpV69Stub.so",
        SNPE_VERSION + "/lib/aarch64-android/libSnpeHtpV68Stub.so",
        SNPE_VERSION + "/lib/aarch64-android/libSNPE.so",
        SNPE_VERSION + "/lib/aarch64-android/libSnpeHtpPrepare.so",
        SNPE_VERSION + "/lib/hexagon-v68/unsigned/libSnpeHtpV68Skel.so",
        SNPE_VERSION + "/lib/hexagon-v69/unsigned/libSnpeHtpV69Skel.so",
        SNPE_VERSION + "/lib/hexagon-v73/unsigned/libSnpeHtpV73Skel.so",
        SNPE_VERSION + "/lib/hexagon-v75/unsigned/libSnpeHtpV75Skel.so",
    ],
)

cc_library(
    name = "qtibackend",
    srcs = select({
        "//conditions:default": [
            "//mobile_back_qti/cpp/backend_mock_qti:libqtibackend.so",
        ],
        ":android_arm64": [
            "//mobile_back_qti/cpp/backend_qti:libqtibackend.so",
        ],
    }),
    deps = select({
        "//conditions:default": [],
        ":android_arm64": [
            "snpe",
            "snpe_deps",
        ],
    }),
)

cc_library(
    name = "qtibackenddll",
    srcs = select({
        "//conditions:default": [
            "//mobile_back_qti/cpp/backend_mock_qti:libqtibackend.dll",
        ],
        ":android_arm64": [
            "//mobile_back_qti/cpp/backend_qti:libqtibackend.dll",
        ],
    }),
    linkstatic = 0,
    deps = [
        ":snpe_depsdll",
        ":snpewindowslib",
    ],
)
