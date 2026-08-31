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
	@# directory, so it has to be embedded next to the backend frameworks
	cp -f ${backend_litert_ios_file} ${flutter_ios_fw_dir}

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

# App Store Connect validation targets
#
# Xcode Cloud's "Preparing build for App Store Connect" step fails on bundles
# that xcodebuild itself accepts: `archive` and `-exportArchive` both succeed,
# and the ITMS checks that reject the build run on Apple's servers afterwards.
# Reproducing those checks therefore takes a real submission to the validation
# service, which `altool --validate-app` performs without publishing a build.
ios_appstore_archive=$(CURDIR)/output/flutter/ios/appstore.xcarchive
ios_appstore_export=$(CURDIR)/output/flutter/ios/appstore-export
ios_appstore_export_options=$(CURDIR)/output/flutter/ios/ExportOptions-appstore.plist

# xcodebuild talks to App Store Connect for automatic signing. Only pass the
# key when it is fully configured, the same way flutter/ios/test-package/build
# does, so a run without those secrets fails on signing rather than on flags.
ios_xcodebuild_auth_args=$$(if [ -n "$${APP_STORE_CONNECT_API_KEY_PATH}" ] && [ -s "$${APP_STORE_CONNECT_API_KEY_PATH}" ] && [ -n "$${APP_STORE_CONNECT_API_KEY_ID}" ] && [ -n "$${APP_STORE_CONNECT_API_KEY_ISSUER_ID}" ]; then echo "-authenticationKeyPath $${APP_STORE_CONNECT_API_KEY_PATH} -authenticationKeyID $${APP_STORE_CONNECT_API_KEY_ID} -authenticationKeyIssuerID $${APP_STORE_CONNECT_API_KEY_ISSUER_ID}"; fi)

flutter/ios/appstore-validate: flutter/ios/appstore/archive flutter/ios/appstore/export flutter/ios/appstore/validate

.PHONY: flutter/ios/appstore/archive
flutter/ios/appstore/archive:
	@[ -n "${FLUTTER_BUILD_NUMBER}" ] || (echo FLUTTER_BUILD_NUMBER env must be set; exit 1)
	@[ -n "$${DEVELOPMENT_TEAM}" ] || (echo DEVELOPMENT_TEAM env must be set; exit 1)
	# See flutter/ios/ipa: SPM (default-on in Flutter 3.44+) breaks this path;
	# keep the CocoaPods integration.
	flutter --no-version-check config --no-enable-swift-package-manager
	cd flutter && flutter --no-version-check build ios \
		--config-only \
		${flutter_official_build_arg} \
		${flutter_build_number_arg}
	mkdir -p $(CURDIR)/output/flutter/ios
	rm -rf ${ios_appstore_archive}
	cd flutter/ios && xcodebuild \
		-workspace Runner.xcworkspace \
		-scheme Runner \
		-configuration Release \
		-sdk iphoneos \
		-destination generic/platform=iOS \
		-archivePath ${ios_appstore_archive} \
		-allowProvisioningUpdates \
		CODE_SIGN_STYLE=Automatic \
		DEVELOPMENT_TEAM=$${DEVELOPMENT_TEAM} \
		${ios_xcodebuild_auth_args} \
		archive

.PHONY: flutter/ios/appstore/export
flutter/ios/appstore/export:
	@[ -n "$${DEVELOPMENT_TEAM}" ] || (echo DEVELOPMENT_TEAM env must be set; exit 1)
	@[ -d "${ios_appstore_archive}" ] \
		|| (echo "No archive at ${ios_appstore_archive}; run flutter/ios/appstore/archive first"; exit 1)
	mkdir -p $(CURDIR)/output/flutter/ios
	@# Xcode 16 renamed the distribution methods: "app-store" became
	@# "app-store-connect" and "ad-hoc" became "release-testing". The old names
	@# still work but log a deprecation, so use the current one.
	printf '%s\n' \
		'<?xml version="1.0" encoding="UTF-8"?>' \
		'<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">' \
		'<plist version="1.0">' \
		'<dict>' \
		'<key>method</key><string>app-store-connect</string>' \
		'<key>destination</key><string>export</string>' \
		'<key>signingStyle</key><string>automatic</string>' \
		"<key>teamID</key><string>$${DEVELOPMENT_TEAM}</string>" \
		'<key>uploadSymbols</key><true/>' \
		'</dict>' \
		'</plist>' > ${ios_appstore_export_options}
	rm -rf ${ios_appstore_export}
	xcodebuild -exportArchive \
		-archivePath ${ios_appstore_archive} \
		-exportOptionsPlist ${ios_appstore_export_options} \
		-exportPath ${ios_appstore_export} \
		-allowProvisioningUpdates \
		${ios_xcodebuild_auth_args}

.PHONY: flutter/ios/appstore/validate
flutter/ios/appstore/validate:
	@[ -n "$${APP_STORE_CONNECT_API_KEY_ID}" ] \
		|| (echo APP_STORE_CONNECT_API_KEY_ID env must be set; exit 1)
	@[ -n "$${APP_STORE_CONNECT_API_KEY_ISSUER_ID}" ] \
		|| (echo APP_STORE_CONNECT_API_KEY_ISSUER_ID env must be set; exit 1)
	@xcrun --find altool >/dev/null 2>&1 \
		|| (echo "altool is not in the selected Xcode; App Store validation needs it (or Transporter)"; exit 1)
	@# altool looks the key up by ID rather than path, in ./private_keys,
	@# ~/private_keys, ~/.private_keys or ~/.appstoreconnect/private_keys. The
	@# caller is responsible for putting AuthKey_<id>.p8 in one of those.
	@ipa=$$(ls ${ios_appstore_export}/*.ipa 2>/dev/null | head -1); \
	if [ -z "$$ipa" ]; then \
		echo "No .ipa in ${ios_appstore_export}; run flutter/ios/appstore/export first"; \
		exit 1; \
	fi; \
	echo "Listing embedded binaries (a bare file here is an ITMS-90171 risk):"; \
	unzip -Z1 "$$ipa" 'Payload/*.app/Frameworks/*' | sed 's,Payload/[^/]*\.app/Frameworks/,  ,' | sort -u; \
	echo "Validating $$ipa against App Store Connect"; \
	xcrun altool --validate-app -f "$$ipa" -t ios \
		--apiKey "$${APP_STORE_CONNECT_API_KEY_ID}" \
		--apiIssuer "$${APP_STORE_CONNECT_API_KEY_ISSUER_ID}"
