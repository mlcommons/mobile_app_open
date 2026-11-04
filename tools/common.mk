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

ifeq (${USE_PROXY_WORKAROUND},1)
  proxy_docker_args=\
	-v /etc/ssl/certs:/etc/ssl/certs \
	-v /usr/share/ca-certificates:/usr/share/ca-certificates \
	-v /usr/share/ca-certificates-java:/usr/share/ca-certificates-java

  proxy_bazel_args=--host_jvm_args -Djavax.net.ssl.trustStore=/etc/ssl/certs/java/cacerts
endif

ifeq (${OS},Windows_NT)
# On Windows some commands don't run correctly in the default make shell
_start_args=powershell -Command
else
_start_args=
endif

# We define default value here, but user can overwrite it.
# Particularly, we have to overwrite it when building inside Windows container
BAZEL_LINKS_PREFIX=bazel-
bazel_links_arg=--symlink_prefix ${BAZEL_LINKS_PREFIX}

# Xcode 15+'s clang reads <SDKROOT>/SDKSettings.json on every compile (even
# for non-Apple targets, e.g. host tools like protoc built for Android
# builds), regardless of flags. Bazel's host/exec cc toolchain only allowlists
# that absolute path for the Command Line Tools SDK
# (/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk), not for the
# full-Xcode one, so any host-tool compile fails with "absolute path
# inclusion(s) found" once Xcode.app is the active developer dir. Steering
# the host toolchain at the Command Line Tools SDK sidesteps the mismatch;
# it's only used for host-tool compilation, so it doesn't affect the actual
# Apple target builds (ios.mk/apple_backend), which keep using Xcode.app.
ifeq ($(shell uname -s),Darwin)
ifneq ($(wildcard /Library/Developer/CommandLineTools/usr/bin/clang),)
bazel_macos_host_sdk_args=\
	--repo_env=DEVELOPER_DIR=/Library/Developer/CommandLineTools \
	--action_env=DEVELOPER_DIR=/Library/Developer/CommandLineTools \
	--host_action_env=DEVELOPER_DIR=/Library/Developer/CommandLineTools
endif
endif
