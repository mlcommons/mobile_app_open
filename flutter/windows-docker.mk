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

.PHONY: flutter/windows/docker/image
flutter/windows/docker/image:
	docker build -t mlperf_mobile_flutter:windows-1.0 windows/docker

.PHONY: flutter/windows/docker/create-container
flutter/windows/docker/create-container:
	docker rm -f mobile_app_flutter_windows_container
	mkdir -p output
	echo >output/container-script.bat
	@# "-it" here is required to make the container killable by Ctrl+C signal.
	@# A lot of memory is required by bazel.
	@#		With default settings only 1 bazel job can run at a time, which, obviously, greatly slows down the build.
	@#		4G typically runs 6-7 jobs. 8G is enough for bazel to run 16+ jobs.
	@# Also a lot of memory is required by Flutter. When using 2G Flutter fails with out of memory error, so at least 4G is needed.
	MSYS2_ARG_CONV_EXCL="*" docker run -it --name mobile_app_flutter_windows_container \
		--detach --memory 8G --cpus $$(( `nproc` - 1 )) \
		--volume $(CURDIR):C:/mnt/project \
		--workdir C:/mnt/project \
		mlperf_mobile_flutter:windows-1.0 \
		".\\output\\container-script.bat <NUL"

.PHONY: flutter/windows/docker/native
flutter/windows/docker/native:
	@# We must create the directory here because msys2 commands inside a container
	@# can't manipulate files directly inside a mounted directory for some reason
	@# but can freely manupulate nested directories.
	mkdir -p build
	echo >output/container-script.bat "make BAZEL_LINKS_PREFIX=C:/bazel-links/ flutter/backend-bridge-windows flutter/backends/tflite-windows"
	docker start -ai mobile_app_flutter_windows_container

.PHONY: flutter/windows/docker/flutter-release
flutter/windows/docker/flutter-release:
	mkdir -p output/windows-build
	echo >output/container-script.bat "\
		make flutter/windows/copy-flutter-files-for-docker \
		&& cd C:/project-local \
		&& make flutter/prepare-flutter flutter/windows/flutter-release \
		&& cp -r build/windows/runner/Release C:/mnt/project/output/windows-build \
		"
	docker start -ai mobile_app_flutter_windows_container

# In Docker Windows containers it's impossible to create a link
# from a mounted volume/folder to any folder of the container itself.
# Flutter wants to create links,
# 		and place them here: flutter/windows/flutter/ephemeral/.plugin_symlinks
# so the build doesn't work.
#
# This make target expects that all DLLs are already present.
.PHONY: flutter/windows/copy-flutter-files-for-docker
flutter/windows/copy-flutter-files-for-docker:
	@# for some reason, make can't delete the folder with symlinks:
	@# 		meither rm nor rmdir (both from msys2) doesn't do anything to the .plugin_symlinks folder or its contents,
	@#		so we call native Windows command line here.
	if [ -d "flutter/windows/flutter/ephemeral/.plugin_symlinks" ]; then MSYS2_ARG_CONV_EXCL="*" cmd /S /C "rmdir /S /Q flutter\\windows\\flutter\\ephemeral\\.plugin_symlinks"; fi
	rm -rf C:/project-local
	mkdir -p C:/project-local
	cp -r --target-directory C:/project-local \
		flutter/assets \
		flutter/cpp \
		flutter/integration_test \
		flutter/lib \
		flutter/test_driver \
		flutter/tool \
		flutter/windows \
		flutter/Makefile \
		flutter/pubspec.yaml \
		flutter/pubspec.lock
	mkdir -p C:/project-local/build
	cp -r --target-directory C:/project-local/build \
		build/win-dlls

# _windows_container_redist_dlls_dir is specific to our docker image.
# 		When building locally, path to MSVC DLLs may be different.
#		Particularly, MS VS installer typycally installs Community/Pro/Enterprise version instead of "BuildTools"
#		Also version numbers may potentially be different.
_windows_container_redist_dlls_dir="C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Redist/MSVC/14.29.30133/x64/Microsoft.VC142.CRT"
.PHONY: flutter/windows/flutter-release
flutter/windows/flutter-release:
	flutter build windows ${flutter_common_dart_flags}
	cp -t build/windows/runner/Release \
		${_windows_container_redist_dlls_dir}/msvcp140.dll \
		${_windows_container_redist_dlls_dir}/vcruntime140.dll \
		${_windows_container_redist_dlls_dir}/vcruntime140_1.dll \
		${_windows_container_redist_dlls_dir}/msvcp140_codecvt_ids.dll
