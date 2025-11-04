# Copyright 2020-2024 The MLPerf Authors. All Rights Reserved.
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
user_id=$(shell id -u)

.PHONY: flutter/android/docker/image
flutter/android/docker/image: output/docker/mlperf_mobile_flutter_android_${user_id}.stamp
output/docker/mlperf_mobile_flutter_android_${user_id}.stamp: flutter/android/docker/Dockerfile
	DOCKER_BUILDKIT=1 docker buildx build --tag ${DOCKER_IMAGE_TAG} flutter/android/docker
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
		--workdir /image-workdir/project \
		-v $(CURDIR):/image-workdir/project \
		-v mlperf-mobile-flutter-cache-bazel-${user_id}:/image-workdir/cache/bazel \
		--env BAZEL_CACHE_ARG="--disk_cache=/image-workdir/cache/bazel" \
		--env WITH_TFLITE=${WITH_TFLITE} \
		--env USER_ID=${user_id} \
		--env WITH_QTI=${WITH_QTI} \
		--env WITH_SAMSUNG=${WITH_SAMSUNG} \
		--env WITH_PIXEL=${WITH_PIXEL} \
		--env WITH_MEDIATEK=${WITH_MEDIATEK} \
		--env proxy_bazel_args=${proxy_bazel_args} \
		--env BAZEL_OUTPUT_ROOT_ARG="--output_user_root=/image-workdir/cache/bazel" \
		--env OFFICIAL_BUILD=${OFFICIAL_BUILD} \
		--env FIREBASE_CRASHLYTICS_ENABLED=${FIREBASE_CRASHLYTICS_ENABLED} \
		--env FLUTTER_BUILD_NUMBER=${FLUTTER_BUILD_NUMBER} \
		--env FLUTTER_FORCE_PUB_GET=1 \
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

docker/flutter/clean: flutter/check-release-env
	MSYS2_ARG_CONV_EXCL="*" docker run \
		${flutter_common_docker_flags} \
		make flutter/clean
