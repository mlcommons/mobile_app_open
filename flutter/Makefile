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

.PHONY: all
ifeq (${OS},Windows_NT)
all: windows
else
all: ios
endif

.PHONY: ios
ios: cpp-ios prepare-flutter update-splash-screen

BAZEL_LINKS_DIR=bazel-
_bazel_links_arg=--symlink_prefix ${BAZEL_LINKS_DIR} --experimental_no_product_name_out_symlink

.PHONY: cpp-ios
cpp-ios:
	@# NOTE: add `--copt -g` for debug info (but the resulting library would be 0.5 GiB)
	bazel ${BAZEL_OUTPUT_ROOT_ARG} build --config=ios_fat64 -c opt //flutter/cpp/flutter:ios_backend_fw_static

	rm -rf ${_xcode_fw}
	cp -a ${_bazel_ios_fw} ${_xcode_fw}

_bazel_ios_fw := ../bazel-bin/flutter/cpp/flutter/ios_backend_fw_static_archive-root/ios_backend_fw_static.framework
_xcode_fw := ios/Flutter/ios_backend_fw_static.framework

# To add a new backend, add backends/example-windows target (e.g. backends/intel-windows)
.PHONY: windows
windows: backend-bridge-windows backends/tflite-windows prepare-flutter

debug_flags_windows=-c dbg --copt /Od --copt /Z7 --linkopt -debug

.PHONY: android
android: backend-bridge-android backends/tflite-android prepare-flutter

.PHONY: android/apk
android/apk: android
	flutter clean
	@# take results from build/app/outputs/flutter-apk/app-release.apk
	flutter build apk

.PHONY: ci/android/test_apk
ci/android/test_apk: android/apk
	@# take results from build/app/outputs/apk/androidTest/debug/app-debug-androidTest.apk
	cd android && ./gradlew app:assembleAndroidTest
	@# take results from build/app/outputs/apk/debug/app-debug.apk
	cd android && ./gradlew app:assembleDebug -Ptarget=integration_test/first_test.dart

.PHONY: docker/android/apk
docker/android/apk: android/docker/image
	@# if the build fails with java.io.IOException: Input/output error
	@# remove file android/gradle/wrapper/gradle-wrapper.jar
	MSYS2_ARG_CONV_EXCL="*" docker run --rm -it --user `id -u`:`id -g` -v $(CURDIR)/..:/mnt/project mlcommons/mlperf_mobile_flutter bash -c "cd /mnt/project/flutter && make prepare-flutter && make android/apk"

.PHONY: android/docker/image
android/docker/image: build/docker/mlperf_mobile_flutter.stamp

build/docker/mlperf_mobile_flutter.stamp: android/docker/Dockerfile
	docker image build -t mlcommons/mlperf_mobile_flutter android/docker
	mkdir -p build/docker
	touch $@

.PHONY: windows/docker/image
windows/docker/image:
	docker build -t mlperf_mobile_flutter:windows-1.0 windows/docker

.PHONY: windows/docker/create-container
windows/docker/create-container:
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
		--volume $(CURDIR):C:/mobile_app_flutter \
		--workdir C:/mobile_app_flutter \
		mlperf_mobile_flutter:windows-1.0 \
		".\\output\\container-script.bat <NUL"

.PHONY: windows/docker/native
windows/docker/native:
	@# We must create the directory here because msys2 commands inside a container
	@# can't manipulate files directly inside a mounted directory for some reason
	@# but can freely manupulate nested directories.
	mkdir -p build
	echo >output/container-script.bat "make BAZEL_LINKS_DIR=C:/bazel-links/ backend-bridge-windows backends/tflite-windows"
	docker start -ai mobile_app_flutter_windows_container

.PHONY: windows/docker/flutter-release
windows/docker/flutter-release:
	mkdir -p output/windows-build
	echo >output/container-script.bat "\
		make windows/copy-flutter-files-for-docker \
		&& cd C:/mobile_app_flutter-local \
		&& make prepare-flutter windows/flutter-release \
		&& cp -r build/windows/runner/Release C:/mobile_app_flutter/output/windows-build \
		"
	docker start -ai mobile_app_flutter_windows_container

