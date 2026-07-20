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
    patch_args = ctx.attr.patch_args if ctx.attr.patch_args else ["-p0"]
    for patch_label in ctx.attr.patches:
        patch_file = ctx.path(patch_label)
        cmd = ["patch"] + patch_args + ["-i", str(patch_file)]
        result = ctx.execute(cmd)
        if result.return_code != 0:
            fail("Failed to apply patch {}: {}\n{}".format(
                patch_label, result.stdout, result.stderr,
            ))

def _apply_patch_cmds(ctx):
    if not ctx.attr.patch_cmds:
        return
    for cmd in ctx.attr.patch_cmds:
        result = ctx.execute(["bash", "-c", cmd])
        if result.return_code != 0:
            fail("patch_cmd failed: {}\nstdout: {}\nstderr: {}".format(
                cmd, result.stdout, result.stderr,
            ))

def _apply_patch_scripts(ctx):
    """Execute Python scripts (resolved from labels) post-extraction."""
    if not ctx.attr.patch_scripts:
        return
    for script_label in ctx.attr.patch_scripts:
        script_path = ctx.path(script_label)
        result = ctx.execute(["python3", str(script_path)])
        if result.return_code != 0:
            fail("patch_script failed: {}\nstdout: {}\nstderr: {}".format(
                script_label, result.stdout, result.stderr,
            ))
        if result.stdout:
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

    _apply_patches(ctx)
    _apply_patch_cmds(ctx)
    _apply_patch_scripts(ctx)

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
