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
include flutter/cpp/binary/cmdline.mk

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
flutter/result: flutter/result/json flutter/result/gen-schema
flutter/prepare: flutter/pub flutter/result/json flutter/backend-list flutter/protobuf flutter/l10n flutter/build-info flutter/firebase-config flutter/set-windows-build-number
flutter/check-release-env: flutter/check/official-build flutter/check/build-number
flutter/test: flutter/test/unit flutter/test/integration

OFFICIAL_BUILD?=false
flutter_official_build_arg=--dart-define=official-build=${OFFICIAL_BUILD}
.PHONY: flutter/check/official-build
flutter/check/official-build:
	@[ "$$OFFICIAL_BUILD" = "true" ] || [ "$$OFFICIAL_BUILD" = "false" ] \
		|| (echo OFFICIAL_BUILD env must be explicitly set to \"true\" or \"false\"; exit 1)

FLUTTER_APP_VERSION?=$(shell grep 'version:' flutter/pubspec.yaml | head -n1 | awk '{ print $$2}' | awk -F+ '{ print $$1}')
FLUTTER_BUILD_NUMBER?=0
flutter_build_number_arg=--build-number ${FLUTTER_BUILD_NUMBER}
.PHONY: flutter/check/build-number
flutter/check/build-number:
	@echo FLUTTER_APP_VERSION=${FLUTTER_APP_VERSION}
	@echo FLUTTER_BUILD_NUMBER=${FLUTTER_BUILD_NUMBER}
	@[ -n "$$FLUTTER_BUILD_NUMBER" ] \
		|| (echo FLUTTER_BUILD_NUMBER env must be explicitly set; exit 1)

ifneq (${FLUTTER_DATA_FOLDER},)
flutter_data_folder_arg="--dart-define=default-data-folder=${FLUTTER_DATA_FOLDER}"
else
flutter_data_folder_arg=
endif
ifneq (${FLUTTER_CACHE_FOLDER},)
flutter_cache_folder_arg="--dart-define=default-cache-folder=${FLUTTER_CACHE_FOLDER}"
else
flutter_cache_folder_arg=
endif
flutter_folder_args=${flutter_data_folder_arg} ${flutter_cache_folder_arg}

FIREBASE_ENV_FILE?=flutter/lib/firebase/firebase_options.env
-include ${FIREBASE_ENV_FILE}
.PHONY: flutter/firebase-config
flutter/firebase-config:
    export
	bash flutter/tool/generate-firebase-config-files.sh

.PHONY: flutter/check/firebase-env
flutter/check/firebase-env:
	@echo FIREBASE_PROJECT_ID=${FIREBASE_PROJECT_ID}
	@[ -n "${FIREBASE_PROJECT_ID}" ] || (echo FIREBASE_PROJECT_ID env must be set; exit 1)

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

RESULT_JSON_SAMPLE_PATH?=output/extended_result_example.json
.PHONY: flutter/result/gen-extended-sample
flutter/result/gen-extended-sample:
	cd flutter_common && \
		${_start_args} dart run \
		--define=jsonFileName=../${RESULT_JSON_SAMPLE_PATH} \
		lib/data/generation_helpers/write_json_sample.main.dart

default_result_json_schema_path=tools/extended_result_schema.json
RESULT_JSON_SCHEMA_PATH?=${default_result_json_schema_path}
.PHONY: flutter/result/gen-schema
flutter/result/gen-schema: flutter/result/gen-extended-sample
	quicktype ${RESULT_JSON_SAMPLE_PATH} \
		--lang schema \
		--out ${RESULT_JSON_SCHEMA_PATH}
	cd flutter_common && \
		${_start_args} dart run \
		--define=schemaPath=../${RESULT_JSON_SCHEMA_PATH} \
		lib/data/generation_helpers/edit_json_schema.main.dart

.PHONY: flutter/result/json
flutter/result/json:
	cd flutter_common && ${_start_args} flutter --no-version-check pub run \
		build_runner build --delete-conflicting-outputs

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
	cd flutter && FLUTTER_APP_VERSION='$(FLUTTER_APP_VERSION)' tool/update-splash-screen.sh

.PHONY: flutter/l10n
flutter/l10n:
	flutter --no-version-check gen-l10n \
		--arb-dir=flutter/lib/l10n \
		--output-dir=flutter/lib/localizations \
		--template-arb-file=app_en.arb \
		--output-localization-file=app_localizations.dart \
		--no-synthetic-package
	dart format flutter/lib/localizations

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
		output/flutter/pub/flutter_common.stamp
	[ -z "${FLUTTER_FORCE_PUB_GET}" ] || rm -rf output/flutter/pub
output/flutter/pub/%.stamp: %/pubspec.yaml
	cd $(shell basename $@ .stamp) && ${_start_args} flutter --no-version-check pub get
	mkdir -p $(shell dirname $@)
	touch $@

.PHONY: flutter/test/unit
flutter/test/unit:
	cd flutter && ${_start_args} flutter --no-version-check test --no-pub test -r expanded
	cd flutter_common && ${_start_args} flutter --no-version-check test --no-pub test -r expanded

ifneq (${FLUTTER_TEST_DEVICE},)
flutter_test_device_arg=--device-id "${FLUTTER_TEST_DEVICE}"
else
flutter_test_device_arg=
endif
flutter_perf_test_arg=--dart-define=enable-perf-test=${PERF_TEST}
.PHONY: flutter/test/integration
flutter/test/integration:
	cd flutter && ${_start_args} \
		flutter --no-version-check test --no-pub -r expanded \
		integration_test/first_test.dart \
		${flutter_test_device_arg} \
		${flutter_official_build_arg} \
		${flutter_perf_test_arg} \
		${flutter_folder_args}

.PHONY: flutter/run
flutter/run:
	cd flutter && ${_start_args} \
		flutter --no-version-check \
		run \
		${flutter_folder_args} \
		${flutter_test_device_arg} \
		${flutter_official_build_arg}

.PHONY: flutter/clean
flutter/clean:
	cd flutter && ${_start_args} flutter --no-version-check clean
	cd flutter_common && ${_start_args} flutter --no-version-check clean
	rm -rf output/flutter/pub

