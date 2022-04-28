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
flutter/ios: flutter/ios/native flutter/update-splash-screen flutter/prepare

# BAZEL_OUTPUT_ROOT_ARG is set on our Jenkins CI
bazel_ios_fw := bazel-bin/flutter/cpp/flutter/ios_backend_fw_static_archive-root/ios_backend_fw_static.framework
xcode_fw := flutter/ios/Flutter/ios_backend_fw_static.framework
.PHONY: flutter/ios/native
flutter/ios/native:
	@# NOTE: add `--copt -g` for debug info (but the resulting library would be 0.5 GiB)
	bazel ${BAZEL_OUTPUT_ROOT_ARG} build --config=ios_fat64 -c opt //flutter/cpp/flutter:ios_backend_fw_static

	rm -rf ${xcode_fw}
	cp -a ${bazel_ios_fw} ${xcode_fw}

.PHONY: flutter/ios/ipa
flutter/ios/ipa: flutter/ios
	cd flutter && flutter clean
	cd flutter && flutter build ipa ${flutter_common_dart_flags}
	mkdir -p output/flutter/ios/
	cp -rf flutter/build/ios/archive/Runner.xcarchive output/flutter/ios/release.xcarchive
