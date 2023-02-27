
.PHONY: format
format: format/bazel format/clang format/dart format/ts format/line-endings format/markdown
	@echo "Finished running make target: format"

.PHONY: format/bazel
format/bazel:
	buildifier -r .

.PHONY: format/clang
format/clang:
	git ls-files -z | grep --null-data "\.h$$\|\.hpp$$\|\.cc$$\|\.cpp$$\|\.proto$$" | xargs --null --no-run-if-empty clang-format -i -style=google

.PHONY: format/dart/pub-get
format/dart/pub-get:
	cd flutter && ${_start_args} dart pub get
	cd flutter_common && ${_start_args} dart pub get

.PHONY: format/dart
format/dart: format/dart/pub-get
	cd flutter && ${_start_args} dart run import_sorter:main
	cd flutter_common && ${_start_args} dart run import_sorter:main
	dart format flutter flutter_common

.PHONY: format/ts/install
format/ts/install:
	cd firebase_functions/functions && ${_start_args} npm install --production=false

.PHONY: format/ts
format/ts: format/ts/install flutter/result/ts flutter/firebase/prefix
	cd firebase_functions/functions && ${_start_args} npm run format
	cd firebase_functions/functions && ${_start_args} npm run lint-fix

.PHONY: format/line-endings
format/line-endings:
	git ls-files -z | xargs --null ${_start_args} dos2unix --keep-bom --quiet --

.PHONY: format/markdown
format/markdown:
	git ls-files -z | \
		grep --null-data -v "LICENSE.md" | \
		grep --null-data "\.md$$" --exclude="*LICENSE.md" | \
		xargs --null --no-run-if-empty markdownlint -c tools/formatter/configs/markdownlint.yml --fix

.PHONY: lint
lint: lint/bazel lint/clang lint/dart lint/ts lint/yaml lint/prohibited-extensions lint/big-files lint/result-schema
	@echo "Finished running make target: lint"

.PHONY: lint/bazel
lint/bazel:
	buildifier -lint=warn -mode=check -r .

.PHONY: lint/clang
lint/clang:
	git ls-files -z | grep --null-data "\.h$$\|\.hpp$$\|\.cc$$\|\.cpp$$\|\.proto$$" | xargs --null --no-run-if-empty clang-format -i -style=google --Werror --dry-run

.PHONY: lint/dart
lint/dart: format/dart/pub-get
	cd flutter && ${_start_args} dart run import_sorter:main --exit-if-changed
	cd flutter_common && ${_start_args} dart run import_sorter:main --exit-if-changed
	dart format --output=none --set-exit-if-changed flutter flutter_common
	dart analyze --fatal-infos

.PHONY: lint/yaml
lint/yaml:
	git ls-files -z | grep --null-data "\.yml$$\|\.yaml$$" | xargs --null --no-run-if-empty yamllint -c tools/formatter/configs/yamllint.yml

.PHONY: lint/ts
lint/ts: format/ts/install flutter/result/ts flutter/firebase/prefix
	cd firebase_functions/functions && ${_start_args} npm run lint
	cd firebase_functions/functions && npx prettier --check .

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

.PHONY: lint/markdown
lint/markdown:
	git ls-files -z | \
		grep --null-data -v "LICENSE.md" | \
		grep --null-data "\.md$$" --exclude="*LICENSE.md" | \
		xargs --null --no-run-if-empty markdownlint -c tools/formatter/configs/markdownlint.yml

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
	touch $@

FORMAT_DOCKER_ARGS= \
	--mount source=mlperf-pubcache,target=/home/.pub-cache \
	--mount source=mlperf-output,target=/home/mlperf/mobile_app_open/output \
	-v $(CURDIR):/home/mlperf/mobile_app_open \
	-w /home/mlperf/mobile_app_open \
	mlperf/formatter \

.PHONY: docker/format
docker/format: output/docker_mlperf_formatter.stamp
	MSYS2_ARG_CONV_EXCL="*" docker run -it --rm \
		${FORMAT_DOCKER_ARGS} \
		make flutter/prepare format

.PHONY: docker/lint
docker/lint: output/docker_mlperf_formatter.stamp
	MSYS2_ARG_CONV_EXCL="*" docker run -it --rm \
		${FORMAT_DOCKER_ARGS} \
		make flutter/prepare lint

.PHONY: docker/format/--
docker/format/--: output/docker_mlperf_formatter.stamp
	MSYS2_ARG_CONV_EXCL="*" docker run -it --rm \
		${FORMAT_DOCKER_ARGS} \
		bash
