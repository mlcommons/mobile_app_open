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

include flutter/windows/windows-docker.mk

debug_flags_windows=-c dbg --copt /Od --copt /Z7 --linkopt -debug

flutter/windows: flutter/windows/libs

flutter_windows_libs_folder=flutter/windows/libs
.PHONY: flutter/windows/libs
flutter/windows/libs:
	bazel ${BAZEL_OUTPUT_ROOT_ARG} build ${BAZEL_CACHE_ARG} ${bazel_links_arg} \
		--config=windows \
		${backend_tflite_windows_target} \
		//flutter/cpp/flutter:backend_bridge.dll
	rm -rf ${flutter_windows_libs_folder}
	mkdir -p ${flutter_windows_libs_folder}
	cp -f --target-directory ${flutter_windows_libs_folder} \
		${backend_tflite_windows_files} \
		${BAZEL_LINKS_PREFIX}bin/flutter/cpp/flutter/backend_bridge.dll
	chmod 777 --recursive ${flutter_windows_libs_folder}

# set parameters before running `make flutter/windows/release`
FLUTTER_MSVC_DLLS?=
FLUTTER_RELEASE_NAME?=
flutter_windows_releases=output/flutter-windows-releases
.PHONY: flutter/windows/release
flutter/windows/release: \
	flutter/windows/release/prepare-dlls \
	flutter/windows/release/build \
	flutter/windows/release/copy-dlls \
	flutter/windows/release/name

flutter_windows_dlls_path=output/win-redist-dlls
flutter_windows_dlls_list=msvcp140.dll vcruntime140.dll vcruntime140_1.dll msvcp140_codecvt_ids.dll
.PHONY: flutter/windows/release/prepare-dlls
flutter/windows/release/prepare-dlls:
	@[ -n "${FLUTTER_MSVC_DLLS}" ] || (echo FLUTTER_MSVC_DLLS env must be set; exit 1)

	rm -rf ${flutter_windows_dlls_path}
	mkdir -p ${flutter_windows_dlls_path}
	currentDir=$$(pwd) && cd "${FLUTTER_MSVC_DLLS}" && \
		cp  --target-directory $$currentDir/${flutter_windows_dlls_path} ${flutter_windows_dlls_list}

.PHONY: flutter/windows/release/copy-dlls
flutter/windows/release/copy-dlls:
	currentDir=$$(pwd) && cd "${flutter_windows_dlls_path}" && \
		cp  --target-directory $$currentDir/flutter/build/windows/x64/runner/Release ${flutter_windows_dlls_list}

.PHONY: flutter/windows/release/build
flutter/windows/release/build:
	rm -rf flutter/build/windows/x64/runner/Release
	cd flutter && ${_start_args} flutter --no-version-check build windows --no-pub \
		${flutter_official_build_arg} \
		${flutter_firebase_crashlytics_arg} \
		${flutter_build_number_arg}

.PHONY: flutter/windows/release/name
flutter/windows/release/name:
	@[ -n "${FLUTTER_RELEASE_NAME}" ] || (echo FLUTTER_RELEASE_NAME env must be set; exit 1)

	rm -rf ${flutter_windows_releases}/${FLUTTER_RELEASE_NAME}
	mkdir -p ${flutter_windows_releases}
	mv flutter/build/windows/x64/runner/Release ${flutter_windows_releases}/${FLUTTER_RELEASE_NAME}

.PHONY: flutter/windows/release/archive
flutter/windows/release/archive:
	@[ -n "${FLUTTER_RELEASE_NAME}" ] || (echo FLUTTER_RELEASE_NAME env must be set; exit 1)

	powershell Compress-Archive \
		${flutter_windows_releases}/${FLUTTER_RELEASE_NAME} \
		${flutter_windows_releases}/${FLUTTER_RELEASE_NAME}.zip
