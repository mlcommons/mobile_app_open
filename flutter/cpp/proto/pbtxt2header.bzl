"""Generates header files from .pbtxt files."""

def pbtxt2header(name, srcs):
    """Generates header files from .pbtxt files.

    Generated header file name is the same as src file name plus the .h extension.
    The content of the header file is a std::string containing the content of the .pbtxt file.
    Args:
        name: Name of the rule.
        srcs: List of .pbtxt files to generate header files from.
    """
    hdrs = []
    for src in srcs:
        out = "%s.h" % src
        _pbtxt2header(
            name = "%s.rule" % src,
            src = src,
            out = out,
            tool = "//flutter/cpp/proto:pbtxt2header",
        )
        hdrs.append(out)

    native.cc_library(
        name = name,
        hdrs = hdrs,
        include_prefix = ".",
        visibility = ["//visibility:public"],
    )

def _impl(ctx):
    input = ctx.file.src
    output = ctx.outputs.out
    args = [input.path] + [output.path]
    ctx.actions.run(
        inputs = [input],
        outputs = [output],
        arguments = args,
        progress_message = "Generate header file for %s" % input.short_path,
        executable = ctx.executable.tool,
    )

_pbtxt2header = rule(
    implementation = _impl,
    output_to_genfiles = True,
    attrs = {
        "src": attr.label(
            mandatory = True,
            allow_single_file = [".pbtxt"],
        ),
        "out": attr.output(
            mandatory = True,
        ),
        "tool": attr.label(
            executable = True,
            cfg = "exec",
            allow_files = True,
        ),
    },
)
