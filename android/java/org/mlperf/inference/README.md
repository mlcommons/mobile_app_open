## MLPerf Evaluation App

This Android app evaluates the on-device MLPerf inference benchmark.

## Build & Run the app

To build and install the app, do:

```
# EVALUATION_APP_PATH=//java/org/mlperf/inference
# bazel build -c opt --cxxopt='--std=c++14' --fat_apk_cpu=x86_64,arm64-v8a ${EVALUATION_APP_PATH}:mlperf_app
# adb install -r bazel-bin/${EVALUATION_APP_PATH}/mlperf_app.apk
```

Please click on the hamburger menu icon to go to `Settings` and change settings
(whether to use accelerator, number of threads).
