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

.PHONY: flutter
ifeq (${OS},Windows_NT)
flutter: flutter/windows
else
ifeq ($(shell uname -s),Darwin)
flutter: flutter/ios
else
flutter: flutter/android
endif
endif

.PHONY: flutter/ios
flutter/ios: flutter/cpp-ios flutter/prepare-flutter flutter/update-splash-screen

BAZEL_LINKS_DIR=bazel-
_bazel_links_arg=--symlink_prefix ${BAZEL_LINKS_DIR} --experimental_no_product_name_out_symlink

.PHONY: flutter/cpp-ios
flutter/cpp-ios:
	@# NOTE: add `--copt -g` for debug info (but the resulting library would be 0.5 GiB)
	bazel ${BAZEL_OUTPUT_ROOT_ARG} build --config=ios_fat64 -c opt //flutter/cpp/flutter:ios_backend_fw_static

	rm -rf ${_xcode_fw}
	cp -a ${_bazel_ios_fw} ${_xcode_fw}

_bazel_ios_fw := bazel-bin/flutter/cpp/flutter/ios_backend_fw_static_archive-root/ios_backend_fw_static.framework
_xcode_fw := flutter/ios/Flutter/ios_backend_fw_static.framework

# To add a new backend, add flutter/backends/example-windows target (e.g. flutter/backends/intel-windows)
.PHONY: flutter/windows
flutter/windows: flutter/backend-bridge-windows flutter/backends/tflite-windows flutter/prepare-flutter

debug_flags_windows=-c dbg --copt /Od --copt /Z7 --linkopt -debug

.PHONY: flutter/android
flutter/android: flutter/backend-bridge-android flutter/backends/tflite-android flutter/prepare-flutter

.PHONY: flutter/android/apk
flutter/android/apk: flutter/android
	cd flutter && flutter clean
	@# take results from flutter/build/app/outputs/flutter-apk/app-release.apk
	cd flutter && flutter build apk

.PHONY: ci/flutter/android/test_apk
ci/flutter/android/test_apk: flutter/android/apk
	@# take results from flutter/build/app/outputs/apk/androidTest/debug/app-debug-androidTest.apk
	cd flutter/android && ./gradlew app:assembleAndroidTest
	@# take results from flutter/build/app/outputs/apk/debug/app-debug.apk
	cd flutter/android && ./gradlew app:assembleDebug -Ptarget=integration_test/first_test.dart

.PHONY: flutter/docker/android/apk
flutter/docker/android/apk: flutter/android/docker/image
	@# if the build fails with java.io.IOException: Input/output error
	@# remove file flutter/android/gradle/wrapper/gradle-wrapper.jar
	MSYS2_ARG_CONV_EXCL="*" docker run --rm -it --user `id -u`:`id -g` -v $(CURDIR):/mnt/project mlcommons/mlperf_mobile_flutter bash -c "cd /mnt/project && make flutter/android/apk"

.PHONY: flutter/android/docker/image
flutter/android/docker/image: flutter/build/docker/mlperf_mobile_flutter.stamp

flutter/build/docker/mlperf_mobile_flutter.stamp: flutter/android/docker/Dockerfile
	docker image build -t mlcommons/mlperf_mobile_flutter android/docker
	mkdir -p build/docker
	touch $@

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
	echo >output/container-script.bat "make BAZEL_LINKS_DIR=C:/bazel-links/ flutter/backend-bridge-windows flutter/backends/tflite-windows"
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
	flutter build windows
	cp -t build/windows/runner/Release \
		${_windows_container_redist_dlls_dir}/msvcp140.dll \
		${_windows_container_redist_dlls_dir}/vcruntime140.dll \
		${_windows_container_redist_dlls_dir}/vcruntime140_1.dll \
		${_windows_container_redist_dlls_dir}/msvcp140_codecvt_ids.dll

.PHONY: flutter/backend-bridge-windows
flutter/backend-bridge-windows:
	bazel build ${BAZEL_CACHE_ARG} ${_bazel_links_arg} --config=windows -c opt //flutter/cpp/flutter:backend_bridge.dll
	chmod +w ${BAZEL_LINKS_DIR}bin/flutter/cpp/flutter/backend_bridge.dll
	mkdir -p flutter/build/win-dlls/
	rm -f flutter/build/win-dlls/backend_bridge.dll
	cp ${BAZEL_LINKS_DIR}bin/flutter/cpp/flutter/backend_bridge.dll flutter/build/win-dlls/backend_bridge.dll

