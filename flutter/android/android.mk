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
flutter/android/libs: flutter/android/libs/deps flutter/android/libs/checksum flutter/android/libs/build flutter/android/libs/copy
# run `make flutter/android/apk` before `flutter/android/test-apk`
flutter/android/test-apk: flutter/android/test-apk/main flutter/android/test-apk/helper

.PHONY: flutter/android/libs/checksum
flutter/android/libs/checksum:
ifeq (${WITH_SAMSUNG},1)
	@echo "Validate checksum of Samsung lib files"
	flutter/tool/validate-checksum.sh \
		-d ${backend_samsung_lib_root} \
		-f ${backend_samsung_checksum_file}
endif
ifeq (${WITH_QTI},1)
ifeq (${WITH_STABLEDIFFUSION},1)
	@echo "Validate checksum of QTI lib files"
	flutter/tool/validate-checksum.sh \
		-d ${backend_qti_lib_root} \
		-f ${backend_qti_checksum_file}
endif
endif
ifeq (${WITH_MEDIATEK},1)
	$(MAKE) decode_so
	@echo "Validate checksum of Mediatek lib files"
	flutter/tool/validate-checksum.sh \
		-d ${backend_mediatek_lib_root} \
		-f ${backend_mediatek_checksum_file}
endif

.PHONY: flutter/android/libs/deps
flutter/android/libs/deps:
	${backend_qti_libs_deps}

.PHONY: flutter/android/libs/build
flutter/android/libs/build:
	bazel ${BAZEL_OUTPUT_ROOT_ARG} ${proxy_bazel_args} ${sonar_bazel_startup_options} \
		build ${BAZEL_CACHE_ARG} ${bazel_links_arg} ${sonar_bazel_build_args} \
		--config=android_arm64 \
		--platforms=//platforms:android_arm64 \
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
# Dart symbols for Crashlytics. With FIREBASE_CRASHLYTICS_ENABLED=true the Dart
# AOT code (libapp.so) is built with --split-debug-info and its symbols are
# uploaded with the Firebase CLI, so native crashes in libapp.so and Dart stack
# traces are symbolicated in the Crashlytics console. The Crashlytics Gradle
# plugin uploads the symbols of the other native libraries; it does not handle
# Dart symbols.
# Each artifact gets its own folder (relative to flutter/), so the uploaded
# symbols are always the ones of the build just made. Keep these folders to
# symbolicate a stack trace offline with `flutter symbolize`.
# The upload (flutter/tool/upload-dart-symbols.sh) needs the Firebase CLI
# (FIREBASE_CLI, preinstalled in the Android Docker image) and
# FIREBASE_ANDROID_APP_ID. It does not need a Firebase login.
# Both variables are recursive so the gate is read when a recipe expands:
# FIREBASE_CRASHLYTICS_ENABLED may come from FIREBASE_ENV_FILE, which
# flutter.mk includes after this file.
# Ref: https://firebase.google.com/docs/crashlytics/flutter/get-deobfuscated-reports
FIREBASE_CLI?=firebase
FLUTTER_ANDROID_SYMBOLS_FOLDER?=build/app/outputs/symbols
flutter_android_split_debug_info_arg=$(if $(filter true,${FIREBASE_CRASHLYTICS_ENABLED}),\
--split-debug-info=${FLUTTER_ANDROID_SYMBOLS_FOLDER}/$(1))
flutter_android_upload_dart_symbols=$(if $(filter true,${FIREBASE_CRASHLYTICS_ENABLED}),\
bash flutter/tool/upload-dart-symbols.sh flutter/${FLUTTER_ANDROID_SYMBOLS_FOLDER}/$(1))

.PHONY: flutter/android/apk
flutter/android/apk:
	mkdir -p $$(dirname ${flutter_android_apk_release_path})
	cd flutter && ${_start_args} flutter --no-version-check build apk \
		${flutter_official_build_arg} \
		${flutter_firebase_crashlytics_arg} \
		$(call flutter_android_split_debug_info_arg,apk) \
		${flutter_build_number_arg}
	$(call flutter_android_upload_dart_symbols,apk)
	cp -f flutter/build/app/outputs/flutter-apk/app-release.apk ${flutter_android_apk_release_path}.apk
.PHONY: flutter/android/appbundle
flutter/android/appbundle:
	mkdir -p $$(dirname ${flutter_android_apk_release_path})
	cd flutter && ${_start_args} flutter --no-version-check build appbundle \
		${flutter_official_build_arg} \
		${flutter_firebase_crashlytics_arg} \
		$(call flutter_android_split_debug_info_arg,aab) \
		${flutter_build_number_arg}
	$(call flutter_android_upload_dart_symbols,aab)
	cp -f flutter/build/app/outputs/bundle/release/app-release.aab ${flutter_android_apk_release_path}.aab

# Flutter 3.44 strips dev_dependency plugins (integration_test) from the
# generated plugin registrant on release builds. CI runs a release
# `flutter build` immediately before building these test APKs, so the registrant
# no longer registers integration_test. The on-device test then cannot report
# its results (MissingPluginException on 'allTestsFinished'), and the
# BrowserStack session is marked failed even though every test passes.
# Regenerate the debug build files (registrant with integration_test) before
# assembling the test APKs. --config-only only rewrites the build files, it does
# not build an artifact. Ref: https://docs.flutter.dev/testing/integration-tests
.PHONY: flutter/android/test-apk/debug-plugins
flutter/android/test-apk/debug-plugins:
	cd flutter && ${_start_args} flutter --no-version-check build apk --debug --config-only

FLUTTER_ANDROID_APK_TEST_MAIN?=test-main.apk
flutter_android_apk_test_main_path=${FLUTTER_ANDROID_APK_FOLDER}/${FLUTTER_ANDROID_APK_TEST_MAIN}
.PHONY: flutter/android/test-apk/main
flutter/android/test-apk/main: flutter/android/test-apk/debug-plugins
	mkdir -p $$(dirname ${flutter_android_apk_test_main_path})
	flutter_android_apk_perf_test_arg=$$(printf PERF_TEST=${PERF_TEST} | base64) && \
	flutter_android_apk_benchmark_ids_arg=$$(printf BENCHMARK_IDS=${BENCHMARK_IDS} | base64) && \
		cd flutter/android && \
		./gradlew app:assembleDebug \
		-Ptarget=integration_test/first_test.dart \
		-Pdart-defines=$${flutter_android_apk_perf_test_arg} \
		-Pdart-defines=$${flutter_android_apk_benchmark_ids_arg}
	cp -f flutter/build/app/outputs/apk/debug/app-debug.apk ${flutter_android_apk_test_main_path}

FLUTTER_ANDROID_APK_TEST_HELPER?=test-helper.apk
flutter_android_apk_test_helper_path=${FLUTTER_ANDROID_APK_FOLDER}/${FLUTTER_ANDROID_APK_TEST_HELPER}
.PHONY: flutter/android/test-apk/helper
flutter/android/test-apk/helper: flutter/android/test-apk/debug-plugins
	mkdir -p $$(dirname ${flutter_android_apk_test_helper_path})
	cd flutter/android && ./gradlew app:assembleAndroidTest
	cp -f flutter/build/app/outputs/apk/androidTest/debug/app-debug-androidTest.apk ${flutter_android_apk_test_helper_path}
