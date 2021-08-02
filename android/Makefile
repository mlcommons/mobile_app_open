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


#SAMSUNG_BACKEND=--//java/org/mlperf/inference:with_samsung="1"
SAMSUNG_BACKEND=

#MEDIATEK_BACKEND=--//java/org/mlperf/inference:with_mediatek="1"
MEDIATEK_BACKEND=

.PHONY: docker_image app bazel-version tflite_backend_sync rundocker clean

all: app

include qti_backend.mk


tflite_backend/.stamp:
	git clone -b ${TFLITE_BRANCH} https://github.com/mlcommons/mobile_back_tflite tflite_backend
	touch $@

tflite_backend_sync: tflite_backend/.stamp
	cd tflite_backend && git fetch && git pull

sync_all: tflite_backend_sync ${QTI_SYNC}

output/mlperf_mobile_docker_0_2.stamp: tflite_backend/.stamp
	@mkdir -p output/docker
	@docker image build -t mlcommons/mlperf_mobile:0.2 tflite_backend/docker/mlperf_mobile
	@touch $@

docker_image: output/mlperf_mobile_docker_0_2.stamp

COMMON_DOCKER_FLAGS1= \
                -e USER=mlperf \
                ${PROXY_WORKAROUND1} \
		-v $(CURDIR):/home/mlperf/mobile_app \
		-v $(CURDIR)/output/home/mlperf/cache:/home/mlperf/cache \
                ${QTI_VOLUMES} \
		-w /home/mlperf/mobile_app \
                -u `id -u`:`id -g` \
		mlcommons/mlperf_mobile:0.2 bazel-3.7.2

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

proto_test: docker_image tflite_backend/.stamp Makefile
	@echo "Building proto_test"
	@mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	@docker run \
		${NATIVE_DOCKER_FLAGS} --experimental_repo_remote_exec \
		//cpp/proto:proto_test
	@cp output/`readlink bazel-bin`/cpp/proto/proto_test output/proto_test
	@chmod 777 output/proto_test

main: docker_image tflite_backend/.stamp Makefile
	@echo "Building main"
	@mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	@docker run \
		${COMMON_DOCKER_FLAGS} \
		--config android_arm64 //cpp/binary:tflitebackend //cpp/binary:main
	@rm -rf output/binary && mkdir -p output/binary
	@cp output/`readlink bazel-bin`/cpp/binary/main output/binary/main
	@cp output/`readlink bazel-out`/android-arm64-v8a-opt/bin/external/tflitebackend/cpp/backend_tflite/libtflitebackend.so output/binary/libtflitebackend.so
	@chmod 777 output/binary/main output/binary/libtflitebackend.so

app: docker_image tflite_backend/.stamp ${QTI_STAMP} Makefile
	@echo "Building mlperf_app.apk"
	@mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	@docker run \
		${COMMON_DOCKER_FLAGS} \
                ${QTI_BACKEND} ${SAMSUNG_BACKEND} ${MEDIATEK_BACKEND} \
		--fat_apk_cpu=arm64-v8a \
		//java/org/mlperf/inference:mlperf_app
	@cp output/`readlink bazel-bin`/java/org/mlperf/inference/mlperf_app.apk output/mlperf_app.apk
	@chmod 777 output/mlperf_app.apk

app_x86_64: docker_image tflite_backend/.stamp ${QTI_STAMP} Makefile
	@echo "Building mlperf_app.apk"
	@mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	@docker run \
		${COMMON_DOCKER_FLAGS} \
                ${QTI_BACKEND} ${SAMSUNG_BACKEND} ${MEDIATEK_BACKEND} \
		--fat_apk_cpu=x86_64 \
		//java/org/mlperf/inference:mlperf_app
	@cp output/`readlink bazel-bin`/java/org/mlperf/inference/mlperf_app.apk output/mlperf_app_x86_64.apk
	@chmod 777 output/mlperf_app.apk

test_app: docker_image tflite_backend/.stamp
	@echo "Building mlperf_app.apk"
	@mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	@docker run \
		${COMMON_DOCKER_FLAGS} \
                ${QTI_BACKEND} ${SAMSUNG_BACKEND} ${MEDIATEK_BACKEND} \
		--fat_apk_cpu=x86_64,arm64-v8a \
		//androidTest:mlperf_test_app
	@cp output/`readlink bazel-bin`/androidTest/mlperf_test_app.apk output/mlperf_test_app.apk
	@chmod 777 output/mlperf_test_app.apk

rundocker: docker_image
	@docker run -it \
                -e USER=mlperf \
		-v $(CURDIR):/home/mlperf/mobile_app \
		-v $(CURDIR)/output/home/mlperf/cache:/home/mlperf/cache \
		-w /home/mlperf/mobile_app \
                -u `id -u`:`id -g` \
		mlcommons/mlperf_mobile:0.2

rundocker_root: docker_image
	@docker run -it \
                -e USER=mlperf \
		-v $(CURDIR):/home/mlperf/mobile_app \
		-v $(CURDIR)/output/home/mlperf/cache:/home/mlperf/cache \
		-w /home/mlperf/mobile_app \
		mlcommons/mlperf_mobile:0.2

clean:
	@([ -d output/home/mlperf/cache ] && chmod -R +w output/home/mlperf/cache) || true
	@rm -rf output