.PHONY: flutter/backend-bridge-android
flutter/backend-bridge-android:
	bazel build ${BAZEL_CACHE_ARG} ${_bazel_links_arg} --config=android_arm64 -c opt //flutter/cpp/flutter:libbackendbridge.so
	chmod +w ${BAZEL_LINKS_DIR}bin/flutter/cpp/flutter/libbackendbridge.so
	mkdir -p flutter/android/app/src/main/jniLibs/arm64-v8a
	rm -f flutter/android/app/src/main/jniLibs/arm64-v8a/libbackendbridge.so
	cp ${BAZEL_LINKS_DIR}bin/flutter/cpp/flutter/libbackendbridge.so flutter/android/app/src/main/jniLibs/arm64-v8a/libbackendbridge.so

# Use the following block as a template to add a new Windows backend
#.PHONY: backends/example-windows
#backends/example-windows:
#	bazel build ${_bazel_links_arg} --config=windows -c opt //mobile_back_example:examplebackenddll
#	chmod +w ${BAZEL_LINKS_DIR}bin/mobile_back_example/cpp/backend_example/libexamplebackend.dll
#	mkdir -p flutter/build/win-dlls/backends
#	rm -f flutter/build/win-dlls/backends/libexamplebackend.dll
#	cp ${BAZEL_LINKS_DIR}bin/mobile_back_example/cpp/backend_example/libexamplebackend.dll flutter/build/win-dlls/backends/libexamplebackend.dll

.PHONY: flutter/backends/tflite-windows
flutter/backends/tflite-windows:
	bazel build ${BAZEL_CACHE_ARG} ${_bazel_links_arg} --config=windows -c opt //mobile_back_tflite:tflitebackenddll
	chmod +w ${BAZEL_LINKS_DIR}bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.dll
	mkdir -p flutter/build/win-dlls/backends
	rm -f flutter/build/win-dlls/backends/libtflitebackend.dll
	cp ${BAZEL_LINKS_DIR}bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.dll flutter/build/win-dlls/backends/libtflitebackend.dll

# Use the following block as a template to add a new Android backend
#.PHONY: backends/example-android
#backends/example-android:
#	bazel build ${_bazel_links_arg} --config=android_arm64 -c opt //mobile_back_example:examplebackend
#	chmod +w ${BAZEL_LINKS_DIR}bin/mobile_back_example/cpp/backend_example/libexamplebackend.so
#	mkdir -p flutter/android/app/src/main/jniLibs/arm64-v8a
#	rm -f flutter/android/app/src/main/jniLibs/arm64-v8a/libexamplebackend.so
#	cp ${BAZEL_LINKS_DIR}bin/mobile_back_examplet/cpp/backend_example/libexamplebackend.so flutter/android/app/src/main/jniLibs/arm64-v8a/libexamplebackend.so

.PHONY: flutter/backends/tflite-android
flutter/backends/tflite-android:
	bazel build ${BAZEL_CACHE_ARG} ${_bazel_links_arg} --config=android_arm64 -c opt //mobile_back_tflite:tflitebackend
	chmod +w ${BAZEL_LINKS_DIR}bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so
	mkdir -p flutter/android/app/src/main/jniLibs/arm64-v8a
	rm -f flutter/android/app/src/main/jniLibs/arm64-v8a/libtflitebackend.so
	cp ${BAZEL_LINKS_DIR}bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so flutter/android/app/src/main/jniLibs/arm64-v8a/libtflitebackend.so

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
# -e "s/INTEL_TAG/${backend_replace_intel}/"
.PHONY: flutter/set-supported-backends
flutter/set-supported-backends:
	cat flutter/lib/backend/backends_list.in | sed \
		-e "s/EXAMPLE_TAG/${backend_replace_example}/" \
		-e "s/TFLITE_TAG/${backend_replace_tflite}/" \
		> flutter/lib/backend/backends_list.gen.dart

.PHONY: flutter/protobuf
flutter/protobuf:
	cd flutter && ${_start_args} dart pub get
	rm -rf flutter/lib/protos
	mkdir -p flutter/lib/protos
	protoc --proto_path android/cpp/proto \
		--dart_out flutter/lib/protos \
		android/cpp/proto/*.proto

.PHONY: flutter/update-splash-screen
flutter/update-splash-screen:
	cd flutter && tool/update-splash-screen

.PHONY: flutter/generate-localizations
flutter/generate-localizations:
	flutter gen-l10n \
		--arb-dir=flutter/lib/resources \
		--output-dir=flutter/lib/localizations \
		--template-arb-file=app_en.arb \
		--output-localization-file=app_localizations.dart \
		--no-synthetic-package

.PHONY: flutter/prepare-flutter
flutter/prepare-flutter: flutter/set-supported-backends flutter/protobuf flutter/generate-localizations
