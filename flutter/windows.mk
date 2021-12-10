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

debug_flags_windows=-c dbg --copt /Od --copt /Z7 --linkopt -debug

# To add a new backend, add flutter/backends/example-windows target (e.g. flutter/backends/intel-windows)
.PHONY: flutter/windows
flutter/windows: flutter/windows/bridge flutter/windows/backends/tflite flutter/prepare-flutter

.PHONY: flutter/windows/bridge
flutter/windows/bridge:
	bazel build ${BAZEL_CACHE_ARG} ${_bazel_links_arg} --config=windows -c opt //flutter/cpp/flutter:backend_bridge.dll
	mkdir -p flutter/windows/libs
	cp -f ${BAZEL_LINKS_DIR}bin/flutter/cpp/flutter/backend_bridge.dll flutter/windows/libs/backend_bridge.dll

# Use the following block as a template to add a new Windows backend
#.PHONY: flutter/windows/backends/example
#flutter/windows/backends/example:
#	bazel build ${_bazel_links_arg} --config=windows -c opt //mobile_back_example:examplebackenddll
#	mkdir -p flutter/windows/libs
#	cp -f ${BAZEL_LINKS_DIR}bin/mobile_back_example/cpp/backend_example/libexamplebackend.dll flutter/windows/libs/libexamplebackend.dll

.PHONY: flutter/windows/backends/tflite
flutter/windows/backends/tflite:
	bazel build ${BAZEL_CACHE_ARG} ${_bazel_links_arg} --config=windows -c opt ${BACKEND_TFLITE_DLL_TARGET}
	mkdir -p flutter/windows/libs
	cp -f ${BAZEL_LINKS_DIR}bin/${BACKEND_TFLITE_DLL_FILE} flutter/windows/libs/${BACKEND_TFLITE_FILENAME}
