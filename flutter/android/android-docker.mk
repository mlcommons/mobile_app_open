# Copyright 2020-2022 The MLPerf Authors. All Rights Reserved.
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

DOCKER_IMAGE_TAG?=mlcommons/mlperf_mobile_flutter

flutter_docker_postfix=$(shell id -u)
.PHONY: flutter/android/docker/image
flutter/android/docker/image: output/docker/mlperf_mobile_flutter_android_${flutter_docker_postfix}.stamp
output/docker/mlperf_mobile_flutter_android_${flutter_docker_postfix}.stamp: flutter/android/docker/Dockerfile
	docker image build -t ${DOCKER_IMAGE_TAG} flutter/android/docker
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
		-v mlperf-mobile-flutter-cache-bazel-${flutter_docker_postfix}:/mnt/cache \
		--env WITH_TFLITE=${WITH_TFLITE} \
		--env WITH_QTI=${WITH_QTI} \
		--env WITH_SAMSUNG=${WITH_SAMSUNG} \
		--env WITH_PIXEL=${WITH_PIXEL} \
		--env WITH_MEDIATEK=${WITH_MEDIATEK} \
		--env BAZEL_ARGS_GLOBAL="${proxy_bazel_args} --output_user_root=/mnt/cache/bazel" \
		--env OFFICIAL_BUILD=${OFFICIAL_BUILD} \
		--env FLUTTER_BUILD_NUMBER=${FLUTTER_BUILD_NUMBER} \
		--env FLUTTER_FORCE_PUB_GET=1 \
		--env FLUTTER_DATA_FOLDER=${FLUTTER_DATA_FOLDER} \
		--env FLUTTER_CACHE_FOLDER=${FLUTTER_CACHE_FOLDER} \
		${proxy_docker_args} \
		${backend_qti_flutter_docker_args} \
		${backend_samsung_docker_args} \
		${DOCKER_IMAGE_TAG}

.PHONY: docker/flutter/android/libs
docker/flutter/android/libs: flutter/android/docker/image
	MSYS2_ARG_CONV_EXCL="*" docker run \
		${flutter_common_docker_flags} \
		make flutter/android/libs

.PHONY: docker/flutter/android/release
docker/flutter/android/release: flutter/check-release-env flutter/android/docker/image
	MSYS2_ARG_CONV_EXCL="*" docker run \
		${flutter_common_docker_flags} \
		make flutter/android/release
