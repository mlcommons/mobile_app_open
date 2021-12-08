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
  ifeq (${SNPE_SDK},)
    $(error SNPE_SDK env is undefined)
  endif
  QTI_BACKEND=--//android/java/org/mlperf/inference:with_qti="1"
  QTI_TARGET=//mobile_back_qti:qtibackend
  QTI_LIB_COPY=cp output/`readlink bazel-bin`/mobile_back_qti/cpp/backend_qti/libqtibackend.so output/binary/libqtibackend.so
  SNPE_VERSION=$(shell basename ${SNPE_SDK})
  QTI_SNPE_VERSION=$(shell grep SNPE_VERSION mobile_back_qti/variables.bzl | cut -d\" -f2)
  QTI_VOLUMES=-v ${SNPE_SDK}:/home/mlperf/mobile_app/mobile_back_qti/${SNPE_VERSION}
  ifneq (${SNPE_VERSION},${QTI_SNPE_VERSION})
    $(error SNPE_SDK (${SNPE_VERSION}) doesn't match version specified in bazel config: (${QTI_SNPE_VERSION}))
  endif
endif
