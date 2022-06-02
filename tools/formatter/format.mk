
.PHONY: format
ifeq (${OS},Windows_NT)
# format/java is not supported on Windows
format: format/bazel format/clang format/line-endings format/markdown
else
format: format/bazel format/clang format/java format/line-endings format/markdown
endif

.PHONY: format/bazel
format/bazel:
	buildifier -r .

.PHONY: format/clang
format/clang:
	git ls-files -z | grep --null-data "\.h$$\|\.hpp$$\|\.cc$$\|\.cpp$$" | xargs --null --no-run-if-empty clang-format -i -style=google

.PHONY: format/java
format/java:
	git ls-files -z | grep --null-data "\.java$$" | xargs --null --no-run-if-empty java -jar /opt/formatters/google-java-format-1.9-all-deps.jar --replace

.PHONY: format/line-endings
format/line-endings:
	git ls-files -z | xargs --null ${_start_args} dos2unix --keep-bom --

.PHONY: format/markdown
format/markdown:
	git ls-files -z | grep --null-data "\.md$$" | xargs --null --no-run-if-empty markdownlint -c tools/formatter/configs/markdownlint.yml --fix --ignore 'LICENSE.md'

.PHONY: lint
lint: lint/bazel lint/prohibited-extensions lint/big-files

.PHONY: lint/bazel
lint/bazel:
	buildifier -lint=warn -r .

.PHONY: lint/yaml
lint/yaml:
	git ls-files -z | grep --null-data "\.yml$$\|\.yaml$$" | xargs --null --no-run-if-empty yamllint -c tools/formatter/configs/yamllint.yml

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

.PHONY: lint/markdown-links
lint/markdown-links:
	git ls-files -z | grep --null-data "\.md$$" | xargs --null --no-run-if-empty -n1 markdown-link-check

output/docker_mlperf_formatter.stamp: tools/formatter/Dockerfile
	docker build --progress=plain \
		--build-arg UID=`id -u` --build-arg GID=`id -g` \
		-t mlperf/formatter tools/formatter
	touch $@

.PHONY: docker/format
docker/format: output/docker_mlperf_formatter.stamp
	MSYS2_ARG_CONV_EXCL="*" docker run -it --rm \
		-v ~/.pub-cache:/home/mlperf/.pub-cache \
		-v ~/.config/flutter:/home/mlperf/.config/flutter \
		-v $(CURDIR):/home/mlperf/mobile_app_open \
		-w /home/mlperf/mobile_app_open \
		-u `id -u`:`id -g` \
		mlperf/formatter \
		make format
