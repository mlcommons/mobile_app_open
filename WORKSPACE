workspace(name = "mlperf_app")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_google_protobuf",
    sha256 = "528927e398f4e290001886894dac17c5c6a2e5548f3fb68004cfb01af901b53a",
    strip_prefix = "protobuf-3.17.3",
    urls = ["https://github.com/google/protobuf/archive/v3.17.3.zip"],
)

http_archive(
    name = "build_bazel_rules_apple",
    sha256 = "36072d4f3614d309d6a703da0dfe48684ec4c65a89611aeb9590b45af7a3e592",
    url = "https://github.com/bazelbuild/rules_apple/releases/download/1.0.1/rules_apple.1.0.1.tar.gz",
)

load("@build_bazel_rules_apple//apple:repositories.bzl", "apple_rules_dependencies")

apple_rules_dependencies()

load("@build_bazel_apple_support//lib:repositories.bzl", "apple_support_dependencies")

apple_support_dependencies()

# This lib must be named exactly "cpuinfo".
# This name is used by org_tensorflow lib.
# When we use any different name, compilation may fail
# because there will be files from several different versions of cpuinfo.
# We may also need to override clog dependency, which uses the same sources, if we encounter any similar errors.
http_archive(
    name = "cpuinfo",
    patch_args = ["-p1"],
    patches = [
        "//patches:cpuinfo-bazel-patch.diff",
    ],
    sha256 = "3389494589a97122779cd8d57fbffb1ac1e1ca3e795981c1d8d71b92281ae8c4",
    strip_prefix = "cpuinfo-8ec7bd91ad0470e61cf38f618cc1f270dede599c",
    url = "https://github.com/pytorch/cpuinfo/archive/8ec7bd91ad0470e61cf38f618cc1f270dede599c.tar.gz",
)

http_archive(
    name = "org_tensorflow",
    patch_args = ["-p1"],
    patches = [
        # Fix tensorflow not being able to read image files on Windows
        "//:flutter/third_party/tensorflow-fix-file-opening-mode-for-Windows.patch",
        "//:flutter/third_party/tf-eigen.patch",
        # fix memory leak in coreml delegate
        "//:flutter/third_party/tflite_coreml_delegate_memory_leak.patch",
        "//:flutter/third_party/tensorflow-fix-llvm.patch",
        "//patches:feature_level.diff",
    ],
    sha256 = "d2948c066a0bc3f45cb8072def03c85f50af8a75606bbdff91715ef8c5f2a28c",
    strip_prefix = "tensorflow-2.8.0",
    urls = [
        "https://github.com/tensorflow/tensorflow/archive/v2.8.0.zip",
    ],
)

# Initialize tensorflow workspace.
# Must be after apple dependencies
# because it loads older version of build_bazel_rules_apple
load("@org_tensorflow//tensorflow:workspace3.bzl", "tf_workspace3")

tf_workspace3()

load("@org_tensorflow//tensorflow:workspace2.bzl", "tf_workspace2")

tf_workspace2()

# Android.
load("@//flutter/third_party/android:android_configure.bzl", "android_configure")

android_configure(name = "local_config_android")

load("@local_config_android//:android_configure.bzl", "android_workspace")

android_workspace()

# avoid using android_{sdk,ndk}_repo because of bazel 5.0
#
#android_sdk_repository(
#    name = "androidsdk",
#    api_level = 30,
#)
#
#android_ndk_repository(
#    name = "androidndk",
#)

http_archive(
    name = "neuron_delegate",
    sha256 = "85c1b17acb34072baa12cf74011ff45aee9045a12603851b86ce85e3cef66747",
    strip_prefix = "tflite-neuron-delegate-update_for_dujac",
    urls = ["https://github.com/MediaTek-NeuroPilot/tflite-neuron-delegate/archive/refs/heads/update_for_dujac.zip"],
)

new_local_repository(
    name = "samsungbackend",
    build_file = "samsung_backend/BUILD",
    path = "mobile_back_samsung",
)

http_archive(
    name = "org_mlperf_inference",
    build_file = "@//flutter/android/third_party:loadgen.BUILD",
    patch_args = ["-p1"],
    patch_cmds = ["python3 loadgen/version_generator.py loadgen/version_generated.cc loadgen"],
    patches = [],
    sha256 = "40d0123c8447a0bdd4d5c8fda5eea22c7b08f3b05501858d2ba89be4f44fc84b",
    strip_prefix = "inference-f5367250115ad4febf1334b34881ab74f2e55bfe",
    urls = [
        "https://github.com/mlcommons/inference/archive/f5367250115ad4febf1334b34881ab74f2e55bfe.tar.gz",
    ],
)

# This is required to pass SNPE SDK path from external environment to sources,
# without actually modifying files
load("//mobile_back_qti:variables.bzl", "snpe_version_loader")

snpe_version_loader(
    name = "snpe_version_loader",
    workspace_dir = __workspace_dir__,
)
