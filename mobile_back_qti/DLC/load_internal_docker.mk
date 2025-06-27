# Copyright (c) 2020-2025 Qualcomm Innovation Center, Inc. All rights reserved.
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

QTI_DOCKER_IMAGE_PATH?=.
$(info QTI_DOCKER_IMAGE_PATH=${QTI_DOCKER_IMAGE_PATH})

${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp: \
    ${QTI_DOCKER_IMAGE_PATH}/mlperf_mobile_1_1_dlc.tar
	# Building mlperf_mobile docker
	docker load -i ${QTI_DOCKER_IMAGE_PATH}/mlperf_mobile_1_1_dlc.tar
	mkdir -p ${DLCBUILDDIR}
	touch $@

${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp: \
	${QTI_DOCKER_IMAGE_PATH}/mlperf_dlc_prepare.tar
    # Building mlperf_mobile docker
	docker load -i ${QTI_DOCKER_IMAGE_PATH}/mlperf_dlc_prepare.tar
	mkdir -p ${DLCBUILDDIR}
	touch $@

${DLCBUILDDIR}/mlperf_mosaic_docker.stamp: \
	${QTI_DOCKER_IMAGE_PATH}/mlperf_mosaic.tar
	# Building mlperf_mosaic docker
	docker load -i ${QTI_DOCKER_IMAGE_PATH}/mlperf_mosaic.tar
	mkdir -p ${DLCBUILDDIR}
	touch $@

${DLCBUILDDIR}/mlperf_snusr_docker.stamp: \
	${QTI_DOCKER_IMAGE_PATH}/mlperf_snusr.tar
	# Building snusr docker
	docker load -i ${QTI_DOCKER_IMAGE_PATH}/mlperf_snusr.tar
	mkdir -p ${DLCBUILDDIR}
	touch $@
