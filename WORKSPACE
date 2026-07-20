workspace(name = "mlperf_app")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bazel_skylib",
    #sha256 = "66ffd9315665bfaafc96b52278f57c7e2dd09f5ede279ea6d39b2be471e7e3aa",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.7.1/bazel-skylib-1.7.1.tar.gz",
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.7.1/bazel-skylib-1.7.1.tar.gz",
    ],
)


http_archive(
    name = "rules_platform",
    sha256 = "0aadd1bd350091aa1f9b6f2fbcac8cd98201476289454e475b28801ecf85d3fd",
    urls = [
        "https://github.com/bazelbuild/rules_platform/releases/download/0.1.0/rules_platform-0.1.0.tar.gz",
    ],
)

# rules_python is intentionally NOT declared here.
# XLA's python_init_rules() brings in a compatible version automatically.
# Declaring rules_python 0.25.0 here would win over XLA's version and break
# hermetic Python initialization (missing python_version_kind attribute).

load("//:platform.bzl", "tf_patch_finder")

tf_patch_finder(
    name = "tf_patch_finder",
    workspace_dir = __workspace_dir__,
)

# Override zlib and png version to make it compatible with Xcode 16.4
http_archive(
    name = "zlib",
    build_file = "//third_party:zlib.BUILD",
    #sha256 = "17e88863f3600672ab49182f217281b6fc4d3c762bde361935e436a95214d05c",
    strip_prefix = "zlib-1.3.1",
    urls = [
        "https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz",
        "https://github.com/madler/zlib/archive/refs/tags/v1.3.1.tar.gz",
    ],
)

http_archive(
    name = "png",
    build_file = "//third_party:libpng.BUILD",
    sha256 = "71158e53cfdf2877bc99bcab33641d78df3f48e6e0daad030afe9cb8c031aa46",
    strip_prefix = "libpng-1.6.50",
    urls = ["https://github.com/glennrp/libpng/archive/refs/tags/v1.6.50.tar.gz"],
)

load("@tf_patch_finder//:patch_win_arm64.bzl", "PATCH_FILE")

http_archive(
    name = "FP16",
    build_file = "@//third_party:FP16.BUILD",
    patch_args = ["-p1"],
    patches = ["//patches:fp16_math_workaround.patch"],
    sha256 = "e66e65515fa09927b348d3d584c68be4215cfe664100d01c9dbc7655a5716d70",
    strip_prefix = "FP16-0a92994d729ff76a58f692d3028ca1b64b145d91",
    urls = [
        "https://github.com/Maratyszcza/FP16/archive/0a92994d729ff76a58f692d3028ca1b64b145d91.zip",
    ],
)

http_archive(
    name = "cld2",
    build_file = "@//third_party:cld2.BUILD",
    sha256 = "6d8681eadbb64d8fcd3c6c620e81bed16d99ff75627324d66ee2d54bf2b7d749",
    strip_prefix = "cld2-b56fa78a2fe44ac2851bae5bf4f4693a0644da7b",
    urls = [
        "https://github.com/CLD2Owners/cld2/archive/b56fa78a2fe44ac2851bae5bf4f4693a0644da7b.zip",
    ],
)

http_archive(
    name = "litert",
    patch_args = ["-p1"],
    patches = [
        # Add patches for adding png in tflite evaluation code
        "//:flutter/third_party/enable-png-in-tensorflow-lite-tools-evaluation.patch",
        "//:flutter/third_party/png-with-number-of-channels-detected.patch",
        "//:flutter/third_party/use_unsigned_char.patch",
        # Fix tensorflow not being able to read image files on Windows
        "//:flutter/third_party/tensorflow-fix-file-opening-mode-for-Windows.patch",
        "//:patches/litert-internal-visibility.diff",
    ],
    sha256 = "7d0313c4851deb18af6f5f2dbc002bf01293583b87b819b0949ee33dcfe2d91b",
    strip_prefix = "LiteRT-2.1.5",
    urls = [
        "https://github.com/google-ai-edge/LiteRT/archive/v2.1.5.tar.gz",
    ],
)

load("//third_party:tensorflow_source_rules.bzl", "tensorflow_source_repo")

tensorflow_source_repo(
    name = "org_tensorflow",
    patch_args = ["-p1"],
    patches = [
        "//:flutter/third_party/tf-eigen.patch",
    ] + PATCH_FILE,
    patch_cmds = [],
    patch_scripts = [
        "//third_party:fix_tensorflow.py",
    ],
    sha256 = "879cf25692d50c60315a4dd3929dccd923d4c44a2c4b95ebb483666d2c16a22a",
    strip_prefix = "tensorflow-6d40c20cdfe385746c31da6227b95722f5ece342",
    urls = [
        "https://github.com/tensorflow/tensorflow/archive/6d40c20cdfe385746c31da6227b95722f5ece342.tar.gz",
    ],
)

http_archive(
    name = "com_google_sentencepiece",
    build_file = "@//third_party:sentencepiece.BUILD",
    patch_args = ["-p1"],
    patches = ["@//patches:com_google_sentencepiece.diff"],
    sha256 = "8409b0126ebd62b256c685d5757150cf7fcb2b92a2f2b98efb3f38fc36719754",
    strip_prefix = "sentencepiece-0.1.96",
    urls = [
        "https://github.com/google/sentencepiece/archive/refs/tags/v0.1.96.zip",
    ],
)

http_archive(
    name = "darts_clone",
    build_file = "@//third_party:darts_clone.BUILD",
    patch_args = ["-p0"],
    patches = ["//patches:darts_no_exceptions.diff"],
    sha256 = "c97f55d05c98da6fcaf7f9ecc6a6dc6bc5b18b8564465f77abff8879d446491c",
    strip_prefix = "darts-clone-e40ce4627526985a7767444b6ed6893ab6ff8983",
    urls = [
        "https://github.com/s-yata/darts-clone/archive/e40ce4627526985a7767444b6ed6893ab6ff8983.zip",
    ],
)

