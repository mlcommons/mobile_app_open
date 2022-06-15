workspace(name = "mlperf_app")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_google_protobuf",
    sha256 = "528927e398f4e290001886894dac17c5c6a2e5548f3fb68004cfb01af901b53a",
    strip_prefix = "protobuf-3.17.3",
    urls = ["https://github.com/google/protobuf/archive/v3.17.3.zip"],
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
    ],
    sha256 = "d2948c066a0bc3f45cb8072def03c85f50af8a75606bbdff91715ef8c5f2a28c",
    strip_prefix = "tensorflow-2.8.0",
    urls = [
        "https://github.com/tensorflow/tensorflow/archive/v2.8.0.zip",
    ],
)

# Initialize tensorflow workspace.
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

# use Neuron Delegate aar before we have updated source code
#http_archive(
#    name = "neuron_delegate",
#    sha256 = "2e4600c99c9b4ea7a129108cd688419eeef9b2aeabf05df6f385258e19ca96c4",
#    strip_prefix = "tflite-neuron-delegate-2.6.0",
#    urls = ["https://github.com/MediaTek-NeuroPilot/tflite-neuron-delegate/archive/v2.6.0.tar.gz"],
#)

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

http_archive(
    name = "build_bazel_rules_apple",
    sha256 = "9f9eb6cdd25d7932cb939df24807c2d70772aad7a79f1357e25ced9d0d443cfd",
    strip_prefix = "rules_apple-0.19.0",
    urls = [
        "https://github.com/bazelbuild/rules_apple/archive/refs/tags/0.19.0.zip",
    ],
)

http_archive(
    name = "build_bazel_rules_swift",
    sha256 = "ef728d0d99276d62b2393c350f29f176a6f38a925f2d12c37c4ed64f6906c2f5",
    strip_prefix = "rules_swift-0.13.0",
    urls = [
        "https://github.com/bazelbuild/rules_swift/archive/refs/tags/0.13.0.zip",
    ],
)

http_archive(
    name = "build_bazel_apple_support",
    sha256 = "249be3d90bc4211928a5260c4bc5792a236c58d1b6183c0e30f58db8710fc952",
    strip_prefix = "apple_support-0.7.2",
    urls = [
        "https://github.com/bazelbuild/apple_support/archive/refs/tags/0.7.2.zip",
    ],
)

# This is required to pass SNPE SDK path from external environment to sources,
# without actually modifying files
load("//mobile_back_qti:variables.bzl", "snpe_version_loader")

snpe_version_loader(
    name = "snpe_version_loader",
    workspace_dir = __workspace_dir__,
)
