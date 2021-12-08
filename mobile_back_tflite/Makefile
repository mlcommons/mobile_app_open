# Copyright 2020-2021 The MLPerf Authors. All Rights Reserved.
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

this_mkfile := $(abspath $(lastword $(MAKEFILE_LIST)))
TOPDIR := $(shell dirname ${this_mkfile})
BUILDDIR=${TOPDIR}/build

USERID=$(shell id -u)
GROUPID=$(shell id -g)

SRCFILES=\
Makefile \
cpp/BUILD \
cpp/c/BUILD \
cpp/c/version_script.lds \
cpp/c/backend_c.h \
cpp/c/type.h \
cpp/backend_tflite/tflite_c.cc \
cpp/backend_tflite/tflite_settings.h \
cpp/backend_tflite/utils.h \
cpp/backend_tflite/BUILD

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

all: ${BUILDDIR}/lib_x86_64/libtflitebackend.so ${BUILDDIR}/lib_arm64/libtflitebackend.so

.PHONY: clean docker_image

USER_ID=$(shell id -u)

${BUILDDIR}/mlperf_mobile_image.stamp: ${TOPDIR}/docker/mlperf_mobile/Dockerfile
	mkdir -p ${BUILDDIR}
	docker image build -t mlcommons/mlperf_mobile:0.1 docker/mlperf_mobile
	touch $@

docker_image: ${BUILDDIR}/mlperf_mobile_image.stamp

${BUILDDIR}/lib_arm64/libtflitebackend.so: ${SRCFILES} docker_image
	№ Building arm64 libtflitebackend.so
	mkdir -p ${BUILDDIR}/lib_arm64
	mkdir -p ${BUILDDIR}/cache
	docker run \
	        -e USER=mlperf \
                ${PROXY_WORKAROUND1} \
		-v $(CURDIR):/tflite_backend \
		-v ${BUILDDIR}/cache:/cache \
		-w /tflite_backend \
	        -u ${USERID}:${GROUPID} \
		mlcommons/mlperf_mobile:0.1 bazel-3.7.2 ${PROXY_WORKAROUND2} \
                --output_user_root=/cache/bazel build \
		-c opt --cxxopt='--std=c++14' \
                --cxxopt='-Wno-deprecated-declarations' \
                --cxxopt='-Wno-unknown-attributes' \
                --host_cxxopt='--std=c++14'  \
                --host_cxxopt='-Wno-deprecated-declarations' \
                --host_cxxopt='-Wno-class-memaccess' \
		--config=android_arm64 \
		:tflitebackend
	cp build/`readlink bazel-out`/arm64-v8a-opt/bin/cpp/backend_tflite/libtflitebackend.so $@
	chmod 777 $@

${BUILDDIR}/lib_x86_64/libtflitebackend.so: ${SRCFILES} docker_image
	# Building x86_64 libtflitebackend.so
	mkdir -p ${BUILDDIR}/lib_x86_64
	mkdir -p ${BUILDDIR}/cache
	docker run \
	        -e USER=mlperf \
                ${PROXY_WORKAROUND1} \
		-v $(CURDIR):/tflite_backend \
		-v ${BUILDDIR}/cache:/cache \
		-w /tflite_backend \
	        -u ${USERID}:${GROUPID} \
		 mlcommons/mlperf_mobile:0.1 bazel-3.7.2 ${PROXY_WORKAROUND2} \
                --output_user_root=/cache/bazel build \
		-c opt --cxxopt='--std=c++14' \
                --cxxopt='-Wno-deprecated-declarations' \
                --cxxopt='-Wno-unknown-attributes' \
                --host_cxxopt='--std=c++14'  \
                --host_cxxopt='-Wno-deprecated-declarations' \
                --host_cxxopt='-Wno-class-memaccess' \
		--config=android_x86_64 \
		:tflitebackend
	cp build/`readlink bazel-out`/x86_64-opt/bin/cpp/backend_tflite/libtflitebackend.so $@
	chmod 777 $@

clean:
	rm -rf bazel-bin bazel-testlogs bazel-out bazel-tflite_backend
	rm -rf ${BUILDDIR}

