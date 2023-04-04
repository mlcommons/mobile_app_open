# Copyright 2023 The MLPerf Authors. All Rights Reserved.
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

include flutter/cpp/binary/cmdline-docker.mk

cmdline/android/bins/release: cmdline/android/bins/build cmdline/android/bins/copy

.PHONY: cmdline/android/bins/build
cmdline/android/bins/build:
	bazel ${BAZEL_ARGS_GLOBAL} ${sonar_bazel_startup_options} \
		build ${BAZEL_CACHE_ARG} ${bazel_links_arg} ${sonar_bazel_build_args} \
		--config=android_arm64 \
		${backend_tflite_android_target} \
		${backend_mediatek_android_target} \
		${backend_pixel_android_target} \
		${backend_qti_android_target} \
		${backend_samsung_android_target} \
		//flutter/cpp/flutter:libbackendbridge.so \
		//flutter/cpp/binary:main

cmdline_android_bin_release_path=output/cmdline_bins/release
.PHONY: cmdline/android/bins/copy
cmdline/android/bins/copy:
	rm -rf ${cmdline_android_bin_release_path}
	mkdir -p ${cmdline_android_bin_release_path}
	@# macos doesn't support --target-directory flag
	cp -f \
		${backend_tflite_android_files} \
		${backend_mediatek_android_files} \
		${backend_pixel_android_files} \
		${backend_qti_cmdline_files} \
		${backend_samsung_android_files} \
		${BAZEL_LINKS_PREFIX}bin/flutter/cpp/flutter/libbackendbridge.so \
		${BAZEL_LINKS_PREFIX}bin/flutter/cpp/binary/main \
		${cmdline_android_bin_release_path}
		@# macos doesn't support --recursive flag
		chmod -R 777 ${cmdline_android_bin_release_path}