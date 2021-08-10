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

# This script formats all staged C++ and build files.

# Formatting cpp files using clang-format.
cpp_files=$(find android -name "*.h" && find android -name "*.cc")
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
  build_files=$(echo WORKSPACE && find android -name BUILD && find android -name BUILD.bazel && find android -name "*.bzl")
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
  java_files=$(find android -name "*.java")
  if [ "$java_files" ]; then
    java -jar /opt/formatters/google-java-format-1.9-all-deps.jar --replace  $java_files
  fi
fi
