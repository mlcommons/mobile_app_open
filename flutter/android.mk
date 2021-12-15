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
flutter/android/docker/image: output/docker/mlperf_mobile_flutter_android.stamp

# if someone tries to run the build on WSL and then on the host Windows machine,
# and the build uses the same volume for cache, the build will fail
ifeq (${OS},Windows_NT)
flutter_docker_volume_bazel=mlperf-mobile-flutter-cache-bazel-win
flutter_docker_volume_workdir=mlperf-mobile-flutter-cache-workdir-win
else
flutter_docker_volume_bazel=mlperf-mobile-flutter-cache-bazel
flutter_docker_volume_workdir=mlperf-mobile-flutter-cache-workdir
endif
output/docker/mlperf_mobile_flutter_android.stamp: flutter/android/docker/Dockerfile
	docker image build -t mlcommons/mlperf_mobile_flutter flutter/android/docker
	docker volume create ${flutter_docker_volume_bazel}
	docker volume create ${flutter_docker_volume_workdir}
	mkdir -p output/docker
	touch $@

flutter_common_docker_flags= \
		--rm \
		-it \
		--user `id -u`:`id -g` \
		--env USER=mlperf \
		-v $(CURDIR):/mnt/project \
		--workdir /mnt/project \
		-v /mnt/project/flutter/build \
		-v ${flutter_docker_volume_workdir}:/image-workdir/.cache \
		-v ${flutter_docker_volume_bazel}:/mnt/cache \
		--env BAZEL_ARGS_GLOBAL="${proxy_bazel_args} --output_user_root=/mnt/cache/bazel" \
		${proxy_docker_args} \
		${qti_docker_args} \
		mlcommons/mlperf_mobile_flutter

.PHONY: flutter/android/apk
flutter/android/apk: flutter/android
	cd flutter && flutter clean
	cd flutter && flutter build apk
	mkdir -p output/flutter/android/
	cp -f flutter/build/app/outputs/flutter-apk/app-release.apk output/flutter/android/release.apk

.PHONY: docker/flutter/android/apk
docker/flutter/android/apk: flutter/android/docker/image
	MSYS2_ARG_CONV_EXCL="*" docker run \
		${flutter_common_docker_flags} \
		make flutter/android/apk

.PHONY: flutter/android/test_apk
flutter/android/test_apk: flutter/android/apk
	cd flutter/android && ./gradlew app:assembleAndroidTest
	cd flutter/android && ./gradlew app:assembleDebug -Ptarget=integration_test/first_test.dart
	mkdir -p output/flutter/android/
	cp -f flutter/build/app/outputs/apk/androidTest/debug/app-debug-androidTest.apk output/flutter/android/test-helper.apk
	cp -f flutter/build/app/outputs/apk/debug/app-debug.apk output/flutter/android/test.apk

.PHONY: flutter/android/libs
flutter/android/libs:
	bazel ${BAZEL_ARGS_GLOBAL} build ${BAZEL_CACHE_ARG} ${bazel_links_arg} \
		--config=android_arm64 -c opt \
		${backend_tflite_android_target} \
		${backend_mediatek_android_target} \
		${backend_pixel_android_target} \
		${backend_qti_android_target} \
		${backend_samsung_android_target} \
		//flutter/cpp/flutter:libbackendbridge.so
	mkdir -p flutter/android/app/src/main/jniLibs/arm64-v8a
	cp -f --target-directory flutter/android/app/src/main/jniLibs/arm64-v8a \
		${backend_tflite_android_files} \
		${backend_mediatek_android_files} \
		${backend_pixel_android_files} \
		${backend_qti_android_files} \
		${backend_samsung_android_files} \
		${BAZEL_LINKS_PREFIX}bin/flutter/cpp/flutter/libbackendbridge.so

.PHONY: docker/flutter/android/libs
docker/flutter/android/libs:
	MSYS2_ARG_CONV_EXCL="*" docker run \
		${flutter_common_docker_flags} \
		make flutter/android/libs
