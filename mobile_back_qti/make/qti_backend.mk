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
  local_snpe_sdk_root=$(shell find ./mobile_back_qti/ -maxdepth 1 -name "snpe-*" -print -quit)
  ifeq (${local_snpe_sdk_root},)
    ifeq (${SNPE_SDK},)
      $(error SNPE SDK is not found. Define SNPE_SDK env or manually link/copy snpe-<version> folder into mobile_back_qti)
    endif
    ifneq ($(shell [ -e "${SNPE_SDK}" ] && echo exists),exists)
      $(error SNPE_SDK env is invalid: path doesn't exist: ${SNPE_SDK})
    endif
    ifeq ($(findstring snpe-,${SNPE_SDK}),)
      $(error SNPE_SDK env invalid: folder name must start with "snpe-": ${SNPE_SDK})
    endif
    snpe_version=$(shell basename ${SNPE_SDK})
    $(shell ln -s $(SNPE_SDK) ./mobile_back_qti/${snpe_version})
    local_snpe_sdk_root=./mobile_back_qti/${snpe_version}
  endif
  ifneq ($(shell [ -e "${local_snpe_sdk_root}" ] && echo exists),exists)
    $(error SNPE SDK does not exist. Did you change SNPE_SDK env?)
  endif
  $(info using snpe $(shell basename ${local_snpe_sdk_root}))
  is_snpe_symlink=$(shell [ -L "${local_snpe_sdk_root}" ] && [ -d "${local_snpe_sdk_root}" ] && echo symlink)
  ifeq (${is_snpe_symlink},symlink)
    snpe_target=$(shell readlink ${local_snpe_sdk_root})
    qti_volumes=-v ${snpe_target}:${snpe_target}
  endif
  android_qti_backend_bazel_flag=--//android/java/org/mlperf/inference:with_qti="1"
  qti_lib_copy=cp output/`readlink bazel-bin`/mobile_back_qti/cpp/backend_qti/libqtibackend.so output/binary/libqtibackend.so

  backend_qti_android_files=${BAZEL_LINKS_PREFIX}bin/mobile_back_qti/cpp/backend_mock_qti/libqtibackend.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang6.0/libSNPE.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang6.0/libhta.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang6.0/libsnpe_dsp_domains_v2.so \
    ${local_snpe_sdk_root}/lib/aarch64-android-clang6.0/libsnpe_dsp_domains_v3.so \
    ${local_snpe_sdk_root}/lib/dsp/libsnpe_dsp_v66_domains_v2_skel.so \
    ${local_snpe_sdk_root}/lib/dsp/libsnpe_dsp_v68_domains_v3_skel.so
  backend_qti_android_target=//mobile_back_qti/cpp/backend_qti:libqtibackend.so
  backend_qti_filename=libqtibackend
endif
