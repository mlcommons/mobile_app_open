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
	bazel ${BAZEL_OUTPUT_ROOT_ARG} ${proxy_bazel_args} ${sonar_bazel_startup_options} \
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

windows_cmdline_folder=output/windows/cmdline
.PHONY: cmdline/windows/bins
cmdline/windows/bins:
	bazel ${BAZEL_OUTPUT_ROOT_ARG} --output_base=C:\\b_cache1\\ build ${BAZEL_CACHE_ARG} ${bazel_links_arg} \
		--config=windows_arm64 \
		--cpu=x64_arm64_windows --worker_verbose\
		${backend_tflite_windows_target} \
		${backend_qti_windows_target} \
		//flutter/cpp/binary:main
	rm -rf ${windows_cmdline_folder}
	mkdir -p ${windows_cmdline_folder}
	cp -f --target-directory ${windows_cmdline_folder} \
		${backend_tflite_windows_files} \
		${backend_qti_windows_files} \
		${BAZEL_LINKS_PREFIX}bin/flutter/cpp/binary/main.exe
	chmod 777 --recursive ${windows_cmdline_folder}

# set parameters before running `make cmdline/windows/release`
MSVC_ARM_DLLS?=
.PHONY: cmdline/windows/release
cmdline/windows/release: \
	cmdline/windows/prepare-dlls \
	cmdline/windows/bins \
	cmdline/windows/copy-dlls

msvc_arm_dlls_path=output/windows/win-redist-dlls
msvc_arm_dlls_list=msvcp140.dll vcruntime140.dll vcruntime140_1.dll msvcp140_codecvt_ids.dll
.PHONY: cmdline/windows/prepare-dlls
cmdline/windows/prepare-dlls:
	@[ -n "${MSVC_ARM_DLLS}" ] || (echo MSVC_ARM_DLLS env must be set; exit 1)

	rm -rf ${msvc_arm_dlls_path}
	mkdir -p ${msvc_arm_dlls_path}
	currentDir=$$(pwd) && cd "${MSVC_ARM_DLLS}" && \
		cp  --target-directory $$currentDir/${msvc_arm_dlls_path} ${msvc_arm_dlls_list}

.PHONY: cmdline/windows/copy-dlls
cmdline/windows/copy-dlls:
	currentDir=$$(pwd) && cd "${msvc_arm_dlls_path}" && \
		cp  --target-directory $$currentDir/${windows_cmdline_folder} ${msvc_arm_dlls_list}		