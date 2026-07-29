load("@bazel_tools//tools/cpp:unix_cc_configure.bzl", "configure_unix_toolchain")
load("@bazel_tools//tools/cpp:lib_cc_configure.bzl", "get_cpu_value")

def _auto_macos_host_toolchain_impl(repository_ctx):
    os_name = repository_ctx.os.name.lower()
    cpu_value = get_cpu_value(repository_ctx)

    sdk_dir = ""
    if "mac" in os_name or "darwin" in os_name:
        res = repository_ctx.execute(["xcrun", "--show-sdk-path"])
        if res.return_code == 0 and res.stdout.strip():
            sdk_dir = res.stdout.strip()

    configure_unix_toolchain(repository_ctx, cpu_value, {})

    if sdk_dir:
        content = repository_ctx.read("BUILD")
        quoted_sdk = '"' + sdk_dir + '"'
        if quoted_sdk not in content and 'cxx_builtin_include_directories = [' in content:
            new_content = content.replace(
                'cxx_builtin_include_directories = [',
                'cxx_builtin_include_directories = [' + quoted_sdk + ',\n    ',
            )
            repository_ctx.file("BUILD", new_content)

auto_macos_host_toolchain = repository_rule(
    environ = [
        "ABI_LIBC_VERSION",
        "ABI_VERSION",
        "BAZEL_COMPILER",
        "BAZEL_HOST_SYSTEM",
        "BAZEL_CONLYOPTS",
        "BAZEL_CXXOPTS",
        "BAZEL_LINKOPTS",
        "BAZEL_LINKLIBS",
        "BAZEL_LLVM_COV",
        "BAZEL_LLVM_PROFDATA",
        "BAZEL_PYTHON",
        "BAZEL_SH",
        "BAZEL_TARGET_CPU",
        "BAZEL_TARGET_LIBC",
        "BAZEL_TARGET_SYSTEM",
        "BAZEL_DO_NOT_DETECT_CPP_TOOLCHAIN",
        "BAZEL_USE_LLVM_NATIVE_COVERAGE",
        "BAZEL_LLVM",
        "BAZEL_IGNORE_SYSTEM_HEADERS_VERSIONS",
        "USE_CLANG_CL",
        "CC",
        "CC_CONFIGURE_DEBUG",
        "CC_TOOLCHAIN_NAME",
        "CPLUS_INCLUDE_PATH",
        "DEVELOPER_DIR",
        "SDKROOT",
        "GCOV",
        "LIBTOOL",
        "HOMEBREW_RUBY_PATH",
        "SYSTEMROOT",
        "USER",
    ],
    implementation = _auto_macos_host_toolchain_impl,
    configure = True,
)
