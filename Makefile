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

.PHONY: docker_image app bazel-version rundocker clean

all: app

include mobile_back_tflite/tflite_backend.mk
include mobile_back_pixel/pixel_backend.mk
include mobile_back_qti/make/qti_backend.mk


output/mlperf_mobile_docker_1_0.stamp: android/docker/mlperf_mobile/Dockerfile
	@mkdir -p output/docker
	@docker image build -t mlcommons/mlperf_mobile:1.0 android/docker/mlperf_mobile
	@touch $@

docker_image: output/mlperf_mobile_docker_1_0.stamp

BASE_DOCKER_FLAGS= \
                -e USER=mlperf \
                ${PROXY_WORKAROUND1} \
		-v $(CURDIR):/home/mlperf/mobile_app \
		-v $(CURDIR)/output/home/mlperf/cache:/home/mlperf/cache \
                -u `id -u`:`id -g`

COMMON_DOCKER_FLAGS0= \
		${BASE_DOCKER_FLAGS} \
                ${QTI_VOLUMES} \
		-w /home/mlperf/mobile_app \
		mlcommons/mlperf_mobile:1.0

COMMON_DOCKER_FLAGS1= \
		${COMMON_DOCKER_FLAGS0} \
		bazel-3.7.2

COMMON_DOCKER_FLAGS2= \
                ${PROXY_WORKAROUND2} \
                --output_user_root=/home/mlperf/cache/bazel build --verbose_failures \
		-c opt --cxxopt='--std=c++14' --host_cxxopt='--std=c++14'  \
                --host_cxxopt='-Wno-deprecated-declarations' --host_cxxopt='-Wno-class-memaccess' \
                --cxxopt='-Wno-deprecated-declarations' --cxxopt='-Wno-unknown-attributes'

COMMON_DOCKER_FLAGS= \
                ${COMMON_DOCKER_FLAGS1} \
                ${COMMON_DOCKER_FLAGS2}


NATIVE_DOCKER_FLAGS= \
                ${COMMON_DOCKER_FLAGS1} --bazelrc=/dev/null \
                ${COMMON_DOCKER_FLAGS2}

proto_test: output/mlperf_mobile_docker_1_0.stamp
	@echo "Building proto_test"
	@mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	@docker run \
		${NATIVE_DOCKER_FLAGS} --experimental_repo_remote_exec \
		//android/cpp/proto:proto_test
	@cp output/`readlink bazel-bin`/android/cpp/proto/proto_test output/proto_test
	@chmod 777 output/proto_test

main: output/mlperf_mobile_docker_1_0.stamp ${QTI_DEPS}
	@echo "Building main"
	@mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	@docker run \
		${COMMON_DOCKER_FLAGS} \
		--config android_arm64 //mobile_back_tflite:tflitebackend ${QTI_TARGET} //android/cpp/binary:main
	@rm -rf output/binary && mkdir -p output/binary
	@cp output/`readlink bazel-bin`/android/cpp/binary/main output/binary/main
	@cp output/`readlink bazel-bin`/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so output/binary/libtflitebackend.so
	@${QTI_LIB_COPY}
	@chmod 777 output/binary/main output/binary/libtflitebackend.so

libtflite: output/mlperf_mobile_docker_1_0.stamp
	@echo "Building libtflite"
	@mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	@docker run \
		${COMMON_DOCKER_FLAGS} \
		--config android_arm64 //mobile_back_tflite:tflitebackend
	@rm -rf output/binary && mkdir -p output/binary
	@cp output/`readlink bazel-bin`/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so output/binary/libtflitebackend.so
	@chmod 777 output/binary/libtflitebackend.so
	

app: output/mlperf_mobile_docker_1_0.stamp ${QTI_DEPS}
	@echo "Building mlperf_app.apk"
	@mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	@docker run \
		${COMMON_DOCKER_FLAGS} \
                ${QTI_BACKEND} ${SAMSUNG_BACKEND} ${MEDIATEK_BACKEND} ${PIXEL_BACKEND} \
		--fat_apk_cpu=arm64-v8a \
		//android/java/org/mlperf/inference:mlperf_app
	@cp output/`readlink bazel-bin`/android/java/org/mlperf/inference/mlperf_app.apk output/mlperf_app.apk
	@chmod 777 output/mlperf_app.apk

app_x86_64: output/mlperf_mobile_docker_1_0.stamp
	@echo "Building mlperf_app.apk"
	@mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	@docker run \
		${COMMON_DOCKER_FLAGS} \
                ${QTI_BACKEND} ${SAMSUNG_BACKEND} ${MEDIATEK_BACKEND} ${PIXEL_BACKEND} \
		--fat_apk_cpu=x86_64 \
		//android/java/org/mlperf/inference:mlperf_app
	@cp output/`readlink bazel-bin`/android/java/org/mlperf/inference/mlperf_app.apk output/mlperf_app_x86_64.apk
	@chmod 777 output/mlperf_app.apk

flutter_android:
	@echo "Building flutter app for android"
	@mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	@docker run \
		${BASE_DOCKER_FLAGS} \
		-e USE_PROXY_WORKAROUND=${USE_PROXY_WORKAROUND} \
		-w /home/mlperf/mobile_app/flutter \
		mlcommons/mlperf_mobile:1.0 make android

test_app: output/mlperf_mobile_docker_1_0.stamp
	@echo "Building mlperf_app.apk"
	@mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	@docker run \
		${COMMON_DOCKER_FLAGS} \
                ${QTI_BACKEND} ${SAMSUNG_BACKEND} ${MEDIATEK_BACKEND} ${PIXEL_BACKEND} \
		--fat_apk_cpu=x86_64,arm64-v8a \
		//androidTest:mlperf_test_app
	@cp output/`readlink bazel-bin`/android/androidTest/mlperf_test_app.apk output/mlperf_test_app.apk
	@chmod 777 output/mlperf_test_app.apk

rundocker: output/mlperf_mobile_docker_1_0.stamp
	@docker run -it \
                -e USER=mlperf \
                ${PROXY_WORKAROUND1} \
		-v $(CURDIR):/home/mlperf/mobile_app \
		-v $(CURDIR)/output/home/mlperf/cache:/home/mlperf/cache \
		-w /home/mlperf/mobile_app \
                -u `id -u`:`id -g` \
		mlcommons/mlperf_mobile:1.0

rundocker_root: output/mlperf_mobile_docker_1_0.stamp
	@docker run -it \
                -e USER=mlperf \
		-v $(CURDIR):/home/mlperf/mobile_app \
		-v $(CURDIR)/output/home/mlperf/cache:/home/mlperf/cache \
		-w /home/mlperf/mobile_app \
		mlcommons/mlperf_mobile:1.0

clean:
	@([ -d output/home/mlperf/cache ] && chmod -R +w output/home/mlperf/cache) || true
	@rm -rf output

ifeq (${WITH_QTI},1)
  include mobile_back_qti/make/qti_backend_targets.mk
endif


