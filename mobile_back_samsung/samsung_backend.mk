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
  android_samsung_backend_bazel_flag=--//android/java/org/mlperf/inference:with_samsung="1"

  ifeq (${MOBILE_BACK_SAMSUNG_LIB_ROOT},)
    $(error MOBILE_BACK_SAMSUNG_LIB_ROOT env must be defined when building with samsung backend)
  endif
  $(info MOBILE_BACK_SAMSUNG_LIB_ROOT=${MOBILE_BACK_SAMSUNG_LIB_ROOT})
  backend_samsung_docker_args=-v "${MOBILE_BACK_SAMSUNG_LIB_ROOT}:/mnt/samsung_backend" --env MOBILE_BACK_SAMSUNG_LIB_ROOT=/mnt/samsung_backend
  backend_samsung_android_files=${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libsamsungbackend.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libeden_model.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libeden_profiler.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libeden_rt.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libeden_ud_gpu.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libeden_ud_cpu.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libeden_ud_npu.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libeden_nn.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libeden_osal.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libeden_xtool.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libc++.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libeden_nn_on_system.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libutils.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libcutils.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libvndksupport.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libbase.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libcgrouprc.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libhidlbase.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libgpu_boost_vendor.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libprocessgroup.so \
    ${MOBILE_BACK_SAMSUNG_LIB_ROOT}/libvendor_samsung_slsi_hardware_gpu_boost.so \
  # Samsung backend is prebuilt, so we don't need any bazel targets for it
  backend_samsung_android_target=
  backend_samsung_filename=libsamsungbackend
endif
