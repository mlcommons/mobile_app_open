"""Find platform and decide TF patch and pass it to bazel build config"""

def _impl(repository_ctx):
    if "windows" in repository_ctx.os.name:
        patch_file = "[\"//patches:TF-Changes-to-add-windows_arm64.patch\"]"
    else:
        patch_file = []

    repository_ctx.file("BUILD", "")
    repository_ctx.file(
        "patch_win_arm64.bzl",
        "PATCH_FILE=%s" % patch_file,
    )

tf_patch_finder = repository_rule(
    implementation = _impl,
    environ = ["PATCH_FILE"],
    local = True,
    attrs = {"workspace_dir": attr.string(mandatory = True)},
)
