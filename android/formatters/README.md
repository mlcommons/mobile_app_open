# Code formatting

### Prerequisites
You should have in $PATH:
 - `java` to run Java code formating
 - `clang-format` to run C++ code formating
 - `buildifier` ro run bazel congiguration files formating


## Development
You should use script `format_code.sh` in project root directory for usual development purposes. All modified files should be staged by `git`.

Example:
```
$ sh android/formatters/docker_run.sh android/format_code.sh
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
