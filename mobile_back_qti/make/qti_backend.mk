# Copyright 2021-2023 The MLPerf Authors. All Rights Reserved.
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
  local_snpe_sdk_root=$(shell echo mobile_back_qti/snpe-* | awk '{print $$NF}')
  $(info detected SNPE SDK: ${local_snpe_sdk_root})
  backend_qti_android_files=${BAZEL_LINKS_PREFIX}bin/mobile_back_qti/cpp/backend_qti/libqtibackend.so \
    ${BAZEL_LINKS_PREFIX}bin/flutter/android/commonlibs/lib_arm64/libc++_shared.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang8.0/libSNPE.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang8.0/libSnpeHtpV73Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang8.0/libSnpeHtpV69Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang8.0/libSnpeHtpV68Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang8.0/libSnpeHtpPrepare.so \
    ${local_snpe_sdk_root}/lib/dsp/libSnpeHtpV73Skel.so \
    ${local_snpe_sdk_root}/lib/dsp/libSnpeHtpV69Skel.so \
    ${local_snpe_sdk_root}/lib/dsp/libSnpeHtpV68Skel.so
  backend_qti_cmdline_files=${BAZEL_LINKS_PREFIX}bin/mobile_back_qti/cpp/backend_qti/libqtibackend.so \
    ${BAZEL_LINKS_PREFIX}bin/flutter/android/commonlibs/lib_arm64/libc++_shared.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang8.0/libSNPE.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang8.0/libSnpeHtpV73Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang8.0/libSnpeHtpV69Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang8.0/libSnpeHtpV68Stub.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang8.0/libSnpeHtpPrepare.so \
    ${local_snpe_sdk_root}/lib/dsp/libSnpeHtpV73Skel.so \
    ${local_snpe_sdk_root}/lib/dsp/libSnpeHtpV69Skel.so \
    ${local_snpe_sdk_root}/lib/dsp/libSnpeHtpV68Skel.so \
    mobile_back_qti/run_accuracy_tests.sh \
    mobile_back_qti/run_performance_tests.sh
  backend_qti_android_target=//mobile_back_qti/cpp/backend_qti:libqtibackend.so \
                                 //flutter/android/commonlibs:commonlibs
  ifeq ($(WITH_QTI),2)
  	backend_qti_android_target+=--//mobile_back_qti/cpp/backend_qti:with_qti=${WITH_QTI}
  endif

  backend_qti_windows_files=${BAZEL_LINKS_PREFIX}bin/mobile_back_qti/cpp/backend_qti/libqtibackend.dll \
    ${local_snpe_sdk_root}/lib/aarch64-windows-vc19/SNPE.dll \
    ${local_snpe_sdk_root}/lib/aarch64-windows-vc19/SnpeHtpV68Stub.dll \
    ${local_snpe_sdk_root}/lib/dsp/libSnpeHtpV68Skel.so \
    mobile_back_qti/run_performance_tests.bat \
    mobile_back_qti/run_accuracy_tests.bat
  backend_qti_windows_target=//mobile_back_qti/cpp/backend_qti:libqtibackend.dll
  backend_qti_filename=libqtibackend
endif
