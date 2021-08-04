#!/bin/bash

# Copyright 2021 The MLPerf Authors. All Rights Reserved.
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

set -e

ERROR=0

if [ $(git diff --name-only --diff-filter=d origin/master origin/${GIT_BRANCH}| wc -l) -eq 0 ]; then
  echo "No new files found"
  exit 0
fi

ls_staged_files () {
  # Don't throw errors if egrep find no match.
  echo $(git diff --name-only --diff-filter=d origin/master origin/${GIT_BRANCH} | egrep $1 || true)
}

# Formatting cpp files using clang-format.
cpp_files=$(ls_staged_files "\.h|\.cc|\.cpp")
if [ "$cpp_files" ]; then
  echo "Updated/new C++ files: $cpp_files"
  clang-format -i  -style=google $cpp_files
else
  echo "There is no updated/new C++ files"
fi

# Formatting build files using buildifier.
build_files=$(ls_staged_files "WORKSPACE|BUILD|BUILD.bazel|\.bzl")
if [ "$build_files" ]; then
  echo "Updated/new Bazel files: $build_files"
  buildifier -v $build_files
else
  echo "There is no updated/new Bazel files"
fi

# Formatting Java files.
java_files=$(ls_staged_files "\.java")
if [ "$java_files" ]; then
  echo "Updated/new Java files: $java_files"
  java -jar formatters/google-java-format/google-java-format-1.9-all-deps.jar --replace  $java_files
else
  echo "There is no updated/new Java files"
fi

# Create path-file to fix code format
git diff >codeformat-${GIT_COMMIT}.patch

# Return error if file(s) require(s) code formatting
if [ $(git ls-files -m | wc -l) -ne 0 ]; then
  echo "\nNeed code formatting!\n"
  ERROR=1
else
  echo "\nNo need code formatting\n"
fi

# Search files with prohibited extensions
prohibited_files=$(ls_staged_files "\.so|\.apk|\.tflite|\.dlc|\.zip|\.jar|\.tgz")
prohibited_file_list=""
for nf in $prohibited_files
    do
        prohibited_file_list="$prohibited_file_list  $nf\n"
    done
if [ "$prohibited_file_list" ]; then
    echo "\n* File(s) with prohibited extensions:"
    echo "$prohibited_file_list"
    ERROR=1
else
  echo "\n* There is no file(s) with prohibited extensions\n"
fi

# Search files with size >5Mb
new_files=$(git diff --name-only --diff-filter=d origin/master origin/${GIT_BRANCH} )
big_file_list=""
for nf in $new_files
    do
        big_file=$(find . -name $nf -type f -size +5M || true)
        if [ "$big_file" ]; then
            big_file_list="$big_file_list  $big_file\n"
        fi
    done
if [ "$big_file_list" ]; then
    echo "\n* Too big file(s):"
    echo "$big_file_list"
    echo
    ERROR=1
else
  echo "\n* There is no too big file(s)\n"
fi

echo "Exit code: $ERROR"
exit $ERROR