# Required by LiteRT's rules_ml_toolchain for GPU/CC toolchain configuration.
http_archive(
    name = "rules_ml_toolchain",
    sha256 = "0b42f693a60c6050d87db1e0a0eaeb84ab3f54191fce094d86334faedc807da0",
    strip_prefix = "rules_ml_toolchain-398d613aea7a4c294da49b79a6d6f3f8732bd84c",
    urls = [
        "https://github.com/google-ml-infra/rules_ml_toolchain/archive/398d613aea7a4c294da49b79a6d6f3f8732bd84c.tar.gz",
    ],
)

load("//third_party:local_config_python_stub.bzl", "local_config_python_stub")

local_config_python_stub(name = "local_config_python")
local_config_python_stub(name = "local_execution_config_python")

# Initialize tensorflow workspace.
# The chain mirrors LiteRT's own WORKSPACE structure exactly.
load("@org_tensorflow//tensorflow:workspace3.bzl", "tf_workspace3")

tf_workspace3()

# Hermetic Python init — required between tf_workspace3 and tf_workspace2.
# Taken verbatim from LiteRT's WORKSPACE.
load("@xla//third_party/py:python_init_rules.bzl", "python_init_rules")

python_init_rules()

load("@xla//third_party/py:python_init_rules.bzl", "python_init_rules")

python_init_rules()

# Force hermetic Python 3.10 download via rules_python.
# python_init_toolchains() alone isn't registering python_3_10_host
# in INTERPRETER_LABELS (Kind: "" fallback). This explicit call downloads
# Python 3.10 from python-build-standalone and properly registers it.
load("@rules_python//python:repositories.bzl", "python_register_toolchains")

python_register_toolchains(
    name = "python_3_10",
    python_version = "3.10",
    ignore_root_user_error = True,
)

load("@xla//third_party/py:python_init_repositories.bzl", "python_init_repositories")

python_init_repositories(
    default_python_version = "3.10",
    local_wheel_dist_folder = "dist",
    local_wheel_inclusion_list = [
        "tensorflow*",
        "tf_nightly*",
    ],
    local_wheel_workspaces = ["@org_tensorflow//:WORKSPACE"],
    requirements = {
        "3.10": "@org_tensorflow//:requirements_lock_3_10.txt",
        "3.11": "@org_tensorflow//:requirements_lock_3_11.txt",
        "3.12": "@org_tensorflow//:requirements_lock_3_12.txt",
        "3.13": "@org_tensorflow//:requirements_lock_3_13.txt",
    },
)

load("@xla//third_party/py:python_init_toolchains.bzl", "python_init_toolchains")

python_init_toolchains()

load("@xla//third_party/py:python_init_pip.bzl", "python_init_pip")

python_init_pip()

load("@pypi//:requirements.bzl", "install_deps")

install_deps()

# End hermetic Python init.

load("@org_tensorflow//tensorflow:workspace2.bzl", "tf_workspace2")

tf_workspace2()

load("@org_tensorflow//tensorflow:workspace1.bzl", "tf_workspace1")

tf_workspace1()

load("@org_tensorflow//tensorflow:workspace0.bzl", "tf_workspace0")

tf_workspace0()

# Stub @local_config_cuda — satisfies all load() calls from tensorflow.bzl
# and xla/tsl/platform/default/cuda_build_defs.bzl with no-op definitions.
# TF_NEED_CUDA=0 is set in .bazelrc so no actual CUDA is required.
load("//third_party:local_config_cuda_stub.bzl", "local_config_cuda_stub")

local_config_cuda_stub(name = "local_config_cuda")

# ROCm configure — creates @local_config_rocm stub (TF_NEED_ROCM=0).
load("@xla//third_party/gpus:rocm_configure.bzl", "rocm_configure")

rocm_configure(name = "local_config_rocm")

# Required by tensorflow/tf_version.bzl → tf_version.default.bzl.
load(
    "@xla//third_party/py:python_wheel.bzl",
    "python_wheel_version_suffix_repository",
)

python_wheel_version_suffix_repository(name = "tf_wheel_version_suffix")

#TODO remove this since LiteRT has its own MTK delegate
http_archive(
    name = "neuron_delegate",
    sha256 = "7918cc54a2bab63c30eb87a90de8ce3f3730b5572e0269a2b57a0c9bcd28cd69",
    strip_prefix = "tflite-neuron-delegate-update_for_leroy",
    urls = ["https://github.com/MediaTek-NeuroPilot/tflite-neuron-delegate/archive/refs/heads/update_for_leroy.zip"],
)

http_archive(
    name = "oleander_stemming_library",
    build_file = "@//third_party:oleander_stemming_library.BUILD",
    sha256 = "d4390e82590d67c73ac32629ddd4fc3ba0b6b293a2757612a2e76726c3752e0b",
    strip_prefix = "OleanderStemmingLibrary-45eb3485f67b94d67bb883601ed65459975b3960",
    urls = ["https://github.com/Blake-Madden/OleanderStemmingLibrary/archive/45eb3485f67b94d67bb883601ed65459975b3960.zip"],
)

new_git_repository(
    name = "org_mlperf_inference",
    build_file = "@//flutter/android/third_party:loadgen.BUILD",
    commit = "6776245e99dce0600cfc9a6fb61efd310f87de3d",
    patch_args = ["-p1"],
    patch_cmds = ["python3 loadgen/version_generator.py loadgen/version_generated.cc loadgen"],
    patches = ["//patches:loadgen_mobile_update.patch"],
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
