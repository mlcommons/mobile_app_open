# Copyright (c) 2020-2024 Qualcomm Innovation Center, Inc. All rights reserved.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#    http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Find OPENCV folder and pass it to bazel build config"""

def _impl(repository_ctx):
    opencv_path = "opencv"
    repository_ctx.file("BUILD", "")
    repository_ctx.file(
        "stable_diffusion_var_def_shared.bzl",
        "OPENCV_ROOT_DIR = \"include/%s\"" % opencv_path,
    )

stable_diffusion_external_deps_shared = repository_rule(
    implementation = _impl,
    environ = ["OPENCV_ROOT_DIR"],
    local = True,
    attrs = {"workspace_dir": attr.string(mandatory = True)},
)

