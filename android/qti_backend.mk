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

.PHONY: qti_backend_sync

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
#QTI_BRANCH=master
QTI_BRANCH=
TFLITE_BRANCH=main

ifneq (${QTI_BRANCH},)
  ifneq (${SNPE_SDK},)
    QTI_BACKEND=--//java/org/mlperf/inference:with_qti="1"
    QTI_STAMP=qti_backend/.stamp
    QTI_SYNC=qti_backend_sync
    SNPE_VERSION=$(shell basename ${SNPE_SDK})
    QTI_SNPE_VERSION=$(shell grep SNPE_VERSION qti_backend/variables.bzl | cut -d\" -f2)
    QTI_VOLUMES=-v ${SNPE_SDK}:/home/mlperf/mobile_app/qti_backend/${SNPE_VERSION}
  else
    echo "ERROR: SNPE_SDK not set"
    QTI_BACKEND=--ERROR_SNPE_SDK_NOT_SET
    QTI_STAMP=
    QTI_SYNC=
    QTI_VOLUMES=
  endif
else
  QTI_BACKEND=
  QTI_STAMP=
  QTI_SYNC=
  QTI_VOLUMES=
endif

qti_backend/.stamp:
	git clone -b ${QTI_BRANCH} https://github.com/mlcommons/mobile_back_qualcomm qti_backend
	touch $@

qti_backend_sync: qti_backend/.stamp
	[ ${SNPE_VERSION} = ${QTI_SNPE_VERSION} ] || (echo "SNPE_SDK is not ${QTI_SNPE_VERSION}" && fail)
	(cd qti_backend && [ `git branch` = ${QTI_BRANCH} ]) || (echo "Checked out mobile_back_qualcomm branch is not ${QTI_BRANCH}" && fail)
	cd qti_backend && git fetch && git pull

