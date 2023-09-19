workspace(name = "mlperf_app")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bazel_skylib",
    sha256 = "66ffd9315665bfaafc96b52278f57c7e2dd09f5ede279ea6d39b2be471e7e3aa",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.4.2/bazel-skylib-1.4.2.tar.gz",
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.4.2/bazel-skylib-1.4.2.tar.gz",
    ],
)

load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")

bazel_skylib_workspace()

load("//:platform.bzl", "tf_patch_finder")

tf_patch_finder(
    name = "tf_patch_finder",
    workspace_dir = __workspace_dir__,
)

load("@tf_patch_finder//:patch_win_arm64.bzl", "PATCH_FILE")

http_archive(
    name = "org_tensorflow",
    patch_args = ["-p1"],
    patches = [
        # Add patches for adding png in tflite evaluation code
        "//:flutter/third_party/enable-png-in-tensorflow-lite-tools-evaluation.patch",
        "//:flutter/third_party/png-with-number-of-channels-detected.patch",
        "//:flutter/third_party/use_unsigned_char.patch",
        # Fix tensorflow not being able to read image files on Windows
        "//:flutter/third_party/tensorflow-fix-file-opening-mode-for-Windows.patch",
        "//:flutter/third_party/tf-eigen.patch",
	# NDK 25 support
        "//patches:ndk_25_r13.diff",
    ] + PATCH_FILE,
    sha256 = "e58c939079588623e6fa1d054aec2f90f95018266e0a970fd353a5244f5173dc",
    strip_prefix = "tensorflow-2.13.0",
    urls = [
        "https://github.com/tensorflow/tensorflow/archive/v2.13.0.tar.gz",
    ],
)

# Initialize tensorflow workspace.
# Must be after apple dependencies
# because it loads older version of build_bazel_rules_apple
load("@org_tensorflow//tensorflow:workspace3.bzl", "tf_workspace3")

tf_workspace3()

load("@org_tensorflow//tensorflow:workspace2.bzl", "tf_workspace2")

tf_workspace2()

load("@org_tensorflow//tensorflow:workspace1.bzl", "tf_workspace1")

tf_workspace1()

load("@org_tensorflow//tensorflow:workspace0.bzl", "tf_workspace0")

tf_workspace0()

http_archive(
    name = "neuron_delegate",
    sha256 = "85c1b17acb34072baa12cf74011ff45aee9045a12603851b86ce85e3cef66747",
    strip_prefix = "tflite-neuron-delegate-update_for_dujac",
    urls = ["https://github.com/MediaTek-NeuroPilot/tflite-neuron-delegate/archive/refs/heads/update_for_dujac.zip"],
)

http_archive(
    name = "org_mlperf_inference",
    build_file = "@//flutter/android/third_party:loadgen.BUILD",
    patch_args = ["-p1"],
    patch_cmds = ["python3 loadgen/version_generator.py loadgen/version_generated.cc loadgen"],
    patches = [],
    sha256 = "e664f980e84fcab3573447c0cc3adddd1fcf900367c5dcbff17179ece24c484e",
    strip_prefix = "inference-2da0c52666e21e4b296b09e1dbd287bf3a814e96",
    urls = [
        "https://github.com/mlcommons/inference/archive/2da0c52666e21e4b296b09e1dbd287bf3a814e96.tar.gz",
    ],
)

# This is required to pass SNPE SDK path from external environment to sources,
# without actually modifying files
load("//mobile_back_qti:variables.bzl", "snpe_version_loader")

snpe_version_loader(
    name = "snpe_version_loader",
    workspace_dir = __workspace_dir__,
)
