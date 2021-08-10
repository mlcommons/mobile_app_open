#!/bin/bash

# Copyright 2020-2021 The MLPerf Authors. All Rights Reserved.
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

# This script formats staged C++ and build files. Files that are already committed
# will not be checked. So please run this script before you commit your changes.
# In case you did commit them, you can use "git reset --soft HEAD~1" to undo the
# commit and commit again after formatting it.

if [ $(git diff --name-only | wc -l) -ne 0 ]; then
    echo "Please stage or stash unstaged changes first."
    exit 1
fi

ls_staged_files () {
    # Don't throw errors if egrep find no match.
    echo $(git diff --name-only --cached --diff-filter=d | egrep $1 || true)
}

# Formatting cpp files using clang-format.
cpp_files=$(ls_staged_files "\.h|\.cc|\.cpp")
if [ "$cpp_files" ]; then
    clang-format-10 -i -style=google $cpp_files
fi


# Formatting build files using buildifier.
buildifier_is_present=$(which buildifier)
if [ ! -x "$buildifier_is_present" ] ; then
    echo "*"
    echo "* Bazel config files can't be formated because 'buildifier' is not in \$PATH"
    echo "*\n"
else
    build_files=$(ls_staged_files "WORKSPACE|BUILD|BUILD.bazel|\.bzl")
    if [ "$build_files" ]; then
        buildifier -v $build_files
    fi
fi

# Formatting Java files.
java_is_present=$(which java)
if [ ! -x "$java_is_present" ] ; then
    echo "*"
    echo "* Java files can't be formated because 'java' is not in \$PATH"
    echo "*\n"
else
    java_files=$(ls_staged_files "\.java")
    if [ "$java_files" ]; then
        java -jar /opt/formatters/google-java-format-1.9-all-deps.jar --replace  $java_files
    fi
fi
