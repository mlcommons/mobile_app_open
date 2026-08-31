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

# The Metal accelerator ships as a plain dylib, but an iOS app bundle may only
# embed bundles: a bare Mach-O under Frameworks/ is rejected by App Store
# Connect. Wrap it in a framework whose CFBundleExecutable keeps the original
# filename -- LiteRT dlopens the accelerator by joining its runtime library dir
# with that exact name, so the name has to survive the move.
flutter_ios_metal_fw_name=LiteRtMetalAccelerator
flutter_ios_metal_fw_dir=${flutter_ios_fw_dir}/${flutter_ios_metal_fw_name}.framework

.PHONY: flutter/ios/clean
flutter/ios/clean:
	rm -rf flutter/build/ios

# BAZEL_OUTPUT_ROOT_ARG is set on our Jenkins CI
.PHONY: flutter/ios/libs
flutter/ios/libs:
	${backend_litert_ios_lib_deps}
	# --use_top_level_targets_for_symlinks
	bazel ${BAZEL_OUTPUT_ROOT_ARG} build ${BAZEL_CACHE_ARG} \
		--config=ios \
		${backend_bridge_ios_target} \
		${backend_tflite_ios_target} \
		${backend_coreml_ios_target} \
		${backend_litert_ios_target}

	rm -rf ${flutter_ios_fw_dir}
	unzip -q -o -d ${flutter_ios_fw_dir} ${backend_bridge_ios_zip}
	unzip -q -o -d ${flutter_ios_fw_dir} ${backend_tflite_ios_zip}
	unzip -q -o -d ${flutter_ios_fw_dir} ${backend_coreml_ios_zip}
	unzip -q -o -d ${flutter_ios_fw_dir} ${backend_litert_ios_zip}
	@# LiteRT dlopens the Metal accelerator from the app bundle's Frameworks
	@# directory, so it has to be embedded next to the backend frameworks --
	@# as a framework rather than a loose dylib, see flutter_ios_metal_fw_dir.
	rm -rf ${flutter_ios_metal_fw_dir}
	mkdir -p ${flutter_ios_metal_fw_dir}
	cp -f ${backend_litert_ios_file} ${flutter_ios_metal_fw_dir}/
	@# CFBundleExecutable is the dylib's own filename, which is what keeps
	@# kLiteRtEnvOptionTagRuntimeLibraryDir + "libLiteRtMetalAccelerator.dylib"
	@# resolving. MinimumOSVersion has to match the frameworks the app embeds
	@# (LITERT_MIN_IOS_VERSION), or the bundle is rejected as inconsistent.
	printf '%s\n' \
		'<?xml version="1.0" encoding="UTF-8"?>' \
		'<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">' \
		'<plist version="1.0">' \
		'<dict>' \
		'<key>CFBundleDevelopmentRegion</key><string>en</string>' \
		'<key>CFBundleExecutable</key><string>${backend_litert_ios_bin_filename}</string>' \
		'<key>CFBundleIdentifier</key><string>org.mlcommons.litert.metalaccelerator</string>' \
		'<key>CFBundleInfoDictionaryVersion</key><string>6.0</string>' \
		'<key>CFBundleName</key><string>${flutter_ios_metal_fw_name}</string>' \
		'<key>CFBundlePackageType</key><string>FMWK</string>' \
		'<key>CFBundleShortVersionString</key><string>2.1.5</string>' \
		'<key>CFBundleVersion</key><string>2.1.5</string>' \
		'<key>CFBundleSupportedPlatforms</key><array><string>iPhoneOS</string></array>' \
		'<key>MinimumOSVersion</key><string>14.0</string>' \
		'</dict>' \
		'</plist>' > ${flutter_ios_metal_fw_dir}/Info.plist

flutter/ios/release: flutter/check-release-env flutter/ios flutter/prepare flutter/ios/ipa

.PHONY: flutter/ios/ipa
flutter/ios/ipa:
	@[ -n "${FLUTTER_BUILD_NUMBER}" ] || (echo FLUTTER_BUILD_NUMBER env must be set; exit 1)
	# This project integrates iOS plugins via CocoaPods. Swift Package Manager is
	# on by default in Flutter 3.44+, and its build-for-testing path fails with
	# "no such module 'Flutter'"; disable it to keep the CocoaPods integration.
	flutter --no-version-check config --no-enable-swift-package-manager
	cd flutter && flutter --no-version-check clean
	cd flutter && flutter --no-version-check build \
		ipa \
		${flutter_official_build_arg} \
		${flutter_build_number_arg}
	mkdir -p output/flutter/ios/
	cp -rf flutter/build/ios/archive/Runner.xcarchive output/flutter/ios/release.xcarchive

# BrowserStack test package targets
flutter/ios/test-package: flutter/ios/test-package/build flutter/ios/test-package/zip

.PHONY: flutter/ios/test-package/build
flutter/ios/test-package/build:
	# See flutter/ios/ipa: SPM (default-on in Flutter 3.44+) breaks the
	# build-for-testing path; keep the CocoaPods integration.
	flutter --no-version-check config --no-enable-swift-package-manager
	cd flutter && flutter --no-version-check build ios \
		--config-only \
		${flutter_perf_test_arg} \
		integration_test/first_test.dart
	cd flutter/ios && xcodebuild \
		-workspace Runner.xcworkspace \
		-scheme Runner \
		-config Flutter/Release.xcconfig \
		-derivedDataPath ../build/ios_integration \
		-sdk iphoneos \
		-allowProvisioningUpdates \
		CODE_SIGN_IDENTITY="$${CODE_SIGN_IDENTITY:-Apple Development}" \
		$$(if [ -n "$${DEVELOPMENT_TEAM}" ]; then echo "DEVELOPMENT_TEAM=$${DEVELOPMENT_TEAM}"; fi) \
		$$(if [ -n "$${APP_STORE_CONNECT_API_KEY_PATH}" ] && [ -s "$${APP_STORE_CONNECT_API_KEY_PATH}" ] && [ -n "$${APP_STORE_CONNECT_API_KEY_ID}" ] && [ -n "$${APP_STORE_CONNECT_API_KEY_ISSUER_ID}" ]; then echo "-authenticationKeyPath $${APP_STORE_CONNECT_API_KEY_PATH} -authenticationKeyID $${APP_STORE_CONNECT_API_KEY_ID} -authenticationKeyIssuerID $${APP_STORE_CONNECT_API_KEY_ISSUER_ID}"; fi) \
		build-for-testing

FLUTTER_IOS_TEST_PACKAGE?=ios_tests-${FLUTTER_BUILD_NUMBER}.zip
.PHONY: flutter/ios/test-package/zip
flutter/ios/test-package/zip:
	mkdir -p output/ios-test-package
	cd flutter/build/ios_integration/Build/Products && \
		zip -r $(CURDIR)/output/ios-test-package/${FLUTTER_IOS_TEST_PACKAGE} \
		Release-iphoneos/ \
		*.xctestrun
