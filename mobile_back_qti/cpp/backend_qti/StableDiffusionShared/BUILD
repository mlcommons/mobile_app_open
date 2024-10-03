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

load("@stable_diffusion_external_deps_shared//:stable_diffusion_var_def_shared.bzl", "OPENCV_ROOT_DIR")
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
    name = "opencv_core",
    srcs = [
        OPENCV_ROOT_DIR + "/libs/arm64-v8a/libopencv_core.so",
        OPENCV_ROOT_DIR + "/libs/arm64-v8a/libopencv_imgcodecs.so",
    ],
    hdrs = glob([
        OPENCV_ROOT_DIR + "/jni/include/opencv2/*.hpp",
        OPENCV_ROOT_DIR + "/jni/include/opencv2/**/*.hpp",
    ]),
    includes = [OPENCV_ROOT_DIR + "/jni/include/"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "StableDiffusionShared",
    srcs = glob(["libStableDiffusion.so"]),
    hdrs = glob([
        "include/*.hpp",
        "include/**/*.hpp",
        "include/**/*.h",
    ]),
    visibility = ["//visibility:public"],
    deps = [
        ":opencv_core",
    ],
)
