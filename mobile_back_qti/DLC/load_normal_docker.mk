# Copyright (c) 2020-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp: \
	${TOPDIR}/datasets/docker/Dockerfile
	# Building mlperf_mobile docker
	docker image build -t mlperf_mobile:1.1 $(dir $(abspath ${TOPDIR}/datasets/docker/Dockerfile))
	mkdir -p ${DLCBUILDDIR}
	touch $@

${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp: \
	${TOPDIR}/mobile_back_qti/docker/mlperf_dlc_prepare/Dockerfile
    # Building mlperf_mobile docker
	docker image build -t mlperf_dlc_prepare $(dir $(abspath ${TOPDIR}/mobile_back_qti/docker/mlperf_dlc_prepare/Dockerfile))
	mkdir -p ${DLCBUILDDIR}
	touch $@
