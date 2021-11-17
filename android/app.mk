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

#SAMSUNG_BACKEND=--//android/java/org/mlperf/inference:with_samsung="1"
SAMSUNG_BACKEND=

.PHONY: app main app_x86_64 test_app libtflite

output/proto_test: output/mlperf_mobile_docker_1_0.stamp
	echo "Building proto_test"
	mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	docker run \
		${NATIVE_DOCKER_FLAGS} --experimental_repo_remote_exec \
		//android/cpp/proto:proto_test
	cp output/`readlink bazel-bin`/android/cpp/proto/proto_test output/proto_test
	chmod 777 output/proto_test

main: output/mlperf_mobile_docker_1_0.stamp ${QTI_DEPS}
	echo "Building main"
	mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	docker run \
		${COMMON_DOCKER_FLAGS} \
		--config android_arm64 //mobile_back_tflite:tflitebackend ${QTI_TARGET} //android/cpp/binary:main
	rm -rf output/binary && mkdir -p output/binary
	cp output/`readlink bazel-bin`/android/cpp/binary/main output/binary/main
	cp output/`readlink bazel-bin`/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so output/binary/libtflitebackend.so
	${QTI_LIB_COPY}
	chmod 777 output/binary/main output/binary/libtflitebackend.so

libtflite: output/binary/libtflitebackend.so
output/binary/libtflitebackend.so: output/mlperf_mobile_docker_1_0.stamp
	echo "Building libtflite"
	mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	docker run \
		${COMMON_DOCKER_FLAGS} \
		--config android_arm64 //mobile_back_tflite:tflitebackend
	rm -rf output/binary && mkdir -p output/binary
	cp output/`readlink bazel-bin`/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so output/binary/libtflitebackend.so
	chmod 777 output/binary/libtflitebackend.so
	

app: output/mlperf_app.apk
output/mlperf_app.apk: output/mlperf_mobile_docker_1_0.stamp ${QTI_DEPS}
	echo "Building mlperf_app.apk"
	mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	docker run \
		${COMMON_DOCKER_FLAGS} \
                ${QTI_BACKEND} ${SAMSUNG_BACKEND} ${MEDIATEK_BACKEND} ${PIXEL_BACKEND} \
		--fat_apk_cpu=arm64-v8a \
		//android/java/org/mlperf/inference:mlperf_app
	cp output/`readlink bazel-bin`/android/java/org/mlperf/inference/mlperf_app.apk output/mlperf_app.apk
	chmod 777 output/mlperf_app.apk

app_x86_64: output/mlperf_app_x86_64.apk
output/mlperf_app_x86_64.apk: output/mlperf_mobile_docker_1_0.stamp
	echo "Building mlperf_app.apk"
	mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	docker run \
		${COMMON_DOCKER_FLAGS} \
                ${QTI_BACKEND} ${SAMSUNG_BACKEND} ${MEDIATEK_BACKEND} ${PIXEL_BACKEND} \
		--fat_apk_cpu=x86_64 \
		//android/java/org/mlperf/inference:mlperf_app
	cp output/`readlink bazel-bin`/android/java/org/mlperf/inference/mlperf_app.apk output/mlperf_app_x86_64.apk
	chmod 777 output/mlperf_app_x86_64.apk

test_app: output/mlperf_test_app.apk
output/mlperf_test_app.apk: output/mlperf_mobile_docker_1_0.stamp
	echo "Building mlperf_app.apk"
	mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	docker run \
		${COMMON_DOCKER_FLAGS} \
                ${QTI_BACKEND} ${SAMSUNG_BACKEND} ${MEDIATEK_BACKEND} ${PIXEL_BACKEND} \
		--fat_apk_cpu=x86_64,arm64-v8a \
		//androidTest:mlperf_test_app
	cp output/`readlink bazel-bin`/android/androidTest/mlperf_test_app.apk output/mlperf_test_app.apk
	chmod 777 output/mlperf_test_app.apk

ifeq (${WITH_QTI},1)
  include mobile_back_qti/make/qti_backend_targets.mk
endif

