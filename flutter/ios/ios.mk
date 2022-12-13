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

flutter/ios: flutter/ios/libs flutter/ios/clean flutter/update-splash-screen

backend_bridge_ios_target=//flutter/cpp/flutter:backend_bridge_fw
backend_bridge_ios_zip=${BAZEL_LINKS_PREFIX}bin/flutter/cpp/flutter/backend_bridge_fw.xcframework.zip

flutter_ios_fw_dir=flutter/ios/frameworks

.PHONY: flutter/ios/clean
flutter/ios/clean:
	rm -rf flutter/build/ios

# BAZEL_OUTPUT_ROOT_ARG is set on our Jenkins CI
.PHONY: flutter/ios/libs
flutter/ios/libs:
	# --use_top_level_targets_for_symlinks
	bazel ${BAZEL_OUTPUT_ROOT_ARG} build ${BAZEL_CACHE_ARG} \
		--config=ios \
		${backend_bridge_ios_target} \
		${backend_tflite_ios_target} \
		${backend_coreml_ios_target}

	rm -rf ${flutter_ios_fw_dir}
	unzip -q -o -d ${flutter_ios_fw_dir} ${backend_bridge_ios_zip}
	unzip -q -o -d ${flutter_ios_fw_dir} ${backend_tflite_ios_zip}
	unzip -q -o -d ${flutter_ios_fw_dir} ${backend_coreml_ios_zip}

flutter/ios/release: flutter/check-release-env flutter/ios flutter/prepare flutter/ios/ipa

.PHONY: flutter/ios/ipa
flutter/ios/ipa:
	@[ -n "${FLUTTER_BUILD_NUMBER}" ] || (echo FLUTTER_BUILD_NUMBER env must be set; exit 1)
	cd flutter && flutter --no-version-check clean
	cd flutter && flutter --no-version-check build \
		ipa \
		${flutter_official_build_arg} \
		${flutter_build_number_arg}
	mkdir -p output/flutter/ios/
	cp -rf flutter/build/ios/archive/Runner.xcarchive output/flutter/ios/release.xcarchive
