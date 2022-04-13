# Copyright 2021 The MLPerf Authors. All Rights Reserved.
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

ifeq (${WITH_QTI},1)
  $(info WITH_QTI=1)
  ifneq (${SNPE_SDK},)
    backend_qti_android_docker_args=-v "${SNPE_SDK}:/home/mlperf/mobile_app/mobile_back_qti/$(shell basename ${SNPE_SDK})"
    backend_qti_flutter_docker_args=-v "${SNPE_SDK}:/mnt/project/mobile_back_qti/$(shell basename ${SNPE_SDK})"
  endif
  android_qti_backend_bazel_flag=--//android/java/org/mlperf/inference:with_qti="1"
  backend_qti_lib_copy=cp output/`readlink bazel-bin`/mobile_back_qti/cpp/backend_qti/libqtibackend.so output/binary/libqtibackend.so

  local_snpe_sdk_root=$(shell echo mobile_back_qti/snpe-* | awk '{print $$NF}')
  $(info detected SNPE SDK: ${local_snpe_sdk_root})
<<<<<<< HEAD
  backend_qti_android_files= \
    ${BAZEL_LINKS_PREFIX}bin/mobile_back_qti/cpp/backend_qti/libqtibackend.so \
    ${BAZEL_LINKS_PREFIX}bin/flutter/android/commonlibs/lib_arm64/libc++_shared.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang6.0/libSNPE.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang6.0/libhta.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang6.0/libsnpe_dsp_domains_v2.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang6.0/libsnpe_dsp_domains_v3.so \
    ${local_snpe_sdk_root}/lib/dsp/libsnpe_dsp_v66_domains_v2_skel.so \
    ${local_snpe_sdk_root}/lib/dsp/libsnpe_dsp_v68_domains_v3_skel.so
  backend_qti_android_target= \
    //mobile_back_qti/cpp/backend_qti:libqtibackend.so \
    //flutter/android/commonlibs:commonlibs
=======
  backend_qti_android_files=${BAZEL_LINKS_PREFIX}bin/mobile_back_qti/cpp/backend_qti/libqtibackend.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang8.0/libc++_shared.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang8.0/libSNPE.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang8.0/libhta.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang8.0/libsnpe_dsp_domains_v2.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang8.0/libsnpe_dsp_domains_v3.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang8.0/libSnpeHtpV69Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang8.0/libSnpeHtpV68Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang8.0/libSnpeHtpPrepare.so \
    ${local_snpe_sdk_root}/lib/dsp/libsnpe_dsp_v66_domains_v2_skel.so \
    ${local_snpe_sdk_root}/lib/dsp/libsnpe_dsp_v68_domains_v3_skel.so \
    ${local_snpe_sdk_root}/lib/dsp/libSnpeHtpV69Skel.so \
    ${local_snpe_sdk_root}/lib/dsp/libSnpeHtpV68Skel.so
  backend_qti_android_target=//mobile_back_qti/cpp/backend_qti:libqtibackend.so
>>>>>>> submission-v2.0-unified
  backend_qti_filename=libqtibackend
endif