# In Docker Windows containers it's impossible to create a link
# from a mounted volume/folder to any folder of the container itself.
# Flutter wants to create links,
# 		and place them here: windows/flutter/ephemeral/.plugin_symlinks
# so the build doesn't work.
#
# This make target expects that all DLLs are already present.
.PHONY: windows/copy-flutter-files-for-docker
windows/copy-flutter-files-for-docker:
	@# for some reason, make can't delete the folder with symlinks:
	@# 		meither rm nor rmdir (both from msys2) doesn't do anything to the .plugin_symlinks folder or its contents,
	@#		so we call native Windows command line here.
	if [ -d "windows/flutter/ephemeral/.plugin_symlinks" ]; then MSYS2_ARG_CONV_EXCL="*" cmd /S /C "rmdir /S /Q windows\\flutter\\ephemeral\\.plugin_symlinks"; fi
	rm -rf C:/mobile_app_flutter-local
	mkdir -p C:/mobile_app_flutter-local
	cp -r --target-directory C:/mobile_app_flutter-local \
		assets \
		cpp \
		integration_test \
		lib \
		test_driver \
		tool \
		windows \
		Makefile \
		pubspec.yaml \
		pubspec.lock
	mkdir -p C:/mobile_app_flutter-local/build
	cp -r --target-directory C:/mobile_app_flutter-local/build \
		build/win-dlls

# _windows_container_redist_dlls_dir is specific to our docker image.
# 		When building locally, path to MSVC DLLs may be different.
#		Particularly, MS VS installer typycally installs Community/Pro/Enterprise version instead of "BuildTools"
#		Also version numbers may potentially be different.
_windows_container_redist_dlls_dir="C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Redist/MSVC/14.29.30133/x64/Microsoft.VC142.CRT"
.PHONY: windows/flutter-release
windows/flutter-release:
	flutter build windows
	cp -t build/windows/runner/Release \
		${_windows_container_redist_dlls_dir}/msvcp140.dll \
		${_windows_container_redist_dlls_dir}/vcruntime140.dll \
		${_windows_container_redist_dlls_dir}/vcruntime140_1.dll \
		${_windows_container_redist_dlls_dir}/msvcp140_codecvt_ids.dll

.PHONY: backend-bridge-windows
backend-bridge-windows:
	bazel build ${BAZEL_CACHE_ARG} ${_bazel_links_arg} --config=windows -c opt //flutter/cpp/flutter:backend_bridge.dll
	cd .. && chmod +w ${BAZEL_LINKS_DIR}bin/flutter/cpp/flutter/backend_bridge.dll
	mkdir -p build/win-dlls/
	rm -f build/win-dlls/backend_bridge.dll
	cd .. && cp ${BAZEL_LINKS_DIR}bin/flutter/cpp/flutter/backend_bridge.dll flutter/build/win-dlls/backend_bridge.dll

.PHONY: backend-bridge-android
backend-bridge-android:
	bazel build ${BAZEL_CACHE_ARG} ${_bazel_links_arg} --config=android_arm64 -c opt //flutter/cpp/flutter:libbackendbridge.so
	cd .. && chmod +w ${BAZEL_LINKS_DIR}bin/flutter/cpp/flutter/libbackendbridge.so
	mkdir -p android/app/src/main/jniLibs/arm64-v8a
	rm -f android/flutter/app/src/main/jniLibs/arm64-v8a/libbackendbridge.so
	cd .. && cp ${BAZEL_LINKS_DIR}bin/flutter/cpp/flutter/libbackendbridge.so flutter/android/app/src/main/jniLibs/arm64-v8a/libbackendbridge.so

