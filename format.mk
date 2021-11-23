
ifeq (${OS},Windows_NT)
# On Windows some commands don't run correctly in the default make shell
_start_args=powershell -Command
else
_start_args=
endif

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

.PHONY: format/dart
format/dart:
	cd flutter && ${_start_args} dart run import_sorter:main
	cd flutter && ${_start_args} dart format lib integration_test test_driver

.PHONY: format/line-endings
format/line-endings:
	git ls-files -z | xargs -0 ${_start_args} dos2unix --keep-bom --

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

.PHONY: lint/check-line-endings
lint/check-line-endings:
	incorrect_files=$$(git ls-files -z | xargs -0 ${_start_args} dos2unix --info=c --) && \
	if [ ! -z "$$incorrect_files" ]; then \
		echo -e "found files with CRLF line endings: \n$$incorrect_files"; \
		false; \
	fi
