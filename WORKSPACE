workspace(name = "mlperf_app")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bazel_skylib",
    sha256 = "66ffd9315665bfaafc96b52278f57c7e2dd09f5ede279ea6d39b2be471e7e3aa",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.4.2/bazel-skylib-1.4.2.tar.gz",
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.4.2/bazel-skylib-1.4.2.tar.gz",
    ],
)

http_archive(
    name = "rules_python",
    sha256 = "5868e73107a8e85d8f323806e60cad7283f34b32163ea6ff1020cf27abef6036",
    strip_prefix = "rules_python-0.25.0",
    url = "https://github.com/bazelbuild/rules_python/releases/download/0.25.0/rules_python-0.25.0.tar.gz",
)

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
        #"//:flutter/third_party/tf-eigen.patch",
    ] + PATCH_FILE,
    sha256 = "d7876f4bb0235cac60eb6316392a7c48676729860da1ab659fb440379ad5186d",
    strip_prefix = "tensorflow-2.18.0",
    urls = [
        "https://github.com/tensorflow/tensorflow/archive/v2.18.0.tar.gz",
    ],
)

load("@org_tensorflow//third_party/gpus:cuda_configure.bzl", "cuda_configure")
cuda_configure(name = "local_config_cuda")

load("@org_tensorflow//third_party/gpus:rocm_configure.bzl", "rocm_configure")
rocm_configure(name = "local_config_rocm")

http_archive(
    name = "com_google_sentencepiece",
    strip_prefix = "sentencepiece-0.1.96",
    sha256 = "8409b0126ebd62b256c685d5757150cf7fcb2b92a2f2b98efb3f38fc36719754",
    urls = [
        "https://github.com/google/sentencepiece/archive/refs/tags/v0.1.96.zip"
    ],
    build_file = "@//patches:sentencepiece.BUILD",
    patches = ["@//patches:com_google_sentencepiece.diff"],
    patch_args = ["-p1"],
)

http_archive(
    name = "darts_clone",
    sha256 = "c97f55d05c98da6fcaf7f9ecc6a6dc6bc5b18b8564465f77abff8879d446491c",
    strip_prefix = "darts-clone-e40ce4627526985a7767444b6ed6893ab6ff8983",
    urls = [
        "https://github.com/s-yata/darts-clone/archive/e40ce4627526985a7767444b6ed6893ab6ff8983.zip",
    ],
    build_file = "@//patches:darts_clone.BUILD",
    patches = ["//patches:darts_no_exceptions.diff"],
    patch_args = ["-p0"],
)

load(
    "@org_tensorflow//tensorflow/tools/toolchains/python:python_repo.bzl",
    "python_repository",
)
load("@rules_python//python:repositories.bzl", "python_register_toolchains")

python_repository(name = "python_version_repo")

load("@python_version_repo//:py_version.bzl", "HERMETIC_PYTHON_VERSION")

python_register_toolchains(
    name = "python",
    ignore_root_user_error = True,
    python_version = HERMETIC_PYTHON_VERSION,
)

# Initialize tensorflow workspace.
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
    sha256 = "7918cc54a2bab63c30eb87a90de8ce3f3730b5572e0269a2b57a0c9bcd28cd69",
    strip_prefix = "tflite-neuron-delegate-update_for_leroy",
    urls = ["https://github.com/MediaTek-NeuroPilot/tflite-neuron-delegate/archive/refs/heads/update_for_leroy.zip"],
)

new_git_repository(
    name = "org_mlperf_inference",
    build_file = "@//flutter/android/third_party:loadgen.BUILD",
    commit = "238d035ab41d7ddd390b35471af169ea641380f6",
    patch_args = ["-p1"],
    patch_cmds = ["python3 loadgen/version_generator.py loadgen/version_generated.cc loadgen"],
    patches = [],
    remote = "https://github.com/mlcommons/inference.git",
)

# This is required to pass SNPE SDK path from external environment to sources,
# without actually modifying files
load("//mobile_back_qti:variables.bzl", "snpe_version_loader")

snpe_version_loader(
    name = "snpe_version_loader",
    workspace_dir = __workspace_dir__,
)

load("//mobile_back_qti/cpp/backend_qti/StableDiffusionShared:variables.bzl", "stable_diffusion_external_deps_shared")

stable_diffusion_external_deps_shared(
    name = "stable_diffusion_external_deps_shared",
    workspace_dir = __workspace_dir__,
)
