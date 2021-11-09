BAZEL_LINKS_DIR=bazel-
_bazel_links_arg=--symlink_prefix ${BAZEL_LINKS_DIR} --experimental_no_product_name_out_symlink

.PHONY: android
android: backend-bridge-android backends/tflite-android prepare-flutter

.PHONY: backend-bridge-android
backend-bridge-android:
	mkdir -p ../output/home/mlperf/flutter_cache
	bazel ${PROXY_WORKAROUND} --output_user_root=/home/mlperf/flutter_cache/bazel build --config=android_arm64 -c opt //flutter/cpp/flutter:libbackendbridge.so
	chmod +w bazel-bin/cpp/flutter/libbackendbridge.so
	mkdir -p android/app/src/main/jniLibs/arm64-v8a
	rm -f android/app/src/main/jniLibs/arm64-v8a/libbackendbridge.so
	cp ../output/`readlink bazel-bin`/cpp/flutter/libbackendbridge.so android/app/src/main/jniLibs/arm64-v8a/libbackendbridge.so

.PHONY: backends/tflite-android
backends/tflite-android:
	bazel ${PROXY_WORKAROUND} --config=android_arm64 -c opt //flutter/cpp/backend_tflite:libtflitebackend.so
	chmod +w bazel-bin/cpp/backend_tflite/libtflitebackend.so
	mkdir -p android/app/src/main/jniLibs/arm64-v8a
	rm -f android/app/src/main/jniLibs/arm64-v8a/libtflitebackend.so
	cp bazel-bin/cpp/backend_tflite/libtflitebackend.so android/app/src/main/jniLibs/arm64-v8a/libtflitebackend.so

ifeq (${ENABLE_BACKEND_EXAMPLE},1)
backend_replace_example=\'example\',
else
backend_replace_example=
endif

ifeq (${ENABLE_BACKEND_TFLITE},0)
backend_replace_tflite=
else
backend_replace_tflite=\'libtflitebackend\',
endif

.PHONY: set-supported-backends
set-supported-backends:
	cat flutter/lib/backend/backends_list.in | sed \
		-e "s/EXAMPLE_TAG/${backend_replace_example}/" \
		-e "s/TFLITE_TAG/${backend_replace_tflite}/" \
		> flutter/lib/backend/backends_list.gen.dart

script_launch_prefix=./tool/
script_extension=sh

# FIXME generated files should not be in src tree, they should be in output
.PHONY: protobuf
protobuf:
	rm -rf flutter/lib/protos 
	mkdir -p flutter/lib/protos
	docker run \
		${BASE_DOCKER_FLAGS} \
		-e USE_PROXY_WORKAROUND=${USE_PROXY_WORKAROUND} \
		-w /home/mlperf/mobile_app/flutter \
		mlcommons/mlperf_mobile:1.0 /bin/bash -c "dart pub get && \
			protoc --proto_path cpp/proto \
				--plugin protoc-gen-dart='${script_launch_prefix}protoc-gen-dart.${script_extension}' \
				--dart_out lib/protos \
				cpp/proto/*.proto"

.PHONY: format
format: format-clang format-bazel format-dart

.PHONY: format-clang
format-clang:
	${script_launch_prefix}run-clang-format.${script_extension}

.PHONY: format-bazel
format-bazel:
	buildifier -r .

.PHONY: format-dart
format-dart:
	dart run import_sorter:main
	dart format flutter/lib flutter/integration_test flutter/test_driver

.PHONY: lint
lint:
	docker run \
		${BASE_DOCKER_FLAGS} \
		-w /home/mlperf/mobile_app/flutter \
		mlcommons/mlperf_mobile:1.0 flutter analyze

.PHONY: update-splash-screen
update-splash-screen:
	flutter/tool/update-splash-screen

.PHONY: generate-localizations
generate-localizations:
	flutter gen-l10n \
		--arb-dir=flutter/lib/resources \
		--output-dir=flutter/lib/localizations \
		--template-arb-file=app_en.arb \
		--output-localization-file=app_localizations.dart \
		--no-synthetic-package

# this make target is intended to be used by CI system, to avoid calling other targets directly
.PHONY: prepare-lint
prepare-lint: prepare-flutter

.PHONY: prepare-flutter
prepare-flutter: set-supported-backends protobuf generate-localizations
