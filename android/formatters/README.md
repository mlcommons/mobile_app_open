# Code formatting

### Prerequisites
You should have in $PATH:
 - `java` to run Java code formatting
 - `clang-format` to run C++ code formatting
 - `buildifier` ro run bazel configuration files formatting


## Development
You should use script `format_code.sh` in project root directory for usual development purposes. All modified files should be staged by `git`.

Example:
```
$ sh android/formatters/docker_run.sh android/format_code.sh
```

The `format_code.sh` script is the convenient way to format your staged files. It uses the
[clang-format](https://clang.llvm.org/docs/ClangFormat.html) for checking your C/C++ files,
[buildifier](https://github.com/bazelbuild/buildtools/tree/master/buildifier) for bazel files and
[google-java-format](https://github.com/google/google-java-format) for Java files.

The `format_all_code.sh` script is the convenient way to format all files.

### Using docker image
These are installed in the docker image. To make the docker image run:
```
make output/mlperf_mobile_docker_1_0.stamp
```

You can format your staged files by running:
```bash
bash android/formatters/docker_run.sh android/format_code.sh
```

You can format all files by running:
```bash
bash android/formatters/docker_run.sh android/format_all_code.sh
```

### Using native OS

To install its dependencies, do:

```bash
sudo apt install clang-format-10
go get github.com/bazelbuild/buildtools/buildifier
mkdir /opt/formatters/
curl --output /opt/formatters/google-java-format-1.9-all-deps.jar -L https://github.com/google/google-java-format/releases/download/google-java-format-1.9/google-java-format-1.9-all-deps.jar
```

Then you can format your staged files by running:
```bash
bash android/format_code.sh
```

Optionally, you can add it to pre-commit hook by running:
```bash
sed -i -e '$a\' -e 'bash `git rev-parse --show-toplevel`/android/format_code.sh && git add *' \
-e "#android/format_code.sh#d" .git/hooks/pre-commit
```

## CI
The scripts in the current directory are intended to be used primarily in the CI process. "CI" should be used as the only parameter in that case.

To run the scripts in the docker image use:
```
$ sh android/formatters/docker_run.sh android/formatters/run-bazel-format.sh CI
$ sh android/formatters/docker_run.sh android/formatters/run-clang-format.sh CI
$ sh android/formatters/docker_run.sh android/formatters/run-google-java-format.sh CI
```

The scripts format all files found in scan paths. Scan paths are different for the scripts
