"""
Post-extraction fixup script for org_tensorflow at LiteRT 2.1.2's pinned commit.

Run from the root of the extracted TF archive. Applies changes that cannot be
expressed as static patch files due to line number uncertainty:

1. tensorflow/tensorflow.bzl
   - Removes the @local_config_cuda//cuda:build_defs.bzl load block.
   - Inlines stub functions (cuda_library, if_cuda, if_cuda_exec) AFTER the
     last load() statement, satisfying Starlark's load-before-code rule.

2. third_party/py/python_configure.bzl
3. third_party/xla/third_party/py/python_configure.bzl
   - Changes hard INTERPRETER_LABELS[key] lookup to .get() with a fallback
     so the file loads cleanly when the hermetic Python host toolchain is not
     registered (Kind: "" case).
"""

import re
import sys
import os


def fix_tensorflow_bzl(path):
    with open(path, "r") as f:
        content = f.read()

    # Remove the @local_config_cuda load block
    content = re.sub(
        r'load\(\s*"@local_config_cuda//cuda:build_defs\.bzl".*?\)',
        "",
        content,
        flags=re.DOTALL,
    )

    # Find the last load() statement and insert stubs immediately after it.
    # Starlark requires all load() before any def/variable.
    stubs = (
        "\n"
        "# Inlined CUDA stubs for TF_NEED_CUDA=0 builds.\n"
        "# @local_config_cuda still required for BUILD config_settings.\n"
        "def cuda_library(**kwargs): native.cc_library(**kwargs)\n"
        "def if_cuda(if_true, if_false = []): return if_false\n"
        "def if_cuda_exec(if_true, if_false = []): return if_false\n"
    )

    # Split into lines, find last load() block end, insert after it
    lines = content.split("\n")
    last_load_end = 0
    i = 0
    while i < len(lines):
        stripped = lines[i].lstrip()
        if stripped.startswith("load("):
            # Find the closing paren of this load block
            j = i
            depth = 0
            while j < len(lines):
                depth += lines[j].count("(") - lines[j].count(")")
                if depth <= 0:
                    last_load_end = j
                    break
                j += 1
        i += 1

    stub_lines = stubs.split("\n")
    lines = lines[: last_load_end + 1] + stub_lines + lines[last_load_end + 1 :]
    content = "\n".join(lines)

    with open(path, "w") as f:
        f.write(content)

    print("Fixed: {}".format(path))


def fix_python_configure(path):
    if not os.path.exists(path):
        print("Skipping (not found): {}".format(path))
        return

    with open(path, "r") as f:
        content = f.read()

    old = "return str(INTERPRETER_LABELS[python_toolchain_name])"
    new = (
        "return INTERPRETER_LABELS.get("
        "python_toolchain_name, "
        "str(Label(\"@python_3_10_x86_64-unknown-linux-gnu//:python3\")))"
    )

    if old not in content:
        print("Pattern not found (already patched?): {}".format(path))
        return

    content = content.replace(old, new)

    with open(path, "w") as f:
        f.write(content)

    print("Fixed: {}".format(path))


def fix_saved_model_loader(path):
    if not os.path.exists(path):
        print("Skipping (not found): {}".format(path))
        return

    with open(path, "r") as f:
        content = f.read()

    new_content = re.sub(
        r'if_static\(\s*\[\s*'
        r'"//tensorflow/core:all_kernels",\s*'
        r'"//tensorflow/core:direct_session",\s*\]\s*\)',
        '[\n            "//tensorflow/core:all_kernels",\n'
        '            "//tensorflow/core:direct_session",\n        ]',
        content,
    )

    if new_content == content:
        print("Pattern not found (already patched?): {}".format(path))
        return

    with open(path, "w") as f:
        f.write(new_content)

    print("Fixed: {}".format(path))


if __name__ == "__main__":
    fix_tensorflow_bzl("tensorflow/tensorflow.bzl")
    fix_tensorflow_bzl("third_party/xla/xla/tsl/tsl.bzl")
    fix_python_configure("third_party/py/python_configure.bzl")
    fix_python_configure("third_party/xla/third_party/py/python_configure.bzl")
    fix_saved_model_loader("tensorflow/cc/saved_model/BUILD")
    print("All done.")
