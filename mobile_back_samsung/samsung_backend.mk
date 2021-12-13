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
  ANDROID_SAMSUNG_BACKEND_BAZEL_FLAG=--//android/java/org/mlperf/inference:with_samsung="1"

  ifeq (${MOBILE_BACK_SAMSUNG_ROOT},)
    $(error MOBILE_BACK_SAMSUNG_ROOT env must be defined when building with samsung backend)
  endif
  backend_samsung_android_files=${MOBILE_BACK_SAMSUNG_ROOT}/lib/libsamsungbackend.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libeden_model.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libeden_profiler.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libeden_rt.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libeden_ud_gpu.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libeden_ud_cpu.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libeden_ud_npu.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libeden_nn.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libeden_osal.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libeden_xtool.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libc++.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libeden_nn_on_system.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libutils.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libcutils.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libvndksupport.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libbase.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libcgrouprc.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libhidlbase.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libgpu_boost_vendor.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libprocessgroup.so \
    ${MOBILE_BACK_SAMSUNG_ROOT}/lib/libvendor_samsung_slsi_hardware_gpu_boost.so \
  # Samsung backend is prebuilt, so we don't need any bazel target for it
  backend_samsung_android_target=
  backend_samsung_filename=libsamsungbackend
endif
