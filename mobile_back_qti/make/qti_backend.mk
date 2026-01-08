# Copyright 2020-2025 The MLPerf Authors. All Rights Reserved.
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
ifeq ($(WITH_QTI),)
  $(info WITH_QTI option is empty)
else ifeq ($(WITH_QTI),$(filter $(WITH_QTI),1 2))
  ifneq (${SNPE_SDK},)
    backend_qti_flutter_docker_args=-v "${SNPE_SDK}:/mnt/project/mobile_back_qti/$(shell basename ${SNPE_SDK})"
  endif
  $(info WITH_QTI=$(WITH_QTI))
  local_snpe_sdk_root=$(shell echo mobile_back_qti/qairt/* | awk '{print $$NF}')
  $(info detected SNPE SDK: ${local_snpe_sdk_root})
  backend_qti_android_files=${BAZEL_LINKS_PREFIX}bin/mobile_back_qti/cpp/backend_qti/libqtibackend.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libSNPE.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libSnpeHtpV75Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libSnpeHtpV73Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libSnpeHtpV69Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libSnpeHtpV68Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libSnpeHtpV79Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libSnpeHtpV81Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libSnpeHtpPrepare.so \
    ${local_snpe_sdk_root}/lib/hexagon-v75/unsigned/libSnpeHtpV75Skel.so \
    ${local_snpe_sdk_root}/lib/hexagon-v73/unsigned/libSnpeHtpV73Skel.so \
    ${local_snpe_sdk_root}/lib/hexagon-v69/unsigned/libSnpeHtpV69Skel.so \
    ${local_snpe_sdk_root}/lib/hexagon-v68/unsigned/libSnpeHtpV68Skel.so \
    ${local_snpe_sdk_root}/lib/hexagon-v79/unsigned/libSnpeHtpV79Skel.so \
    ${local_snpe_sdk_root}/lib/hexagon-v81/unsigned/libSnpeHtpV81Skel.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libQnnHtp.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libQnnHtpV73Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libQnnHtpV75Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libQnnHtpV79Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libQnnHtpV81Stub.so \
    ${local_snpe_sdk_root}/lib/hexagon-v73/unsigned/libQnnHtpV73Skel.so \
    ${local_snpe_sdk_root}/lib/hexagon-v75/unsigned/libQnnHtpV75Skel.so \
    ${local_snpe_sdk_root}/lib/hexagon-v79/unsigned/libQnnHtpV79Skel.so \
    ${local_snpe_sdk_root}/lib/hexagon-v81/unsigned/libQnnHtpV81Skel.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libQnnSystem.so \
    ${BAZEL_LINKS_PREFIX}bin/flutter/android/commonlibs/lib_arm64/libc++_shared.so
  backend_qti_cmdline_files=${BAZEL_LINKS_PREFIX}bin/mobile_back_qti/cpp/backend_qti/libqtibackend.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libSNPE.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libSnpeHtpV75Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libSnpeHtpV73Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libSnpeHtpV69Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libSnpeHtpV68Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libSnpeHtpV79Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libSnpeHtpV81Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libSnpeHtpPrepare.so \
    ${local_snpe_sdk_root}/lib/hexagon-v75/unsigned/libSnpeHtpV75Skel.so \
    ${local_snpe_sdk_root}/lib/hexagon-v73/unsigned/libSnpeHtpV73Skel.so \
    ${local_snpe_sdk_root}/lib/hexagon-v69/unsigned/libSnpeHtpV69Skel.so \
    ${local_snpe_sdk_root}/lib/hexagon-v68/unsigned/libSnpeHtpV68Skel.so \
    ${local_snpe_sdk_root}/lib/hexagon-v79/unsigned/libSnpeHtpV79Skel.so \
    ${local_snpe_sdk_root}/lib/hexagon-v81/unsigned/libSnpeHtpV81Skel.so \
    mobile_back_qti/run_mlperf_tests.sh \
    ${local_snpe_sdk_root}/lib/aarch64-android/libQnnHtp.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libQnnHtpV73Stub.so \
    ${local_snpe_sdk_root}/lib/hexagon-v73/unsigned/libQnnHtpV73Skel.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libQnnHtpV75Stub.so \
    ${local_snpe_sdk_root}/lib/hexagon-v75/unsigned/libQnnHtpV75Skel.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libQnnHtpV79Stub.so \
    ${local_snpe_sdk_root}/lib/hexagon-v79/unsigned/libQnnHtpV79Skel.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libQnnHtpV81Stub.so \
    ${local_snpe_sdk_root}/lib/hexagon-v81/unsigned/libQnnHtpV81Skel.so \
    ${local_snpe_sdk_root}/lib/aarch64-android/libQnnSystem.so \
    ${BAZEL_LINKS_PREFIX}bin/flutter/android/commonlibs/lib_arm64/libc++_shared.so

  backend_qti_android_target_sd=//mobile_back_qti/cpp/backend_qti/StableDiffusion:stableDiffusion
  backend_qti_android_target=//mobile_back_qti/cpp/backend_qti:libqtibackend.so \
                                 //flutter/android/commonlibs:commonlibs

  ifeq ($(EXTERNAL_CONFIG),1)
	backend_qti_flutter_docker_args = --env EXTERNAL_CONFIG=${EXTERNAL_CONFIG}
	backend_qti_android_target+=--//mobile_back_qti/cpp/backend_qti:external_config=${EXTERNAL_CONFIG}
  endif

  ifeq ($(WITH_STABLEDIFFUSION),1)
	backend_qti_libs_deps = rm -f ./mobile_back_qti/cpp/backend_qti/StableDiffusionShared/include/opencv && \
    						ln -s /opt/opencv-3.4.7_android/sdk/native mobile_back_qti/cpp/backend_qti/StableDiffusionShared/include/opencv
	backend_qti_flutter_docker_args = --env WITH_STABLEDIFFUSION=${WITH_STABLEDIFFUSION}
	backend_qti_android_target+=--//mobile_back_qti/cpp/backend_qti:with_stablediffusion=${WITH_STABLEDIFFUSION}
	backend_qti_cmdline_files+=mobile_back_qti/cpp/backend_qti/StableDiffusionShared/include/opencv/libs/arm64-v8a/libopencv_core.so \
                               mobile_back_qti/cpp/backend_qti/StableDiffusionShared/include/opencv/libs/arm64-v8a/libopencv_imgcodecs.so \
                               mobile_back_qti/cpp/backend_qti/StableDiffusionShared/include/opencv/libs/arm64-v8a/libopencv_imgproc.so \
                               mobile_back_qti/cpp/backend_qti/StableDiffusionShared/libStableDiffusion.so
	backend_qti_android_files+=mobile_back_qti/cpp/backend_qti/StableDiffusionShared/include/opencv/libs/arm64-v8a/libopencv_core.so \
                               mobile_back_qti/cpp/backend_qti/StableDiffusionShared/include/opencv/libs/arm64-v8a/libopencv_imgcodecs.so \
                               mobile_back_qti/cpp/backend_qti/StableDiffusionShared/include/opencv/libs/arm64-v8a/libopencv_imgproc.so \
                               mobile_back_qti/cpp/backend_qti/StableDiffusionShared/libStableDiffusion.so
  endif

  backend_qti_windows_files=${BAZEL_LINKS_PREFIX}bin/mobile_back_qti/cpp/backend_qti/libqtibackend.dll \
    ${BAZEL_LINKS_PREFIX}bin/mobile_back_qti/cpp/backend_qti/libqtibackend.pdb \
    ${local_snpe_sdk_root}/lib/aarch64-windows-msvc/SNPE.dll \
    ${local_snpe_sdk_root}/lib/aarch64-windows-msvc/SnpeHtpV68Stub.dll \
    ${local_snpe_sdk_root}/lib/aarch64-windows-msvc/SnpeHtpV73Stub.dll \
    ${local_snpe_sdk_root}/lib/aarch64-windows-msvc/SnpeHtpPrepare.dll \
    ${local_snpe_sdk_root}/lib/hexagon-v68/unsigned/libSnpeHtpV68Skel.so \
    ${local_snpe_sdk_root}/lib/hexagon-v73/unsigned/libSnpeHtpV73Skel.so \
    ${local_snpe_sdk_root}/lib/hexagon-v73/unsigned/libsnpehtpv73.cat \
    mobile_back_qti/run_mlperf_tests.bat
  backend_qti_windows_target=//mobile_back_qti/cpp/backend_qti:libqtibackend.dll
  backend_qti_filename=libqtibackend

  ifeq ($(WITH_QTI),2)
  	backend_qti_android_target+=--//mobile_back_qti/cpp/backend_qti:with_qti=${WITH_QTI}
  	backend_qti_windows_target+=--//mobile_back_qti/cpp/backend_qti:with_qti=${WITH_QTI}
  endif

	# variables needed to run make target flutter/android/libs/checksum
	backend_qti_lib_root=mobile_back_qti
	backend_qti_checksum_file=mobile_back_qti/checksums.txt
endif

QTI_DOCKER_IMAGE_PATH?=.
.PHONY: flutter/android/docker/qti/image
flutter/android/docker/qti/image: output/docker/mlperf_mobile_flutter_qti_android_${user_id}.stamp
output/docker/mlperf_mobile_flutter_qti_android_${user_id}.stamp: ${QTI_DOCKER_IMAGE_PATH}/mlperf_mobile_flutter.tar
	docker load -i ${QTI_DOCKER_IMAGE_PATH}/mlperf_mobile_flutter.tar
	mkdir -p output/docker
	touch $@

.PHONY: docker/flutter/android/qti/release
docker/flutter/android/qti/release: flutter/check-release-env flutter/android/docker/qti/image
	MSYS2_ARG_CONV_EXCL="*" docker run \
		${flutter_common_docker_flags} \
		make flutter/android/release


.PHONY: docker/cmdline/android/qti/release
docker/cmdline/android/qti/release: flutter/android/docker/qti/image
	MSYS2_ARG_CONV_EXCL="*" docker run \
		${flutter_common_docker_flags} \
		make cmdline/android/bins/release