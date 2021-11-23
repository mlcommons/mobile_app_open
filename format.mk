
.PHONY: format
ifeq (${OS},Windows_NT)
# format/java is not supported on Windows
format: format/bazel format/clang format/dart
else
format: format/bazel format/clang format/java format/dart
endif

.PHONY: format/bazel
format/bazel:
	buildifier -r .

.PHONY: format/clang
format/clang:
	_files=$$(git ls-files | grep "\.h$$\|\.cc$$\|\.cpp$$") \
		&& [ ! -z "$$_files" ] \
		&& clang-format -i -style=google $${_files}

.PHONY: format/java
format/java:
	_files=$$(git ls-files | grep "\.java$$") \
		&& [ ! -z "$$_files" ] \
		&& java -jar /opt/formatters/google-java-format-1.9-all-deps.jar --replace $${_files}

ifeq (${OS},Windows_NT)
# On Windows dart and flutter commands are defined as .bat files.
# When these commands are used in makefile directly, they work as expected.
# When these commands are used after a `cd` command, make apparently uses a different shell,
# and `dart` command fails with "some/path/flutter/bin/dart: No such file or directory".
# Explicitly using cmd prevents this.
_dart_start_args=cmd //C
else
_dart_start_args=
endif

.PHONY: format/dart
format/dart:
	cd flutter && ${_dart_start_args} dart run import_sorter:main
	cd flutter && ${_dart_start_args} dart format lib integration_test test_driver

.PHONY: lint
lint: lint/bazel lint/dart lint/check-prohibited lint/check-big-files

.PHONY: lint/bazel
lint/bazel:
	buildifier -lint=warn -r .

.PHONY: lint/dart
lint/dart:
	cd flutter && make prepare-flutter
	dart analyze flutter

.PHONY: lint/check-prohibited
lint/check-prohibited:
	_files=$$(git ls-files | grep "\.so$$\|\.apk$$\|\.tflite$$\|\.dlc$$\|\.zip$$\|\.jar$$\|\.tgz$$"); \
		if [ ! -z "$$_files" ]; then \
			echo found prohibited files:; \
			echo "$$_files"; \
			false; \
		fi

.PHONY: lint/check-big-files
lint/check-big-files:
	big_file_list=""; \
		for f in $$(git ls-files); do \
			if [ "$$(stat $$f --format %s)" -gt 5242880 ]; then \
				big_file_list="$$big_file_list$$f\n"; \
			fi; \
		done; \
		if [ ! -z "$$big_file_list" ]; then \
			echo -e "found too big files: \n$$big_file_list"; \
			false; \
		fi
