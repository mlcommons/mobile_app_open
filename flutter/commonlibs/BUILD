# Copyright 2021 The MLPerf Authors. All Rights Reserved.
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
load("@bazel_skylib//rules:common_settings.bzl", "string_flag")

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

genrule(
    name = "asan_arm64",
    srcs = [
        "@androidndk//:default_crosstool",
    ],
    outs = [
        "lib_arm64/libclang_rt.asan-aarch64-android.so",
    ],
    cmd = """\
cp `find . -name libclang_rt.asan-aarch64-android.so` $(@)
    """,
)

genrule(
    name = "asan_x86_64",
    srcs = [
        "@androidndk//:default_crosstool",
    ],
    outs = [
        "lib_x86_64/libclang_rt.asan-x86_64-android.so",
    ],
    cmd = """\
cp `find . -name libclang_rt.asan-x86_64-android.so` $(@)
    """,
)

genrule(
    name = "libcpp_shared_arm64",
    srcs = [
        "@androidndk//:default_crosstool",
    ],
    outs = [
        "lib_arm64/libc++_shared.so",
    ],
    cmd = """\
cp `find . -name libc++_shared.so | grep arm64-v8a` $(@)
    """,
)

config_setting(
    name = "android_arm64",
    values = {"cpu": "arm64-v8a"},
)

config_setting(
    name = "android_arm",
    values = {"cpu": "armeabi-v7a"},
)

config_setting(
    name = "android_x86_64",
    values = {"cpu": "x86_64"},
)

string_flag(
    name = "with_asan",
    build_setting_default = "0",
)

config_setting(
    name = "use_asan",
    flag_values = {
        ":with_asan": "1",
    },
)

cc_library(
    name = "asan_lib",
    srcs = select({
        "android_arm64": [
            "lib_arm64/libclang_rt.asan-aarch64-android.so",
        ],
        "android_arm": [
            "lib_arm/libclang_rt.asan-arm-android.so",
        ],
        "android_x86_64": [
            "lib_x86_64/libclang_rt.asan-x86_64-android.so",
        ],
        "//conditions:default": [],
    }),
    data = select({
        "android_arm64": [
            "//flutter/commonlibs:asan_arm64",
        ],
        "android_x86_64": [
            "//flutter/commonlibs:asan_x86_64",
        ],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "commonlibs",
    srcs = select({
        "android_arm64": [
            "lib_arm64/libc++_shared.so",
        ],
        "//conditions:default": [],
    }),
    data = select({
        "android_arm64": [
            "//flutter/commonlibs:libcpp_shared_arm64",
        ],
        "//conditions:default": [],
    }),
    deps = select({
        ":use_asan": [
            "//flutter/commonlibs:asan_lib",
        ],
        "//conditions:default": [],
    }),
)
