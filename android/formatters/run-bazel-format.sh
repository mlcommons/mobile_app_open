#!/bin/bash

# Copyright 2020 The MLPerf Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

buildifier WORKSPACE
find android -name BUILD | xargs buildifier
find android -name BUILD.bazel | xargs buildifier
find android -iname "*.bzl" | xargs buildifier

if [ "$1" = "CI" ]; then
    git diff >bazel-codeformat-${GIT_COMMIT}.patch

    if [ $(git ls-files -m | wc -l) -ne 0 ]; then
        echo "\nNeed code formatting!\n"
        exit 1
    fi
fi
