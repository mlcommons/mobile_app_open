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
flutter/android: flutter/android/libs flutter/prepare-flutter

.PHONY: flutter/android/docker/image
flutter/android/docker/image: output/docker/mlperf_mobile_flutter.stamp

output/docker/mlperf_mobile_flutter.stamp: flutter/android/docker/Dockerfile
	docker image build -t mlcommons/mlperf_mobile_flutter android/docker
	mkdir -p output/docker
	touch $@

.PHONY: flutter/android/apk
flutter/android/apk: flutter/android
	cd flutter && flutter clean
	cd flutter && flutter build apk
	@# take results from flutter/build/app/outputs/flutter-apk/app-release.apk

.PHONY: flutter/android/test_apk
flutter/android/test_apk: flutter/android/apk
	cd flutter/android && ./gradlew app:assembleAndroidTest
	cd flutter/android && ./gradlew app:assembleDebug -Ptarget=integration_test/first_test.dart
	# take result from flutter/build/app/outputs/apk/androidTest/debug/app-debug-androidTest.apk
	# take result from flutter/build/app/outputs/apk/debug/app-debug.apk

.PHONY: docker/flutter/android/apk
docker/flutter/android/apk: flutter/android/docker/image
	@# if the build fails with java.io.IOException: Input/output error
	@# remove file flutter/android/gradle/wrapper/gradle-wrapper.jar
	MSYS2_ARG_CONV_EXCL="*" docker run --rm -it --user `id -u`:`id -g` -v $(CURDIR):/mnt/project mlcommons/mlperf_mobile_flutter bash -c "cd /mnt/project && make flutter/android/apk"

.PHONY: flutter/android/libs
flutter/android/libs:
	bazel build ${BAZEL_CACHE_ARG} ${_bazel_links_arg} \
		--config=android_arm64 -c opt \
		${BACKEND_TFLITE_SO_TARGET} \
		${BACKEND_MEDIATEK_SO_TARGET} \
		${BACKEND_PIXEL_SO_TARGET} \
		${BACKEND_QTI_SO_TARGET} \
		${BACKEND_SAMSUNG_SO_TARGET} \
		//flutter/cpp/flutter:backend_bridge.dll
	mkdir -p flutter/android/app/src/main/jniLibs/arm64-v8a
	cp -f --target-directory flutter/android/app/src/main/jniLibs/arm64-v8a \
		${BACKEND_TFLITE_SO_FILE} \
		${BACKEND_MEDIATEK_SO_FILE} \
		${BACKEND_PIXEL_SO_FILE} \
		${BACKEND_QTI_SO_FILES} \
		${BACKEND_SAMSUNG_SO_FILES} \
		${BAZEL_LINKS_PREFIX}bin/flutter/cpp/flutter/libbackendbridge.so
