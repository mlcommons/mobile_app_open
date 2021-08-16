# Copyright (c) 2020-2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

.PHONY: docker_image

${BUILDDIR}/mlperf_mobile_docker_1_0.stamp: ${APP_DIR}/android/docker/mlperf_mobile/Dockerfile
	@mkdir -p ${BUILDDIR}
	@docker image build -t mlcommons/mlperf_mobile:1.0 ${APPDIR}/android/docker/mlperf_mobile
	@touch $@

${BUILDDIR}/mlperf_mobiledet_docker_image.stamp: ${TOPDIR}/docker/mlperf_mobiledet/Dockerfile
	@mkdir -p ${BUILDDIR}
	@docker image build -t mlcommons/mlperf_mobiledet:0.1 ${TOPDIR}/docker/mlperf_mobiledet
	@touch $@

docker_image: ${BUILDDIR}/mlperf_mobile_docker_1_0.stamp

