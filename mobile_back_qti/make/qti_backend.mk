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

.PHONY: snpe_version_check

ifeq (${USE_PROXY_WORKAROUND},1)
  export PROXY_WORKAROUND1=\
	-v /etc/ssl/certs:/etc/ssl/certs \
	-v /usr/share/ca-certificates:/usr/share/ca-certificates \
	-v /usr/share/ca-certificates-java:/usr/share/ca-certificates-java

  export PROXY_WORKAROUND2=--host_jvm_args -Djavax.net.ssl.trustStore=/etc/ssl/certs/java/cacerts
else
  export PROXY_WORKAROUND1=
  export PROXY_WORKAROUND2=
endif

# Set the included backends
WITH_QTI=0

ifeq (${WITH_QTI},1)
  ifneq (${SNPE_SDK},)
    QTI_BACKEND=--//android/java/org/mlperf/inference:with_qti="1"
    QTI_TARGET=//mobile_back_qti:qtibackend
    QTI_LIB_COPY=cp output/`readlink bazel-bin`/mobile_back_qti/cpp/backend_qti/libqtibackend.so output/binary/libqtibackend.so
    SNPE_VERSION=$(shell basename ${SNPE_SDK})
    QTI_SNPE_VERSION=$(shell grep SNPE_VERSION mobile_back_qti/variables.bzl | cut -d\" -f2)
    QTI_VOLUMES=-v ${SNPE_SDK}:/home/mlperf/mobile_app/mobile_back_qti/${SNPE_VERSION}
    QTI_DEPS=snpe_version_check
  else
    echo "ERROR: SNPE_SDK not set"
    QTI_BACKEND=--ERROR_SNPE_SDK_NOT_SET
    QTI_VOLUMES=
    QTI_DEPS=
    QTI_TARGET=
    QTI_LIB_COPY=true
  endif
else
  QTI_BACKEND=
  QTI_VOLUMES=
  QTI_DEPS=
  QTI_TARGET=
  QTI_LIB_COPY=true
endif

snpe_version_check:
	@[ ${SNPE_VERSION} = ${QTI_SNPE_VERSION} ] || (echo "SNPE_SDK (${SNPE_VERSION}) doesn't match required version (${QTI_SNPE_VERSION})" && false)

