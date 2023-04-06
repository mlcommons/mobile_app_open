# Copyright (c) 2020-2023 Qualcomm Innovation Center, Inc. All rights reserved.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#    http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Find SNPE folder and pass it to bazel build config"""

def _impl(repository_ctx):
    if "windows" in repository_ctx.os.name:
        # print(repository_ctx.attr.workspace_dir + "/mobile_back_qti/")
        found = repository_ctx.execute(["ls", repository_ctx.attr.workspace_dir + "/mobile_back_qti"])
        if found.return_code != 0 or found.stdout == "" or found.stdout == "\n":
            fail("snpe folder is not found in the repo: " + found.stderr)
        filelist = found.stdout.split("\n")
        filepath = ""
        for x in filelist:
            if x.find("snpe-") == 0:
                filepath = x
                break
        if filepath == "":
            fail("snpe folder is not found in the repo")
    else:
        found = repository_ctx.execute(["find", repository_ctx.attr.workspace_dir + "/mobile_back_qti/", "-maxdepth", "1", "-name", "snpe-*", "-type", "d", "-print", "-quit"])
        if found.return_code != 0 or found.stdout == "" or found.stdout == "\n":
            fail("snpe folder is not found in the repo")
        filepath = found.stdout[:-1]

    sdk_version = filepath[found.stdout.rfind("/") + 1:]
    print("Update SNPE version: " + sdk_version)  # buildifier: disable=print
    repository_ctx.read(Label("@//:mobile_back_qti/" + sdk_version + "/ReleaseNotes.txt"))

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
