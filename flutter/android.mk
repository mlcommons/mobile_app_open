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

.PHONY: flutter/android
flutter/android: flutter/backend-bridge-android flutter/backends/tflite-android flutter/prepare-flutter

.PHONY: flutter/android/docker/image
flutter/android/docker/image: output/docker/mlperf_mobile_flutter.stamp

output/docker/mlperf_mobile_flutter.stamp: flutter/android/docker/Dockerfile
	docker image build -t mlcommons/mlperf_mobile_flutter android/docker
	mkdir -p output/docker
	touch $@

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

.PHONY: flutter/backend-bridge-android
flutter/backend-bridge-android:
	bazel build ${BAZEL_CACHE_ARG} ${_bazel_links_arg} --config=android_arm64 -c opt //flutter/cpp/flutter:libbackendbridge.so
	chmod +w ${BAZEL_LINKS_DIR}bin/flutter/cpp/flutter/libbackendbridge.so
	mkdir -p flutter/android/app/src/main/jniLibs/arm64-v8a
	rm -f flutter/android/app/src/main/jniLibs/arm64-v8a/libbackendbridge.so
	cp ${BAZEL_LINKS_DIR}bin/flutter/cpp/flutter/libbackendbridge.so flutter/android/app/src/main/jniLibs/arm64-v8a/libbackendbridge.so

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
