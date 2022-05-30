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
  backend_qti_android_target=//mobile_back_qti/cpp/backend_qti:libqtibackend.so
endif
