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

.PHONY: flutter/windows/docker/image
flutter/windows/docker/image:
	docker build -t mlperf_mobile_flutter:windows-1.0 flutter/windows/docker

autocopy_folder=output/auto-copy

ifneq (${GOOGLE_APPLICATION_CREDENTIALS},)
google_credentials_local_folder=${autocopy_folder}/google-credentials
google_credentials_container_path=${google_credentials_local_folder}/credentials.json
else
google_credentials_local_folder=
google_credentials_container_path=
endif
.PHONY: flutter/windows/docker/create-container
flutter/windows/docker/create-container:
	# docker rm -f mobile_app_flutter_windows_container
	mkdir -p output
	echo >output/container-script.bat
	if [ -n "${GOOGLE_APPLICATION_CREDENTIALS}" ]; then \
		rm -rf ${google_credentials_local_folder}; \
		mkdir -p ${google_credentials_local_folder}; \
		cp "${GOOGLE_APPLICATION_CREDENTIALS}" ${google_credentials_container_path}; \
	fi
	@# "-it" here is required to make the container killable by Ctrl+C signal.
	@# A lot of memory is required by bazel.
	@#		With default settings only 1 bazel job can run at a time, which, obviously, greatly slows down the build.
	@#		4G typically runs 6-7 jobs. 8G is enough for bazel to run 16+ jobs.
	@# Also a lot of memory is required by Flutter. When using 2G Flutter fails with out of memory error, so at least 4G is needed.
		# --name mobile_app_flutter_windows_container \
		# --detach \
		# ".\\output\\container-script.bat <NUL"
		# --env BAZEL_LINKS_PREFIX=C:/bazel-links/ \
	MSYS2_ARG_CONV_EXCL="*" docker run \
		-it \
		--rm \
		--memory 8G --cpus $$(( `nproc` - 1 )) \
		--volume "$(CURDIR):C:/mnt/project-parent" \
		--workdir "C:/mnt/project-parent/" \
		--env "BAZEL_CACHE_ARG=${BAZEL_CACHE_ARG}" \
		--env GOOGLE_APPLICATION_CREDENTIALS=${google_credentials_container_path} \
		--env BAZEL_LINKS_PREFIX=C:/bazel-links/ \
		mlperf_mobile_flutter:windows-1.0 \
		cmd
		# make flutter/prepare

.PHONY: flutter/windows/docker/cmd
flutter/windows/docker/cmd:
	echo >output/container-script.bat \
		"cmd"
	docker start -ai mobile_app_flutter_windows_container

# In Docker Windows containers it's impossible to create a link inside a mounted volume.
# Flutter wants to create links in `flutter/windows/flutter/ephemeral/.plugin_symlinks`
# so `flutter build windows` doesn't work in a folder mounted from host.
# We have to create a copy of the project outside of the mounted volume.
#
# We could run bazel in the original mounted folder and only copy the flutter folder
# but it wouldn't make much difference
container_local_project=C:/project-local
container_msvc_dlls=C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Redist/MSVC/14.29.30133/x64/Microsoft.VC142.CRT
release_dir_path=${autocopy_folder}/flutter-windows-release
.PHONY: flutter/windows/ci
flutter/windows/ci:
	# Only run `make flutter/windows/ci/*` inside the docker windows container
	make flutter/windows/ci/copy-mount-to-local
	cd ${container_local_project} && make \
		"FLUTTER_MSVC_DLLS=${container_msvc_dlls}" \
		flutter/prepare \
		flutter/windows/libs \
		flutter/windows/release
	make flutter/windows/ci/copy-release-from-local

.PHONY: flutter/windows/ci/copy-mount-to-local
flutter/windows/ci/copy-mount-to-local:
	rm -rf ${container_local_project}
	mkdir -p ${container_local_project}
	cp --target-directory ${container_local_project} --parents --recursive \
		${GOOGLE_APPLICATION_CREDENTIALS} \
		`git ls-files`

.PHONY: flutter/windows/ci/copy-release-from-local
flutter/windows/ci/copy-release-from-local:
	rm -rf ${release_dir_path}
	mkdir -p ${release_dir_path}
	cp --target-directory ${release_dir_path} --recursive \
		${container_local_project}/flutter/build/windows/runner/Release
