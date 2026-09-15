# This file is used to define a custom repository rule for TensorFlow submodule used by LiteRT.
#
# Modifications vs upstream LiteRT:
#   - Creates an empty 'unused' file after extraction so that tf_vendored's
#     ctx.path(@org_tensorflow//:unused).dirname path resolution works correctly.
#   - Adds 'patches' and 'patch_args' attributes (mirrors http_archive).
#   - Adds 'patch_cmds' attribute for shell commands post-extraction.
#   - Adds 'patch_scripts' attribute: list of Python script labels to execute
#     post-extraction. Avoids shell escaping issues with complex transformations.

"""Custom TensorFlow source repository rule for LiteRT builds."""

def _apply_patches(ctx):
    if not ctx.attr.patches:
        return

    # Use Bazel's built-in patch implementation (the same one http_archive
    # uses): it is pure Java, so it works on hosts without a `patch` binary,
    # such as the Windows builder. Only `-pN` strip levels are supported.
    strip = 0
    for arg in ctx.attr.patch_args:
        if arg.startswith("-p"):
            strip = int(arg[2:])
        else:
            fail("Unsupported patch_args {}: only -pN is supported.".format(
                ctx.attr.patch_args,
            ))
    for patch_label in ctx.attr.patches:
        ctx.patch(ctx.path(patch_label), strip = strip)

def _apply_patch_cmds(ctx):
    if not ctx.attr.patch_cmds:
        return
    for cmd in ctx.attr.patch_cmds:
        result = ctx.execute(["bash", "-c", cmd])
        if result.return_code != 0:
            fail("patch_cmd failed: {}\nstdout: {}\nstderr: {}".format(
                cmd,
                result.stdout,
                result.stderr,
            ))

def _apply_patch_scripts(ctx):
    """Execute Python scripts (resolved from labels) post-extraction."""
    if not ctx.attr.patch_scripts:
        return
    python = "python3"
    if ctx.which("python3") == None and ctx.which("python") != None:
        # Windows installs typically ship `python`, not `python3`.
        python = "python"
    for script_label in ctx.attr.patch_scripts:
        script_path = ctx.path(script_label)
        result = ctx.execute([python, str(script_path)])
        if result.return_code != 0:
            fail("patch_script failed: {}\nstdout: {}\nstderr: {}".format(
                script_label,
                result.stdout,
                result.stderr,
            ))
        if result.stdout:
            # buildifier: disable=print
            print(result.stdout)

def _tensorflow_source_repo_impl(ctx):
    use_local_tf = ctx.os.environ.get("USE_LOCAL_TF", "false") == "true"

    if use_local_tf:
        TF_LOCAL_SOURCE_PATH_ENV = ctx.os.environ.get("TF_LOCAL_SOURCE_PATH", "")
        if not TF_LOCAL_SOURCE_PATH_ENV:
            fail("""ERROR: USE_LOCAL_TF is true, but TF_LOCAL_SOURCE_PATH is not set.""")
        resolved_local_path = ctx.path(TF_LOCAL_SOURCE_PATH_ENV)
        for f in resolved_local_path.readdir():
            ctx.symlink(f, f.basename)
    else:
        ctx.download_and_extract(
            url = ctx.attr.urls[0],
            sha256 = ctx.attr.sha256,
            stripPrefix = ctx.attr.strip_prefix,
        )

    # Required by tf_vendored for @xla path resolution.
    ctx.file("unused", "")

    _patch_xla_windows_copts(ctx)
    _apply_patches(ctx)
    _apply_patch_cmds(ctx)
    _apply_patch_scripts(ctx)

def _patch_xla_windows_copts(ctx):
    """Switch XLA Windows copts to MSVC-style flags (same fix as upstream LiteRT).

    xla/tsl assumes clang-cl on Windows and hardcodes is_msvc = False, which
    passes GNU-style flags such as -Wno-sign-compare that cl.exe rejects
    (error D8021). Only the //xla/tsl:windows select branch is affected, so
    this is a no-op for every non-Windows build.
    """
    tsl_bzl = "third_party/xla/xla/tsl/tsl.bzl"
    content = ctx.read(tsl_bzl)
    old = 'clean_dep("//xla/tsl:windows"): get_win_copts(is_external, is_msvc = False),'
    new = 'clean_dep("//xla/tsl:windows"): get_win_copts(is_external, is_msvc = True),'
    if old not in content:
        fail("{} does not contain the expected is_msvc line; ".format(tsl_bzl) +
             "re-check _patch_xla_windows_copts against the new TF pin.")
    ctx.file(tsl_bzl, content.replace(old, new))

tensorflow_source_repo = repository_rule(
    implementation = _tensorflow_source_repo_impl,
    local = False,
    attrs = {
        "sha256": attr.string(mandatory = False),
        "strip_prefix": attr.string(mandatory = True),
        "urls": attr.string_list(mandatory = True),
        "patches": attr.label_list(
            mandatory = False,
            default = [],
            doc = "Patch files to apply after extraction.",
        ),
        "patch_args": attr.string_list(
            mandatory = False,
            default = ["-p1"],
            doc = "Arguments for the patch tool.",
        ),
        "patch_cmds": attr.string_list(
            mandatory = False,
            default = [],
            doc = "Shell commands to run after patches.",
        ),
        "patch_scripts": attr.label_list(
            mandatory = False,
            default = [],
            doc = "Python script labels to execute after patch_cmds.",
        ),
    },
)
