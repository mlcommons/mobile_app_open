
.PHONY: format
ifeq (${OS},Windows_NT)
# format/java is not supported on Windows
format: format/bazel format/clang format/dart format/ts format/line-endings format/markdown
else
format: format/bazel format/clang format/java format/dart format/ts format/line-endings format/markdown
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
	cd flutter_common && ${_start_args} dart run import_sorter:main
	cd website && ${_start_args} dart run import_sorter:main
	dart format flutter flutter_common website

.PHONY: format/ts
format/ts:
	cd firebase_functions/functions && ${_start_args} npm run format
	cd firebase_functions/functions && ${_start_args} npm run lint-fix

.PHONY: format/line-endings
format/line-endings:
	git ls-files -z | xargs --null ${_start_args} dos2unix --keep-bom --quiet --

.PHONY: format/markdown
format/markdown:
	git ls-files -z | grep --null-data "\.md$$" | xargs --null --no-run-if-empty markdownlint -c tools/formatter/configs/markdownlint.yml --fix --ignore 'LICENSE.md'

.PHONY: lint
lint: lint/bazel lint/dart lint/ts lint/yaml lint/prohibited-extensions lint/big-files lint/result-schema

.PHONY: lint/bazel
lint/bazel:
	buildifier -lint=warn -r .

.PHONY: lint/dart
lint/dart:
	dart analyze --fatal-infos flutter flutter_common website

.PHONY: lint/yaml
lint/yaml:
	git ls-files -z | grep --null-data "\.yml$$\|\.yaml$$" | xargs --null --no-run-if-empty yamllint -c tools/formatter/configs/yamllint.yml

.PHONY: lint/ts
lint/ts:
	cd firebase_functions/functions && ${_start_args} npm run lint

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

lint_result_json_schema_path=output/extended-result.schema.json
.PHONY: lint/result-schema
lint/result-schema:
	mkdir -p $$(dirname ${lint_result_json_schema_path})
	make RESULT_JSON_SCHEMA_PATH=${lint_result_json_schema_path} flutter/result/schema >/dev/null 2>/dev/null
	@[ "$$(cat ${default_result_json_schema_path})" = "$$(cat ${lint_result_json_schema_path})" ] \
		|| (echo ${default_result_json_schema_path} is outdated; echo run \`make flutter/result/schema\` to update it; exit 1)

output/docker_mlperf_formatter.stamp: tools/formatter/Dockerfile
	docker build --progress=plain \
		--build-arg UID=`id -u` --build-arg GID=`id -g` \
		-t mlperf/formatter tools/formatter
	# need to clean flutter cache first else we will have error when running `dart run import_sorter:main` later in docker
	cd flutter && ${_start_args} flutter clean
	touch $@

FORMAT_DOCKER_ARGS= \
	-v ~/.pub-cache:/home/mlperf/.pub-cache \
	-v ~/.config/flutter:/home/mlperf/.config/flutter \
	-v $(CURDIR):/home/mlperf/mobile_app_open \
	-w /home/mlperf/mobile_app_open \
	-u `id -u`:`id -g` \
	mlperf/formatter \

.PHONY: docker/format
docker/format: output/docker_mlperf_formatter.stamp
	MSYS2_ARG_CONV_EXCL="*" docker run -it --rm \
		${FORMAT_DOCKER_ARGS} \
		make format

.PHONY: docker/format/--
docker/format/--: output/docker_mlperf_formatter.stamp
	MSYS2_ARG_CONV_EXCL="*" docker run -it --rm \
		${FORMAT_DOCKER_ARGS} \
		bash
