# Copyright 2020-2023 Samsung Electronics Co. LTD  All Rights Reserved.
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

  MOBILE_BACK_SAMSUNG_LIB_ROOT=$(shell echo mobile_back_samsung/samsung/lib* | awk '{print $$NF}')
  $(info MOBILE_BACK_SAMSUNG_LIB_ROOT=${MOBILE_BACK_SAMSUNG_LIB_ROOT})

  backend_samsung_android_files= \
    	${BAZEL_LINKS_PREFIX}bin/flutter/android/commonlibs/lib_arm64/libc++_shared.so \
        ${BAZEL_LINKS_PREFIX}bin/mobile_back_samsung/samsung/lib/libsamsungbackend.so \
		${MOBILE_BACK_SAMSUNG_LIB_ROOT}/internal/libenn_public_api_cpp.so \
		${MOBILE_BACK_SAMSUNG_LIB_ROOT}/internal/libenn_extension.so \
        ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/internal/libmbe2300_core.so \
        ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/internal/libmbe2400_core.so \
        ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/internal/libmbe2200_core.so \
        ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/internal/libmbe2100_core.so \
        ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/internal/libmbe1200_core.so \
        ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/internal/libc++.so \
        ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/internal/libeden_nn_on_system.so \

  # main binaries for Samsung backend are prebuilt
  backend_samsung_android_target= \
    //mobile_back_samsung/samsung/lib:samsungbackend \
    //flutter/android/commonlibs:commonlibs
  backend_samsung_filename=libsamsungbackend

  # variables needed to run make target flutter/android/libs/checksum
  backend_samsung_lib_root=${MOBILE_BACK_SAMSUNG_LIB_ROOT}
  backend_samsung_checksum_file=${MOBILE_BACK_SAMSUNG_LIB_ROOT}/checksums.txt
endif
