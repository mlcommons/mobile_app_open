"""
Stub @local_config_cuda repository for TF_NEED_CUDA=0 builds.

The cuda/ subdirectory is required by Bazel to resolve the package
@local_config_cuda//cuda referenced throughout XLA and TF BUILD files.

cuda/build_defs.bzl provides all symbols loaded from this package by any
file in tensorflow.bzl, xla/tsl/tsl.bzl, and xla/tsl/platform/default/.
The top-level BUILD provides config_settings used in select() expressions.
"""

def _local_config_cuda_impl(ctx):
    ctx.file("BUILD", """
config_setting(
    name = "is_cuda_enabled",
    values = {"define": "using_cuda=true"},
    visibility = ["//visibility:public"],
)
config_setting(
    name = "is_cuda_clang",
    values = {"define": "using_cuda_clang=true"},
    visibility = ["//visibility:public"],
)
config_setting(
    name = "cuda_tools_and_libs",
    values = {"define": "using_cuda=true"},
    visibility = ["//visibility:public"],
)
""")

    ctx.file("cuda/BUILD", "")

    ctx.file("cuda/build_defs.bzl", """\
\"\"\"Stub CUDA build defs for TF_NEED_CUDA=0 builds.\"\"\"

def cuda_library(**kwargs): native.cc_library(**kwargs)
def if_cuda(if_true, if_false = []): return if_false
def if_cuda_exec(if_true, if_false = []): return if_false
def if_cuda_is_configured(x, no_cuda = []): return no_cuda
def is_cuda_configured(): return False
def if_cuda_clang(if_true, if_false = []): return if_false
def if_cuda_clang_opt(if_true, if_false = []): return if_false
def if_cuda_newer_than(ver, if_true, if_false = []): return if_false
def if_nccl(if_true, if_false = []): return if_false
def cuda_header_library(**kwargs): native.cc_library(**kwargs)
def cuda_copts(proto = False): return []
def cuda_default_copts(): return []
def cuda_gpu_kernels_copts(): return []
def cuda_gpu_architectures(): return []
CUDA_VERSION = ""
CUDNN_VERSION = ""
TF_NEED_CUDA = 0
""")

local_config_cuda_stub = repository_rule(
    implementation = _local_config_cuda_impl,
    local = True,
    doc = "Stub @local_config_cuda for TF_NEED_CUDA=0 builds.",
)
