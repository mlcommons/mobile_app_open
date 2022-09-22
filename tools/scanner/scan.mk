# Copyright 2020-2022 The MLPerf Authors. All Rights Reserved.
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

# Note:
# SonarScanner requires a clean build for every run. Do not cache build files.
# https://docs.sonarcloud.io/advanced-setup/languages/c-c-objective-c/#analysis-steps-using-build-wrapper

ifdef SONAR_OUT_DIR
	scanner_docker_args=\
		--env SONAR_OUT_DIR=${SONAR_OUT_DIR} \
		--env SONAR_TOKEN=${SONAR_TOKEN} \
		--env PR_NUMBER=${PR_NUMBER} \
		--env PR_BRANCH=${PR_BRANCH} \
		--env PR_BASE=${PR_BASE}

	# https://docs.sonarcloud.io/advanced-setup/languages/c-c-objective-c/#building-with-bazel
	sonar_bazel_startup_options=--batch
	sonar_bazel_build_args=--spawn_strategy=processwrapper-sandbox,local --strategy=Genrule=local
endif

# Use the same image tag used in `flutter_common_docker_flags`
output/docker_mlperf_scanner.stamp: flutter/android/docker/image tools/scanner/Dockerfile
	docker image build -t mlcommons/mlperf_mobile_flutter tools/scanner
	touch $@

.PHONY: scanner/image
scanner/image: output/docker_mlperf_scanner.stamp

.PHONY: scanner/build
scanner/build:
	bazel clean
	build-wrapper-linux-x86-64 --out-dir "${SONAR_OUT_DIR}" \
		make flutter/android

# TODO (anhappdev): Use MLCommons organization
.PHONY: scanner/scan
scanner/scan:
	sonar-scanner \
		-Dsonar.organization=anhappdev \
		-Dsonar.projectKey=mobile_app_open \
		-Dsonar.sources=. \
		-Dsonar.python.version="3.8, 3.9, 3.10" \
		-Dsonar.cfamily.build-wrapper-output="${SONAR_OUT_DIR}" \
		-Dsonar.host.url=https://sonarcloud.io \
		-Dsonar.scm.provider=git \
		-Dsonar.pullrequest.key=${PR_NUMBER} \
		-Dsonar.pullrequest.branch=${PR_BRANCH} \
		-Dsonar.pullrequest.base=${PR_BASE}

.PHONY: docker/scanner/build
docker/scanner/build:
	MSYS2_ARG_CONV_EXCL="*" docker run ${flutter_common_docker_flags} \
		make scanner/build

.PHONY: docker/scanner/scan
docker/scanner/scan:
	MSYS2_ARG_CONV_EXCL="*" docker run ${flutter_common_docker_flags} \
		make scanner/scan
