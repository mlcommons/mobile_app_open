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

.PHONY: flutter/windows
flutter/windows: flutter/windows/libs

flutter_windows_libs_folder=flutter/windows/libs
.PHONY: flutter/windows/libs
flutter/windows/libs:
	bazel build ${BAZEL_CACHE_ARG} ${bazel_links_arg} \
		--config=windows \
		${backend_tflite_windows_target} \
		//flutter/cpp/flutter:backend_bridge.dll
	rm -rf ${flutter_windows_libs_folder}
	mkdir -p ${flutter_windows_libs_folder}
	cp -f --target-directory ${flutter_windows_libs_folder} \
		${backend_tflite_windows_files} \
		${BAZEL_LINKS_PREFIX}bin/flutter/cpp/flutter/backend_bridge.dll
	chmod 777 --recursive ${flutter_windows_libs_folder}
