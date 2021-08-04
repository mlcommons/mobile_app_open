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

if [ $(git diff --name-only | wc -l) -ne 0 ]; then
  echo "Please stage or stash unstaged changes first."
  exit 1
fi

ls_staged_files () {
  # Don't throw errors if egrep find no match.
  echo $(git diff --name-only --cached --diff-filter=d | egrep $1 || true)
}

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
fi

# Search files with size >5Mb
new_files=$(git diff --name-only --cached --diff-filter=d)
big_file_list=""
for nf in $new_files
    do
        big_file=$(find $nf -type f -size +5M || true)
        if [ "$big_file" ]; then
            big_file_list="$big_file_list  $big_file\n"
        fi
    done
if [ "$big_file_list" ]; then
    echo "\n* Too big file(s):"
    echo "$big_file_list"
    echo
fi
