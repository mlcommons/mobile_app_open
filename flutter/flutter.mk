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
        --dart-define=official_build=${OFFICIAL_BUILD}

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

.PHONY: flutter/protobuf
flutter/protobuf:
	cd flutter && ${_start_args} dart pub get
	rm -rf flutter/lib/protos
	mkdir -p flutter/lib/protos
	protoc --proto_path android/cpp/proto \
		--dart_out flutter/lib/protos \
		android/cpp/proto/*.proto

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

.PHONY: flutter/prepare-flutter
flutter/prepare-flutter: flutter/set-supported-backends flutter/protobuf flutter/generate-localizations

.PHONY: flutter/clean
flutter/clean:
	cd flutter && ${_start_args} flutter --no-version-check clean
