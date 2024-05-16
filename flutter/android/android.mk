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

include flutter/android/android-docker.mk

ANDROID_NDK_VERSION?=25
ANDROID_NDK_API_LEVEL?=33

flutter/android: flutter/android/libs
flutter/android/release: flutter/check-release-env flutter/android flutter/prepare flutter/android/apk flutter/android/appbundle
flutter/android/libs: flutter/android/libs/checksum flutter/android/libs/build flutter/android/libs/copy
# run `make flutter/android/apk` before `flutter/android/test-apk`
flutter/android/test-apk: flutter/android/test-apk/main flutter/android/test-apk/helper

.PHONY: flutter/android/libs/checksum
flutter/android/libs/checksum:
ifeq (${WITH_SAMSUNG},1)
	@echo "Validate checksum of Samsung lib files"
	flutter/tool/validate-checksum.sh \
		-d ${backend_samsung_lib_root} \
		-f ${backend_samsung_checksum_file}
else
	@echo "Skip checksum validation"
endif

.PHONY: flutter/android/libs/build
flutter/android/libs/build:
	bazel ${BAZEL_OUTPUT_ROOT_ARG} ${proxy_bazel_args} ${sonar_bazel_startup_options} \
		build ${BAZEL_CACHE_ARG} ${bazel_links_arg} ${sonar_bazel_build_args} \
		--config=android_arm64 \
		${backend_tflite_android_target} \
		${backend_mediatek_android_target} \
		${backend_pixel_android_target} \
		${backend_qti_android_target} \
		${backend_samsung_android_target} \
		//flutter/cpp/flutter:libbackendbridge.so

flutter_android_libs_folder=flutter/android/app/src/main/jniLibs/arm64-v8a
.PHONY: flutter/android/libs/copy
flutter/android/libs/copy:
	rm -rf ${flutter_android_libs_folder}
	mkdir -p ${flutter_android_libs_folder}
	@# macos doesn't support --target-directory flag
	cp -f \
		${backend_tflite_android_files} \
		${backend_mediatek_android_files} \
		${backend_pixel_android_files} \
		${backend_qti_android_files} \
		${backend_samsung_android_files} \
		${BAZEL_LINKS_PREFIX}bin/flutter/cpp/flutter/libbackendbridge.so \
		${flutter_android_libs_folder}
	@# macos doesn't support --recursive flag
	chmod -R 777 ${flutter_android_libs_folder}

FLUTTER_ANDROID_APK_FOLDER?=output/android-apks

FLUTTER_ANDROID_APK_RELEASE?=$(shell flutter/tool/generate-apk-filename.sh)
flutter_android_apk_release_path=${FLUTTER_ANDROID_APK_FOLDER}/${FLUTTER_ANDROID_APK_RELEASE}
.PHONY: flutter/android/apk
flutter/android/apk:
	mkdir -p $$(dirname ${flutter_android_apk_release_path})
	cd flutter && ${_start_args} flutter --no-version-check build apk \
		${flutter_official_build_arg} \
		${flutter_firebase_crashlytics_arg} \
		${flutter_build_number_arg} \
		${flutter_folder_args}
	cp -f flutter/build/app/outputs/flutter-apk/app-release.apk ${flutter_android_apk_release_path}.apk
.PHONY: flutter/android/appbundle
flutter/android/appbundle:
	mkdir -p $$(dirname ${flutter_android_apk_release_path})
	cd flutter && ${_start_args} flutter --no-version-check build appbundle \
		${flutter_official_build_arg} \
		${flutter_firebase_crashlytics_arg} \
		${flutter_build_number_arg} \
		${flutter_folder_args}
	cp -f flutter/build/app/outputs/bundle/release/app-release.aab ${flutter_android_apk_release_path}.aab

FLUTTER_ANDROID_APK_TEST_MAIN?=test-main.apk
flutter_android_apk_test_main_path=${FLUTTER_ANDROID_APK_FOLDER}/${FLUTTER_ANDROID_APK_TEST_MAIN}
.PHONY: flutter/android/test-apk/main
flutter/android/test-apk/main:
	mkdir -p $$(dirname ${flutter_android_apk_test_main_path})
	flutter_android_apk_test_perf_arg=$$(printf enable-perf-test=${PERF_TEST} | base64) && \
		cd flutter/android && \
		./gradlew app:assembleDebug \
		-Ptarget=integration_test/first_test.dart \
		-Pdart-defines=$${flutter_android_apk_test_perf_arg}
	cp -f flutter/build/app/outputs/apk/debug/app-debug.apk ${flutter_android_apk_test_main_path}

FLUTTER_ANDROID_APK_TEST_HELPER?=test-helper.apk
flutter_android_apk_test_helper_path=${FLUTTER_ANDROID_APK_FOLDER}/${FLUTTER_ANDROID_APK_TEST_HELPER}
.PHONY: flutter/android/test-apk/helper
flutter/android/test-apk/helper:
	mkdir -p $$(dirname ${flutter_android_apk_test_helper_path})
	cd flutter/android && ./gradlew app:assembleAndroidTest
	cp -f flutter/build/app/outputs/apk/androidTest/debug/app-debug-androidTest.apk ${flutter_android_apk_test_helper_path}
