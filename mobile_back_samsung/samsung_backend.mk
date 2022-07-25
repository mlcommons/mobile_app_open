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

ifeq (${WITH_SAMSUNG},1)
  $(info WITH_SAMSUNG=1)

  MOBILE_BACK_SAMSUNG_LIB_ROOT=$(shell echo mobile_back_samsung/samsung_backend/lib* | awk '{print $$NF}')
  $(info MOBILE_BACK_SAMSUNG_LIB_ROOT=${MOBILE_BACK_SAMSUNG_LIB_ROOT})

  backend_samsung_android_files= \
    ${BAZEL_LINKS_PREFIX}bin/flutter/android/commonlibs/lib_arm64/libc++_shared.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libsamsungbackend.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libenn_public_api_cpp.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libsbe2200_core.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libsbe1200_core.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libc++.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libeden_nn_on_system.so \
  # main binaries for Samsung backend are prebuilt
  backend_samsung_android_target= \
    //mobile_back_samsung/samsung_backend/lib:libsamsungbackend.so \
    //flutter/android/commonlibs:commonlibs
  backend_samsung_filename=libsamsungbackend
endif
