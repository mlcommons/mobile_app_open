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

fwc_image_name=mlperf_mobile_flutter_windows
.PHONY: flutter/windows/docker/image
flutter/windows/docker/image:
	docker build -t ${fwc_image_name} flutter/windows/docker

.PHONY: flutter/windows/docker/--
flutter/windows/docker/--: flutter/windows/docker/image
	MSYS2_ARG_CONV_EXCL="*" docker run \
		-it \
		--rm \
		--memory 8G \
		--volume "$(CURDIR):C:/workdir" \
		--workdir "C:/workdir" \
		--env "BAZEL_CACHE_ARG=${BAZEL_CACHE_ARG}" \
		--env GOOGLE_APPLICATION_CREDENTIALS=${GOOGLE_APPLICATION_CREDENTIALS} \
		--env FLUTTER_FORCE_PUB_GET=1 \
		${fwc_image_name} \
		cmd

fwc_local_project=C:/project-local
fwc_msvc_dlls=C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Redist/MSVC/14.29.30133/x64/Microsoft.VC142.CRT
.PHONY: flutter/windows/ci
flutter/windows/ci:
	@[ -n "${FLUTTER_RELEASE_NAME}" ] || (echo FLUTTER_RELEASE_NAME env must be set; exit 1)

	# Only run `make flutter/windows/ci/*` inside the docker windows container
	make flutter/windows/ci/copy-mount-to-local
	# A work-around to have `pub get` not updating committed, generated files in CI env.
	flutter config --no-enable-windows-desktop
	cd ${fwc_local_project} && make \
		BAZEL_OUTPUT_ROOT_ARG=--output_user_root=C:/bazel-root/ \
		BAZEL_LINKS_PREFIX=C:/bazel-links/ \
		flutter/prepare \
		flutter/windows/libs
	flutter config --enable-windows-desktop
	cd ${fwc_local_project} && make \
		flutter/test/integration
	cd ${fwc_local_project} && make \
		"FLUTTER_MSVC_DLLS=${fwc_msvc_dlls}" \
		flutter/windows/release
	make \
		flutter/windows/ci/copy-release-from-local \
		flutter/windows/release/archive

# Flutter wants to create links in `flutter/windows/flutter/ephemeral/.plugin_symlinks`
# In Docker Windows containers it's impossible to create a link inside a mounted volume.
.PHONY: flutter/windows/ci/copy-mount-to-local
flutter/windows/ci/copy-mount-to-local:
	rm -rf ${fwc_local_project}
	mkdir -p ${fwc_local_project}
	cp --target-directory ${fwc_local_project} --parents --recursive \
		${GOOGLE_APPLICATION_CREDENTIALS} \
		`git ls-files`

.PHONY: flutter/windows/ci/copy-release-from-local
flutter/windows/ci/copy-release-from-local:
	@[ -n "${FLUTTER_RELEASE_NAME}" ] || (echo FLUTTER_RELEASE_NAME env must be set; exit 1)

	rm -rf ${flutter_windows_releases}/${FLUTTER_RELEASE_NAME}
	mkdir -p ${flutter_windows_releases}
	cp --target-directory ${flutter_windows_releases} --recursive \
		${fwc_local_project}/${flutter_windows_releases}/${FLUTTER_RELEASE_NAME}
