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
$ sh format_code.sh
```


## CI
The scripts in the current directory are intended to be used primarily in the CI process. "CI" should be used as the only parameter in that case.

Example for bazel:
```
$ sh formatters/run-bazel-format.sh CI
```

The scripts format all files found in scan paths. Scan paths are different for the scripts