# Use the following block as a template to add a new Windows backend
#.PHONY: backends/example-windows
#backends/example-windows:
#	bazel build ${_bazel_links_arg} --config=windows -c opt //mobile_back_example:examplebackenddll
#	cd .. && chmod +w ${BAZEL_LINKS_DIR}bin/mobile_back_example/cpp/backend_example/libexamplebackend.dll
#	mkdir -p build/win-dlls/backends
#	rm -f build/win-dlls/backends/libexamplebackend.dll
#	cd .. && cp ${BAZEL_LINKS_DIR}bin/mobile_back_example/cpp/backend_example/libexamplebackend.dll flutter/build/win-dlls/backends/libexamplebackend.dll

.PHONY: backends/tflite-windows
backends/tflite-windows:
	bazel build ${BAZEL_CACHE_ARG} ${_bazel_links_arg} --config=windows -c opt //mobile_back_tflite:tflitebackenddll
	cd .. && chmod +w ${BAZEL_LINKS_DIR}bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.dll
	mkdir -p build/win-dlls/backends
	rm -f build/win-dlls/backends/libtflitebackend.dll
	cd .. && cp ${BAZEL_LINKS_DIR}bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.dll flutter/build/win-dlls/backends/libtflitebackend.dll

# Use the following block as a template to add a new Android backend
#.PHONY: backends/example-android
#backends/example-android:
#	bazel build ${_bazel_links_arg} --config=android_arm64 -c opt //mobile_back_example:examplebackend
#	cd .. && chmod +w ${BAZEL_LINKS_DIR}bin/mobile_back_example/cpp/backend_example/libexamplebackend.so
#	mkdir -p android/app/src/main/jniLibs/arm64-v8a
#	rm -f android/app/src/main/jniLibs/arm64-v8a/libexamplebackend.so
#	cd .. && cp ${BAZEL_LINKS_DIR}bin/mobile_back_examplet/cpp/backend_example/libexamplebackend.so flutter/android/app/src/main/jniLibs/arm64-v8a/libexamplebackend.so

.PHONY: backends/tflite-android
backends/tflite-android:
	bazel build ${BAZEL_CACHE_ARG} ${_bazel_links_arg} --config=android_arm64 -c opt //mobile_back_tflite:tflitebackend
	cd .. && chmod +w ${BAZEL_LINKS_DIR}bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so
	mkdir -p android/app/src/main/jniLibs/arm64-v8a
	rm -f android/app/src/main/jniLibs/arm64-v8a/libtflitebackend.so
	cd .. && cp ${BAZEL_LINKS_DIR}bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so flutter/android/app/src/main/jniLibs/arm64-v8a/libtflitebackend.so

# To add a new vendor backend, copy and uncomment this block and replace "EXAMPLE" and "example" with the vendor name (e.g. INTEL and intel)
# See tflite backend below as an example
#ifeq (${ENABLE_BACKEND_EXAMPLE},0)
#backend_replace_example=
#else
#backend_replace_example=\'libexamplebackend\',
#endif

ifeq (${ENABLE_BACKEND_TFLITE},0)
backend_replace_tflite=
else
backend_replace_tflite=\'libtflitebackend\',
endif

# To add a new backend, add a new line to the sed command for your backend replacing "example" with the vendor name. For instance, for Intel it would be:
#               -e "s/INTEL_TAG/${backend_replace_intel}/" \
.PHONY: set-supported-backends
set-supported-backends:
	cat lib/backend/backends_list.in | sed \
		-e "s/EXAMPLE_TAG/${backend_replace_example}/" \
		-e "s/TFLITE_TAG/${backend_replace_tflite}/" \
		> lib/backend/backends_list.gen.dart

.PHONY: protobuf
protobuf:
	dart pub get
	rm -rf lib/protos
	mkdir -p lib/protos
	protoc --proto_path ../android/cpp/proto \
		--dart_out lib/protos \
		../android/cpp/proto/*.proto

.PHONY: update-splash-screen
update-splash-screen:
	tool/update-splash-screen

.PHONY: generate-localizations
generate-localizations:
	flutter gen-l10n \
		--arb-dir=lib/resources \
		--output-dir=lib/localizations \
		--template-arb-file=app_en.arb \
		--output-localization-file=app_localizations.dart \
		--no-synthetic-package

.PHONY: prepare-flutter
prepare-flutter: set-supported-backends protobuf generate-localizations
