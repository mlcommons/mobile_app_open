# Copyright (c) 2020-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

cc_library(
    name = "snpe",
    srcs = [
        SNPE_VERSION + "/lib/aarch64-android-clang8.0/libSNPE.so",
        SNPE_VERSION + "/lib/aarch64-android-clang8.0/libhta.so",
    ],
    hdrs = glob([
        SNPE_VERSION + "/include/zdl/**/*.hpp",
        SNPE_VERSION + "/include/zdl/**/*.h",
    ]),
    copts = ["-I" + SNPE_VERSION + "/include/zdl"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "snpe_deps",
    srcs = [
        SNPE_VERSION + "/lib/aarch64-android-clang8.0/libhta.so",
        SNPE_VERSION + "/lib/aarch64-android-clang8.0/libsnpe_dsp_domains_v2.so",
        SNPE_VERSION + "/lib/aarch64-android-clang8.0/libSnpeHtpV69Stub.so",
        SNPE_VERSION + "/lib/aarch64-android-clang8.0/libSnpeHtpV68Stub.so",
        SNPE_VERSION + "/lib/aarch64-android-clang8.0/libSNPE.so",
        SNPE_VERSION + "/lib/aarch64-android-clang8.0/libSnpeHtpPrepare.so",
        SNPE_VERSION + "/lib/dsp/libsnpe_dsp_v66_domains_v2_skel.so",
        SNPE_VERSION + "/lib/dsp/libSnpeHtpV69Skel.so",
        SNPE_VERSION + "/lib/dsp/libSnpeHtpV68Skel.so",
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
