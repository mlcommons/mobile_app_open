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

.PHONY: android/builder-image
android/builder-image: output/android_docker.stamp

output/android_docker.stamp: android/docker/mlperf_mobile/Dockerfile
	mkdir -p output/docker
	docker image build -t mlcommons/mlperf_mobile_android_builder android/docker/mlperf_mobile
	touch $@

android_common_docker_flags1= \
		-e USER=mlperf \
		--rm \
		--init \
		${proxy_docker_args} \
		-v $(CURDIR):/home/mlperf/mobile_app \
		-v $(CURDIR)/output/home/mlperf/cache:/home/mlperf/cache \
		${backend_qti_android_docker_args} \
		-w /home/mlperf/mobile_app \
		-u `id -u`:`id -g` \
		mlcommons/mlperf_mobile_android_builder bazel-5.0.0

android_common_docker_flags2= \
		${proxy_bazel_args} \
		--output_user_root=/home/mlperf/cache/bazel build --verbose_failures \
		-c opt --cxxopt='--std=c++14' --host_cxxopt='--std=c++14'  \
		--host_cxxopt='-Wno-deprecated-declarations' --host_cxxopt='-Wno-class-memaccess' \
		--cxxopt='-Wno-deprecated-declarations' --cxxopt='-Wno-unknown-attributes'

android_common_docker_flags= \
		${android_common_docker_flags1} \
		${android_common_docker_flags2}


android_native_docker_flags= \
		${android_common_docker_flags1} --bazelrc=/dev/null \
		${android_common_docker_flags2}

.PHONY: android/proto_test
android/proto_test: android/builder-image
	# Building proto_test
	mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	docker run \
		${android_native_docker_flags} --experimental_repo_remote_exec \
		//android/cpp/proto:proto_test
	cp output/`readlink bazel-bin`/android/cpp/proto/proto_test output/proto_test
	chmod 777 output/proto_test

.PHONY: android/main
android/main: android/builder-image
	# Building main
	mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	docker run \
		${android_common_docker_flags} \
		--config android_arm64 //mobile_back_tflite:tflitebackend ${backend_qti_android_target} //android/cpp/binary:main
	rm -rf output/binary && mkdir -p output/binary
	cp output/`readlink bazel-bin`/android/cpp/binary/main output/binary/main
	cp output/`readlink bazel-bin`/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so output/binary/libtflitebackend.so
	${backend_qti_lib_copy}
	chmod 777 output/binary/main output/binary/libtflitebackend.so

.PHONY: android/libtflite
android/libtflite: android/builder-image
	# Building libtflite
	mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	docker run \
		${android_common_docker_flags} \
		--config android_arm64 //mobile_back_tflite:tflitebackend
	rm -rf output/binary && mkdir -p output/binary
	cp output/`readlink bazel-bin`/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so output/binary/libtflitebackend.so
	chmod 777 output/binary/libtflitebackend.so

.PHONY: android/app
android/app: android/builder-image
	# Building mlperf_app.apk
	mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	docker run \
		${android_common_docker_flags} \
		${android_qti_backend_bazel_flag} \
		${android_samsung_backend_bazel_flag} \
		${android_mediatek_backend_bazel_flag} \
		${android_pixel_backend_bazel_flag} \
		--fat_apk_cpu=arm64-v8a \
		//android/java/org/mlperf/inference:mlperf_app
	cp output/`readlink bazel-bin`/android/java/org/mlperf/inference/mlperf_app.apk output/mlperf_app.apk
	chmod 777 output/mlperf_app.apk

.PHONY: android/app_x86_64
android/app_x86_64: android/builder-image
	# Building mlperf_app_x86_64.apk
	mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	docker run \
		${android_common_docker_flags} \
		${android_qti_backend_bazel_flag} \
		${android_samsung_backend_bazel_flag} \
		${android_mediatek_backend_bazel_flag} \
		${android_pixel_backend_bazel_flag} \
		--fat_apk_cpu=x86_64 \
		//android/java/org/mlperf/inference:mlperf_app
	cp output/`readlink bazel-bin`/android/java/org/mlperf/inference/mlperf_app.apk output/mlperf_app_x86_64.apk
	chmod 777 output/mlperf_app.apk

.PHONY: android/test_app
android/test_app: android/builder-image
	# Building mlperf_test_app.apk
	mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	docker run \
		${android_common_docker_flags} \
		${android_qti_backend_bazel_flag} \
		${android_samsung_backend_bazel_flag} \
		${android_mediatek_backend_bazel_flag} \
		${android_pixel_backend_bazel_flag} \
		--fat_apk_cpu=x86_64,arm64-v8a \
		//androidTest:mlperf_test_app
	cp output/`readlink bazel-bin`/android/androidTest/mlperf_test_app.apk output/mlperf_test_app.apk
	chmod 777 output/mlperf_test_app.apk

.PHONY: android/rundocker
android/rundocker: android/builder-image
	docker run -it \
		-e USER=mlperf \
		-v $(CURDIR):/home/mlperf/mobile_app \
		-v $(CURDIR)/output/home/mlperf/cache:/home/mlperf/cache \
		-w /home/mlperf/mobile_app \
		-u `id -u`:`id -g` \
		mlcommons/mlperf_mobile_android_builder

.PHONY: android/rundocker_root
android/rundocker_root: android/builder-image
	docker run -it \
		-e USER=mlperf \
		-v $(CURDIR):/home/mlperf/mobile_app \
		-v $(CURDIR)/output/home/mlperf/cache:/home/mlperf/cache \
		-w /home/mlperf/mobile_app \
		mlcommons/mlperf_mobile_android_builder
