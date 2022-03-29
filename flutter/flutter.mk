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

flutter_common_dart_flags= \
        --dart-define=official-build=${OFFICIAL_BUILD}

.PHONY: flutter
ifeq (${OS},Windows_NT)
flutter: flutter/windows
else
ifeq ($(shell uname -s),Darwin)
flutter: flutter/ios
else
flutter: flutter/android
endif
endif

include flutter/ios.mk
include flutter/windows.mk
include flutter/windows-docker.mk
include flutter/android.mk

.PHONY: flutter/set-supported-backends
flutter/set-supported-backends:
	cat flutter/lib/backend/list.in | sed \
		-e "s/TFLITE_TAG/${backend_tflite_filename}/" \
		-e "s/MEDIATEKE_TAG/${backend_mediatek_filename}/" \
		-e "s/PIXEL_TAG/${backend_pixel_filename}/" \
		-e "s/QTI_TAG/${backend_qti_filename}/" \
		-e "s/SAMSUNG_TAG/${backend_samsung_filename}/" \
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
.PHONY: flutter/generate-firebase-config
flutter/generate-firebase-config:
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
		> flutter_common/lib/firebase/config.gen.dart
	dart format flutter_common/lib/firebase/config.gen.dart
	. ${FIREBASE_CONFIG_ENV_PATH} && \
		cat firebase_functions/functions/src/prefix.in | sed \
		-e "s,FIREBASE_FLUTTER_FUNCTIONS_PREFIX,$$FIREBASE_FLUTTER_FUNCTIONS_PREFIX," \
		> firebase_functions/functions/src/prefix.gen.ts

.PHONY: flutter/generate-result-schema
flutter/generate-result-schema:
	cd flutter_common && ${_start_args} dart run --define=jsonFileName=../output/extended-result-example.json lib/data/json_generator_main.dart
	quicktype output/extended-result-example.json --out flutter/documentation/extended-result.schema.json --lang schema
	quicktype --src-lang schema flutter/documentation/extended-result.schema.json --lang ts --top-level ExtendedResult --out firebase_functions/functions/src/extended-result.gen.ts

.PHONY: flutter/protobuf
flutter/protobuf:
	cd flutter && ${_start_args} dart pub get
	rm -rf flutter/lib/protos
	mkdir -p flutter/lib/protos
	protoc --proto_path flutter/cpp/proto \
		--dart_out flutter/lib/protos \
		flutter/cpp/proto/*.proto
	dart format flutter/lib/protos

.PHONY: flutter/update-splash-screen
flutter/update-splash-screen:
	cd flutter && tool/update-splash-screen

.PHONY: flutter/generate-localizations
flutter/generate-localizations:
	flutter --no-version-check gen-l10n \
		--arb-dir=flutter/lib/l10n \
		--output-dir=flutter/lib/localizations \
		--template-arb-file=app_en.arb \
		--output-localization-file=app_localizations.dart \
		--no-synthetic-package
	dart format flutter/lib/localizations

.PHONY: flutter/prepare
flutter/prepare: flutter/set-supported-backends flutter/protobuf flutter/generate-localizations flutter/generate-firebase-config

.PHONY: flutter/clean
flutter/clean:
	cd flutter && ${_start_args} flutter --no-version-check clean
