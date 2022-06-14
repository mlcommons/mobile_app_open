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

.PHONY: flutter/ios
flutter/ios: flutter/ios/libs flutter/update-splash-screen

# BAZEL_OUTPUT_ROOT_ARG is set on our Jenkins CI
bazel_ios_fw := /tmp/ios_backend_fw_static_archive-root/ios_backend_fw_static.framework
xcode_fw := flutter/ios/Flutter/ios_backend_fw_static.framework
.PHONY: flutter/ios/libs
flutter/ios/libs:
	@# NOTE: add `--copt -g` for debug info (but the resulting library would be 0.5 GiB)
	bazel ${BAZEL_OUTPUT_ROOT_ARG} build --config=ios_fat64 -c opt //flutter/cpp/flutter:ios_backend_fw_static

	rm -rf ${xcode_fw}
	cp -a ${bazel_ios_fw} ${xcode_fw}

.PHONY: flutter/ios/release
flutter/ios/release:
	@[ "${OFFICIAL_BUILD}" == "true" ] || [ "${OFFICIAL_BUILD}" == "false" ] \
		|| (echo OFFICIAL_BUILD env must be set to \"true\" or \"false\"; exit 1)
	@[ -n "${FLUTTER_BUILD_NUMBER}" ] || (echo FLUTTER_BUILD_NUMBER env must be set; exit 1)
	make flutter/ios flutter/prepare flutter/ios/ipa

.PHONY: flutter/ios/ipa
flutter/ios/ipa:
	@[ -n "${FLUTTER_BUILD_NUMBER}" ] || (echo FLUTTER_BUILD_NUMBER env must be set; exit 1)
	cd flutter && flutter --no-version-check clean
	cd flutter && flutter --no-version-check build \
		ipa \
		${flutter_official_build_flag} \
		--build-number ${FLUTTER_BUILD_NUMBER}
	mkdir -p output/flutter/ios/
	cp -rf flutter/build/ios/archive/Runner.xcarchive output/flutter/ios/release.xcarchive
