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
    sha256 = "2bef00cc7ba6f649a437e979fe1b36bdd94f575b5c00e0d08cb4e92e586f49a7",
    strip_prefix = "tflite-neuron-delegate-2.8.0",
    urls = ["https://github.com/MediaTek-NeuroPilot/tflite-neuron-delegate/archive/refs/tags/v2.8.0.tar.gz"],
)

new_local_repository(
    name = "samsungbackend",
    build_file = "samsung_backend/BUILD",
    path = "mobile_back_samsung",
)

http_archive(
    name = "org_mlperf_inference",
    build_file = "@//flutter/android/third_party:loadgen.BUILD",
    patch_cmds = ["python3 loadgen/version_generator.py loadgen/version_generated.cc loadgen"],
    sha256 = "f4c57a3f3cd71f2dac166a79ad760b824aafda7b91400889acff4a9c7dbdaf8e",
    strip_prefix = "inference-a77ac37d07145d9f3123465a8fd18f9ebbde5d6a",
    urls = [
        "https://github.com/mlcommons/inference/archive/a77ac37d07145d9f3123465a8fd18f9ebbde5d6a.tar.gz",
    ],
)

# This is required to pass SNPE SDK path from external environment to sources,
# without actually modifying files
load("//mobile_back_qti:variables.bzl", "snpe_version_loader")

snpe_version_loader(
    name = "snpe_version_loader",
    workspace_dir = __workspace_dir__,
)
