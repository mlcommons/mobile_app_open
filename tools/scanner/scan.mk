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
	# env which needs to be passed into the docker container
	scanner_docker_args=\
		--env SONAR_OUT_DIR=${SONAR_OUT_DIR} \
		--env SONAR_TOKEN=${SONAR_TOKEN} \
		--env GITHUB_TOKEN=${GITHUB_TOKEN} \
		--env FIREBASE_PROJECT_ID=${FIREBASE_PROJECT_ID} \
		--env REPO_NAME=${REPO_NAME} \
		--env PR_NUMBER=${PR_NUMBER} \
		--env PR_BRANCH=${PR_BRANCH} \
		--env PR_BASE=${PR_BASE}

	# https://docs.sonarcloud.io/advanced-setup/languages/c-c-objective-c/#building-with-bazel
	sonar_bazel_startup_options=--batch
	sonar_bazel_build_args=--spawn_strategy=local --strategy=Genrule=local
endif


output/docker_mlperf_scanner.stamp:
	docker image build \
		-t ${DOCKER_IMAGE_TAG} \
		--build-arg BASE_DOCKER_IMAGE_TAG=${BASE_DOCKER_IMAGE_TAG} \
		tools/scanner
	touch $@

.PHONY: scanner/build-image
scanner/build-image: flutter/android/docker/image output/docker_mlperf_scanner.stamp

.PHONY: scanner/build-app
scanner/build-app:
	bazel clean
	build-wrapper-linux-x86-64 --out-dir "${SONAR_OUT_DIR}" \
		make flutter/android

.PHONY: scanner/scan
scanner/scan: scanner/build-app
	sonar-scanner \
		-Dsonar.organization=mlcommons \
		-Dsonar.projectKey=mlcommons_${REPO_NAME} \
		-Dsonar.projectVersion=${FLUTTER_APP_VERSION} \
		-Dsonar.sources=. \
		-Dsonar.exclusions=**/*.java \
		-Dsonar.python.version="3.8, 3.9, 3.10" \
		-Dsonar.cfamily.compile-commands="${SONAR_OUT_DIR}/compile_commands.json" \
		-Dsonar.cfamily.analysisCache.mode=server \
		-Dsonar.host.url=https://sonarcloud.io \
		-Dsonar.scm.provider=git \
		-Dsonar.pullrequest.provider=github \
		-Dsonar.pullrequest.github.endpoint=https://api.github.com/ \
		-Dsonar.pullrequest.github.repository=mlcommons/${REPO_NAME} \
		-Dsonar.pullrequest.github.token.secured=${GITHUB_TOKEN} \
		-Dsonar.pullrequest.key=${PR_NUMBER} \
		-Dsonar.pullrequest.branch=${PR_BRANCH} \
		-Dsonar.pullrequest.base=${PR_BASE}

.PHONY: docker/scanner/scan
docker/scanner/scan:
	MSYS2_ARG_CONV_EXCL="*" docker run \
		${scanner_docker_args} ${flutter_common_docker_flags} \
		make scanner/scan
