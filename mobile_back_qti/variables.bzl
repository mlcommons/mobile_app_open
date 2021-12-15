"""Find SNPE folder and pass it to bazel build config"""

def _impl(repository_ctx):
    if "windows" in repository_ctx.os.name:
        fail("building with SNPE on Windows is not supported")

    found = repository_ctx.execute(["find", repository_ctx.attr.workspace_dir + "/mobile_back_qti/", "-maxdepth", "1", "-name", "snpe-*", "-type", "d", "-print", "-quit"])
    if found.return_code != 0 or found.stdout == "":
        fail("snpe folder is not found in the repo")

    sdk_version = found.stdout[found.stdout.rfind("/") + 1:-1]
    print("Using SNPE: " + sdk_version)  # buildifier: disable=print

    repository_ctx.file("BUILD", "")
    repository_ctx.file(
        "snpe_var_def.bzl",
        "SNPE_VERSION = \"%s\"" % sdk_version,
    )

snpe_version_loader = repository_rule(
    implementation = _impl,
    environ = ["SNPE_VERSION"],
    local = True,
    attrs = {"workspace_dir": attr.string(mandatory = True)},
)
