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

BASE_DOCKER_FLAGS= \
                -e USER=mlperf \
                ${PROXY_WORKAROUND1} \
		-v $(CURDIR):/home/mlperf/mobile_app \
		-v $(CURDIR)/output/home/mlperf/cache:/home/mlperf/cache \
		-v $(CURDIR)/output/home/mlperf/flutter_cache:/home/mlperf/flutter_cache \
                -u `id -u`:`id -g`

COMMON_DOCKER_FLAGS0= \
		${BASE_DOCKER_FLAGS} \
                ${QTI_VOLUMES} \
		-w /home/mlperf/mobile_app \
		mlcommons/mlperf_mobile:1.0

COMMON_DOCKER_FLAGS1= \
		${COMMON_DOCKER_FLAGS0} \
		bazel-3.7.2

COMMON_DOCKER_FLAGS2= \
                ${PROXY_WORKAROUND2} \
                --output_user_root=/home/mlperf/cache/bazel build --verbose_failures \
		-c opt --cxxopt='--std=c++14' --host_cxxopt='--std=c++14'  \
                --host_cxxopt='-Wno-deprecated-declarations' --host_cxxopt='-Wno-class-memaccess' \
                --cxxopt='-Wno-deprecated-declarations' --cxxopt='-Wno-unknown-attributes'

COMMON_DOCKER_FLAGS= \
                ${COMMON_DOCKER_FLAGS1} \
                ${COMMON_DOCKER_FLAGS2}


NATIVE_DOCKER_FLAGS= \
                ${COMMON_DOCKER_FLAGS1} --bazelrc=/dev/null \
                ${COMMON_DOCKER_FLAGS2}

output/mlperf_mobile_docker_1_0.stamp: android/docker/mlperf_mobile/Dockerfile
	mkdir -p output/docker
	docker image build -t mlcommons/mlperf_mobile:1.0 android/docker/mlperf_mobile
	touch $@

.PHONY: docker_image bazel-version rundocker rundocker_root

docker_image: output/mlperf_mobile_docker_1_0.stamp

rundocker: output/mlperf_mobile_docker_1_0.stamp
	docker run -it \
                -e USER=mlperf \
                ${PROXY_WORKAROUND1} \
		-v $(CURDIR):/home/mlperf/mobile_app \
		-v $(CURDIR)/output/home/mlperf/git-certs:/home/mlperf/git-certs \
		-v $(CURDIR)/output/home/mlperf/cache:/home/mlperf/cache \
		-w /home/mlperf/mobile_app \
                -u `id -u`:`id -g` \
		mlcommons/mlperf_mobile:1.0

rundocker_root: output/mlperf_mobile_docker_1_0.stamp
	docker run -it \
                -e USER=mlperf \
		-v $(CURDIR):/home/mlperf/mobile_app \
		-v $(CURDIR)/output/home/mlperf/cache:/home/mlperf/cache \
		-w /home/mlperf/mobile_app \
		mlcommons/mlperf_mobile:1.0

