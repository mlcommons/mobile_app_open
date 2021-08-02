# Copyright 2019 The MLPerf Authors. All Rights Reserved.
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
load("@rules_jvm_external//:defs.bzl", "artifact")

licenses(["notice"])  # Apache 2.0

android_library(
    name = "mlperf_test_lib",
    srcs = glob(["org/mlperf/inference/*.java"]),
    custom_package = "org.mlperf.inference.test",
    deps = [
        ":test_deps",
        "//java/org/mlperf/inference:evaluation_app_lib",
        "//third_party:androidx_preference",
        "//third_party:androidx_recyclerview",
    ],
)

android_binary(
    name = "mlperf_test_app",
    custom_package = "org.mlperf.inference.test",
    instruments = "//java/org/mlperf/inference:mlperf_app",
    manifest = "org/mlperf/inference/AndroidManifest.xml",
    deps = [":mlperf_test_lib"],
)

android_library(
    name = "test_deps",
    visibility = ["//visibility:public"],
    exports = [
        artifact("androidx.annotation:annotation"),
        artifact("androidx.test.espresso:espresso-core"),
        artifact("androidx.test.espresso:espresso-idling-resource"),
        artifact("androidx.test:rules"),
        artifact("androidx.test:runner"),
        artifact("androidx.test:monitor"),
        artifact("androidx.test.ext:junit"),
        artifact("androidx.test:core"),
        artifact("com.google.guava:guava"),
        artifact("com.google.inject:guice"),
        artifact("javax.inject:javax.inject"),
        artifact("junit:junit"),
        artifact("org.hamcrest:java-hamcrest"),
    ],
)
