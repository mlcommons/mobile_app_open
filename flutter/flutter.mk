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

include flutter/ios/ios.mk
include flutter/windows/windows.mk
include flutter/android/android.mk

ifeq (${OS},Windows_NT)
flutter/platform: flutter/windows
else
ifeq ($(shell uname -s),Darwin)
flutter/platform: flutter/ios
else
flutter/platform: flutter/android
endif
endif

flutter: flutter/prepare flutter/platform
flutter/firebase: flutter/firebase/config flutter/firebase/prefix
flutter/result: flutter/result/schema flutter/result/ts
flutter/prepare: flutter/pub flutter/backend-list flutter/protobuf flutter/l10n flutter/firebase flutter/build-info flutter/set-windows-build-number
flutter/check-release-env: flutter/check/official-build flutter/check/build-number
flutter/test: flutter/test/unit flutter/test/integration

OFFICIAL_BUILD?=false
flutter_official_build_arg=--dart-define=official-build=${OFFICIAL_BUILD}
.PHONY: flutter/check/official-build
flutter/check/official-build:
	@[ "$$OFFICIAL_BUILD" = "true" ] || [ "$$OFFICIAL_BUILD" = "false" ] \
		|| (echo OFFICIAL_BUILD env must be explicitly set to \"true\" or \"false\"; exit 1)

FLUTTER_BUILD_NUMBER?=0
flutter_build_number_arg=--build-number ${FLUTTER_BUILD_NUMBER}
.PHONY: flutter/check/build-number
flutter/check/build-number:
	@[ -n "$$FLUTTER_BUILD_NUMBER" ] \
		|| (echo FLUTTER_BUILD_NUMBER env must be explicitly set; exit 1)

.PHONY: flutter/backend-list
flutter/backend-list:
	cat flutter/lib/backend/list.in | sed \
		-e "s/TFLITE_TAG/${backend_tflite_filename}/" \
		-e "s/MEDIATEK_TAG/${backend_mediatek_filename}/" \
		-e "s/PIXEL_TAG/${backend_pixel_filename}/" \
		-e "s/QTI_TAG/${backend_qti_filename}/" \
		-e "s/SAMSUNG_TAG/${backend_samsung_filename}/" \
		-e "s/APPLE_TAG/${backend_coreml_filename}/" \
		> flutter/lib/backend/list.gen.dart
	dart format flutter/lib/backend/list.gen.dart

ifeq (${FIREBASE_CONFIG_ENV_PATH},)
FIREBASE_CONFIG_ENV_PATH=output/flutter/empty.sh
FIREBASE_FLUTTER_ENABLE=false
$(shell mkdir -p output/flutter)
$(shell touch ${FIREBASE_CONFIG_ENV_PATH})
$(shell chmod +x ${FIREBASE_CONFIG_ENV_PATH})
else
FIREBASE_FLUTTER_ENABLE=true
endif

.PHONY: flutter/firebase/config
flutter/firebase/config:
	@echo using firebase config: FIREBASE_CONFIG_ENV_PATH=${FIREBASE_CONFIG_ENV_PATH}
	. ${FIREBASE_CONFIG_ENV_PATH} && \
		cat flutter_common/lib/firebase/config.in | sed \
		-e "s,FIREBASE_FLUTTER_ENABLE,${FIREBASE_FLUTTER_ENABLE}," \
		-e "s,FIREBASE_FLUTTER_CONFIG_API_KEY,$$FIREBASE_FLUTTER_CONFIG_API_KEY," \
		-e "s,FIREBASE_FLUTTER_CONFIG_PROJECT_ID,$$FIREBASE_FLUTTER_CONFIG_PROJECT_ID," \
		-e "s,FIREBASE_FLUTTER_CONFIG_MESSAGING_SENDER_ID,$$FIREBASE_FLUTTER_CONFIG_MESSAGING_SENDER_ID," \
		-e "s,FIREBASE_FLUTTER_CONFIG_APP_ID,$$FIREBASE_FLUTTER_CONFIG_APP_ID," \
		-e "s,FIREBASE_FLUTTER_CONFIG_MEASUREMENT_ID,$$FIREBASE_FLUTTER_CONFIG_MEASUREMENT_ID," \
		-e "s,FIREBASE_FLUTTER_FUNCTIONS_URL,$$FIREBASE_FLUTTER_FUNCTIONS_URL," \
		-e "s,FIREBASE_FLUTTER_FUNCTIONS_PREFIX,$$FIREBASE_FLUTTER_FUNCTIONS_PREFIX," \
		| tee flutter_common/lib/firebase/config.gen.dart
	dart format flutter_common/lib/firebase/config.gen.dart

.PHONY: flutter/firebase/prefix
flutter/firebase/prefix:
	. ${FIREBASE_CONFIG_ENV_PATH} && \
		cat firebase_functions/functions/src/prefix.in | sed \
		-e "s,FIREBASE_FLUTTER_FUNCTIONS_PREFIX,$$FIREBASE_FLUTTER_FUNCTIONS_PREFIX," \
		| tee firebase_functions/functions/src/prefix.gen.ts

result_json_example_path=output/extended-result-example.json
default_result_json_schema_path=flutter/documentation/extended-result.schema.json
RESULT_JSON_SCHEMA_PATH?=${default_result_json_schema_path}
.PHONY: flutter/result/schema
flutter/result/schema:
	cd flutter_common && \
		${_start_args} dart run \
		--define=jsonFileName=../${result_json_example_path} \
		lib/data/generation_helpers/write_json_example.main.dart
	quicktype ${result_json_example_path} \
		--lang schema \
		--out ${RESULT_JSON_SCHEMA_PATH}
	cd flutter_common && \
		${_start_args} dart run \
		--define=schemaPath=../${RESULT_JSON_SCHEMA_PATH} \
		lib/data/generation_helpers/edit_json_schema.main.dart

.PHONY: flutter/result/ts
flutter/result/ts:
	quicktype ${RESULT_JSON_SCHEMA_PATH} \
		--src-lang schema \
		--lang ts \
		--top-level ExtendedResult \
		--out firebase_functions/functions/src/extended-result.gen.ts

.PHONY: flutter/build-info
flutter/build-info:
	cat flutter/lib/build_info.in | sed \
		-e "s,FLUTTER_BUILD_GIT_COMMIT,$(shell git rev-parse HEAD)," \
		-e "s,FLUTTER_BUILD_GIT_BRANCH,$(shell git rev-parse --abbrev-ref HEAD)," \
		-e "s,FLUTTER_BUILD_GIT_DIRTY,$(shell git status --porcelain | head -c1 | wc -c)," \
		| tee flutter/lib/build_info.gen.dart
	dart format flutter/lib/build_info.gen.dart

.PHONY: flutter/protobuf
flutter/protobuf:
	rm -rf flutter/lib/protos
	mkdir -p flutter/lib/protos
	protoc --proto_path flutter/cpp/proto \
		--dart_out flutter/lib/protos \
		flutter/cpp/proto/*.proto
	dart format flutter/lib/protos

.PHONY: flutter/update-splash-screen
flutter/update-splash-screen:
	cd flutter && tool/update-splash-screen

.PHONY: flutter/l10n
flutter/l10n:
	flutter --no-version-check gen-l10n \
		--arb-dir=flutter/lib/l10n \
		--output-dir=flutter/lib/localizations \
		--template-arb-file=app_en.arb \
		--output-localization-file=app_localizations.dart \
		--no-synthetic-package
	dart format flutter/lib/localizations

FLUTTER_APP_VERSION?=$(shell grep 'version:' flutter/pubspec.yaml | head -n1 | awk '{ print $$2}')
.PHONY: flutter/set-windows-build-number
flutter/set-windows-build-number:
	cat flutter/windows/runner/version.in | sed \
		-e "s,APP_VERSION_VALUE,${FLUTTER_APP_VERSION}," \
		-e "s,APP_BUILD_NUMBER_VALUE,$${FLUTTER_APP_BUILD_NUMBER:=0}," \
		| tee flutter/windows/runner/version.gen.h

.PHONY: flutter/pub
flutter/pub:
	[ -z "${FLUTTER_FORCE_PUB_GET}" ] || rm -rf output/flutter/pub
	make \
		output/flutter/pub/flutter.stamp \
		output/flutter/pub/flutter_common.stamp \
		output/flutter/pub/website.stamp
	[ -z "${FLUTTER_FORCE_PUB_GET}" ] || rm -rf output/flutter/pub
output/flutter/pub/%.stamp: %/pubspec.yaml
	cd $(shell basename $@ .stamp) && ${_start_args} flutter --no-version-check pub get
	mkdir -p $(shell dirname $@)
	touch $@

.PHONY: flutter/test/unit
flutter/test/unit:
	cd flutter && ${_start_args} flutter --no-version-check test test

ifneq (${FLUTTER_TEST_DEVICE},)
flutter_test_device_arg=--device-id "${FLUTTER_TEST_DEVICE}"
else
flutter_test_device_arg=
endif
flutter_perf_test_arg=--dart-define=enable-perf-test=${PERF_TEST}
.PHONY: flutter/test/integration
flutter/test/integration:
	cd flutter && ${_start_args} \
		flutter --no-version-check test \
		integration_test \
		${flutter_test_device_arg} \
		${flutter_official_build_arg} \
		${flutter_perf_test_arg}

.PHONY: flutter/run
flutter/run:
	cd flutter && ${_start_args} flutter --no-version-check run ${flutter_test_device_arg} ${flutter_official_build_arg}

.PHONY: flutter/clean
flutter/clean:
	cd flutter && ${_start_args} flutter --no-version-check clean
	rm -rf output/flutter/pub

