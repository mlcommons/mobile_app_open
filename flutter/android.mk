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

flutter_docker_postfix=$(shell id -u)
.PHONY: flutter/android/docker/image
flutter/android/docker/image: output/docker/mlperf_mobile_flutter_android_${flutter_docker_postfix}.stamp
output/docker/mlperf_mobile_flutter_android_${flutter_docker_postfix}.stamp: flutter/android/docker/Dockerfile
	docker image build -t mlcommons/mlperf_mobile_flutter flutter/android/docker
	mkdir -p output/docker
	touch $@

# you can set env CONNECT_TTY_TO_DOCKER=0 to run commands in systems without tty, like jenkins
ifneq (${CONNECT_TTY_TO_DOCKER},0)
flutter_docker_tty_arg=-it
endif

flutter_common_docker_flags= \
		--rm \
		${flutter_docker_tty_arg} \
		--init \
		--user `id -u`:`id -g` \
		--env USER=mlperf \
		-v $(CURDIR):/mnt/project \
		--workdir /mnt/project \
		-v /mnt/project/flutter/build \
		-v mlperf-mobile-flutter-cache-workdir-${flutter_docker_postfix}:/image-workdir/.cache \
		-v mlperf-mobile-flutter-cache-bazel-${flutter_docker_postfix}:/mnt/cache \
		--env WITH_TFLITE=${WITH_TFLITE} \
		--env WITH_QTI=${WITH_QTI} \
		--env WITH_SAMSUNG=${WITH_SAMSUNG} \
		--env WITH_PIXEL=${WITH_PIXEL} \
		--env WITH_MEDIATEK=${WITH_MEDIATEK} \
		--env BAZEL_ARGS_GLOBAL="${proxy_bazel_args} --output_user_root=/mnt/cache/bazel" \
		${proxy_docker_args} \
		${backend_qti_flutter_docker_args} \
		${backend_samsung_docker_args} \
		mlcommons/mlperf_mobile_flutter

.PHONY: flutter/android/apk
flutter/android/apk: flutter/android
	cd flutter && flutter clean
	cd flutter && flutter build apk ${flutter_common_dart_flags}
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

flutter_android_libs_folder=flutter/android/app/src/main/jniLibs/arm64-v8a
.PHONY: flutter/android/libs
flutter/android/libs:
	bazel ${BAZEL_ARGS_GLOBAL} build ${BAZEL_CACHE_ARG} ${bazel_links_arg} \
		--config=android_arm64 \
		${backend_tflite_android_target} \
		${backend_mediatek_android_target} \
		${backend_pixel_android_target} \
		${backend_qti_android_target} \
		${backend_samsung_android_target} \
		//flutter/cpp/flutter:libbackendbridge.so
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

.PHONY: docker/flutter/android/libs
docker/flutter/android/libs: flutter/android/docker/image
	MSYS2_ARG_CONV_EXCL="*" docker run \
		${flutter_common_docker_flags} \
		make flutter/android/libs
