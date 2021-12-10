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

include ios.mk
include windows.mk
include windows-docker.mk
include android.mk

BAZEL_LINKS_DIR=bazel-
_bazel_links_arg=--symlink_prefix ${BAZEL_LINKS_DIR} --experimental_no_product_name_out_symlink

# To add a new vendor backend, copy and uncomment this block and replace "EXAMPLE" and "example" with the vendor name (e.g. INTEL and intel)
# See tflite backend below as an example
#ifeq (${ENABLE_BACKEND_EXAMPLE},0)
#backend_replace_example=
#else
#backend_replace_example=\'libexamplebackend\',
#endif

ifeq (${ENABLE_BACKEND_TFLITE},0)
backend_replace_tflite=
else
backend_replace_tflite=\'libtflitebackend\',
endif

# To add a new backend, add a new line to the sed command for your backend replacing "example" with the vendor name. For instance, for Intel it would be:
# -e "s/INTEL_TAG/${backend_replace_intel}/"
.PHONY: flutter/set-supported-backends
flutter/set-supported-backends:
	cat flutter/lib/backend/backends_list.in | sed \
		-e "s/EXAMPLE_TAG/${backend_replace_example}/" \
		-e "s/TFLITE_TAG/${backend_replace_tflite}/" \
		> flutter/lib/backend/backends_list.gen.dart

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
	flutter gen-l10n \
		--arb-dir=flutter/lib/resources \
		--output-dir=flutter/lib/localizations \
		--template-arb-file=app_en.arb \
		--output-localization-file=app_localizations.dart \
		--no-synthetic-package

.PHONY: flutter/prepare-flutter
flutter/prepare-flutter: flutter/set-supported-backends flutter/protobuf flutter/generate-localizations
