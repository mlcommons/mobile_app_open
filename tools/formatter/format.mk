
ifeq (${OS},Windows_NT)
# On Windows some commands don't run correctly in the default make shell
_start_args=powershell -Command
else
_start_args=
endif

.PHONY: format
ifeq (${OS},Windows_NT)
# format/java is not supported on Windows
format: format/bazel format/clang format/dart format/line-endings
else
format: format/bazel format/clang format/java format/dart format/line-endings
endif

.PHONY: format/bazel
format/bazel:
	buildifier -r .

.PHONY: format/clang
format/clang:
	git ls-files -z | grep --null-data "\.h$$\|\.cc$$\|\.cpp$$" | xargs --null --no-run-if-empty clang-format -i -style=google

.PHONY: format/java
format/java:
	git ls-files -z | grep --null-data "\.java$$" | xargs --null --no-run-if-empty java -jar /opt/formatters/google-java-format-1.9-all-deps.jar --replace

.PHONY: format/dart
format/dart:
	cd flutter && ${_start_args} dart run import_sorter:main
	cd flutter && ${_start_args} dart format lib integration_test test_driver

.PHONY: format/line-endings
format/line-endings:
	git ls-files -z | xargs --null ${_start_args} dos2unix --keep-bom --

.PHONY: lint
lint: lint/bazel lint/dart lint/prohibited-extensions lint/big-files lint/line-endings

.PHONY: lint/bazel
lint/bazel:
	buildifier -lint=warn -r .

.PHONY: lint/dart
lint/dart:
	cd flutter && make prepare-flutter
	dart analyze flutter

.PHONY: lint/prohibited-extensions
lint/prohibited-extensions:
	@echo checking prohibited extensions...
	@_files=$$(git ls-files -z | grep --null-data "\.so$$\|\.apk$$\|\.tflite$$\|\.dlc$$\|\.zip$$\|\.jar$$\|\.tgz$$"); \
	if [ ! -z "$$_files" ]; then echo -e "found prohibited files:\n$$_files"; false; \
	else echo found 0 files with prohibited extensions; \
	fi

.PHONY: lint/big-files
lint/big-files:
	@echo checking big files...
	@big_file_list=""; \
	for f in $$(git ls-files); do \
		if [ "$$(stat $$f --format %s)" -gt 5242880 ]; then big_file_list="$$big_file_list$$f\n"; fi; \
	done; \
	if [ ! -z "$$big_file_list" ]; then echo -e "found too big files: \n$$big_file_list"; false; \
	else echo found 0 too big files; \
	fi

.PHONY: lint/line-endings
lint/line-endings:
	@echo checking line endings...
	@_files=$$(git ls-files -z | xargs --null ${_start_args} dos2unix --info=c --) && \
	if [ ! -z "$$_files" ]; then \
		echo -e "found files with CRLF line endings: \n$$_files"; false; \
	else echo all files have unix line endings; \
	fi

.PHONY: docker/build/format
docker/build/format:
	docker build --progress=plain \
		--build-arg UID=`id -u` --build-arg GID=`id -g` \
		-t mlperf/formatter tools/formatter

.PHONY: docker/run/format
docker/run/format:
	docker run -it --rm \
    	-v ~/.pub-cache:/home/mlperf/.pub-cache \
    	-v ~/.config/flutter:/home/mlperf/.config/flutter \
    	-v $(CURDIR):/home/mlperf/mobile_app_open \
    	-w /home/mlperf/mobile_app_open \
    	-u `id -u`:`id -g` \
    	mlperf/formatter bash -c "make format"
