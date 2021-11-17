# Copyright 2021 The MLPerf Authors. All Rights Reserved.
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

export PATH := $(CURDIR)/output/flutter/bin:$(PATH)
export PUB_CACHE=$(CURDIR)/output/pub_cache

.PHONY: flutter flutter_backendbridge

flutter: output/flutter/.stamp
output/flutter/.stamp:
	cd output && git clone https://github.com/flutter/flutter.git
	touch $@

flutter_backendbridge: flutter output/mlperf_mobile_docker_1_0.stamp
	echo "Building flutter app for android"
	mkdir -p output/home/mlperf/flutter_cache && chmod 777 output/home/mlperf/flutter_cache
	docker run \
		${BASE_DOCKER_FLAGS} \
		-e USE_PROXY_WORKAROUND=${USE_PROXY_WORKAROUND} \
		-w /home/mlperf/mobile_app \
		mlcommons/mlperf_mobile:1.0 \
		bazel ${PROXY_WORKAROUND2} --output_user_root=/home/mlperf/flutter_cache/bazel build --config=android_arm64 -c opt //flutter/cpp/flutter:libbackendbridge.so
		mkdir -p flutter/android/app/src/main/jniLibs/arm64-v8a/
		rm -f flutter/android/app/src/main/jniLibs/arm64-v8a/libbackendbridge.so
		cp output/`readlink bazel-bin`/flutter/cpp/flutter/libbackendbridge.so flutter/android/app/src/main/jniLibs/arm64-v8a/libbackendbridge.so

flutter_tflite:
	echo "Building flutter tflite backend for android"
	mkdir -p output/home/mlperf/flutter_cache && chmod 777 output/home/mlperf/flutter_cache
	docker run \
		${BASE_DOCKER_FLAGS} \
		-e USE_PROXY_WORKAROUND=${USE_PROXY_WORKAROUND} \
		-w /home/mlperf/mobile_app \
		mlcommons/mlperf_mobile:1.0 \
		bazel ${PROXY_WORKAROUND2} --output_user_root=/home/mlperf/flutter_cache/bazel build --config=android_arm64 -c opt //mobile_back_tflite/cpp/backend_tflite:libtflitebackend.so
		chmod +w output/`readlink bazel-bin`/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so
		mkdir -p flutter/android/app/src/main/jniLibs/arm64-v8a
		rm -f flutter/android/app/src/main/jniLibs/arm64-v8a/libtflitebackend.so
		cp output/`readlink bazel-bin`/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so flutter/android/app/src/main/jniLibs/arm64-v8a/libtflitebackend.so

flutter_assets: flutter_backendbridge flutter_tflite output/protoc3/bin/protoc
	cat flutter/lib/backend/backends_list.in | sed \
		-e "s/EXAMPLE_TAG/${backend_replace_example}/" \
		-e "s/TFLITE_TAG/${backend_replace_tflite}/" \
		> flutter/lib/backend/backends_list.gen.dart
	rm -rf flutter/lib/protos 
	mkdir -p flutter/lib/protos
	cd flutter &&  dart pub get
	cd flutter && ../output/protoc3/bin/protoc --proto_path ../android/cpp/proto \
				--plugin protoc-gen-dart='${script_launch_prefix}protoc-gen-dart.${script_extension}' \
				--dart_out lib/protos \
				../android/cpp/proto/*.proto
	flutter gen-l10n \
		--arb-dir=flutter/lib/resources \
		--output-dir=flutter/lib/localizations \
		--template-arb-file=app_en.arb \
		--output-localization-file=app_localizations.dart \
		--no-synthetic-package
	
flutter_analyze: flutter_assets
	cd flutter && ../output/flutter/bin/flutter analyze

flutter_app: flutter_assets
	mkdir -p output/gradle
	cd flutter && GRADLE_USER_HOME=../output/gradle ../output/flutter/bin/flutter build apk

output/protoc3/bin/protoc: output/protoc-3.6.1-linux-x86_64.zip
	rm -rf output/protoc3
	cd output && unzip protoc-3.6.1-linux-x86_64.zip -d protoc3
	touch $@

output/protoc-3.6.1-linux-x86_64.zip:
	cd output && curl -OL https://github.com/google/protobuf/releases/download/v3.6.1/protoc-3.6.1-linux-x86_64.zip

