# Build and Release Rules

## Build artifacts

We published build artifacts
at <https://github.com/mlcommons/mobile_app_open/releases>

For private files, private repo should be used.

## Versioning

* The app version is configured in the file `flutter/pubspec.yaml`
* Every build must have a unique build number. Increment the last part of the
  version number as needed, but don’t have a v1.0.1 with and without a specific
  backend.
* The build system should autoincrement the version number on build. This
  version number should be displayed in the app.
* Package (apk, exe, etc.) metadata should include the list of backends
  supported in the app

## Artifact naming convention

Release files should be named to include version numbers and backends:

Example: `mlperfbench-v1.1-qsmgt.apk`

* Q - QTI
* S - Samsung SLSI
* M - MediaTek
* G - Google Pixel
* I - Intel
* H - Huawei
* T - TFLite

Single version of the app should be used for both submission and release.
