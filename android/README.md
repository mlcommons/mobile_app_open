# MLPerf™ Mobile App

This project contains the MLPerf mobile app, an app-based implementationn of
[MLPerf Inference](https://github.com/mlperf/inference) tasks.

## Overview

The MLPerf app offers a simple mobile UI for executing MLPerf inference tasks
and comparing results. The user can select a task, a supported reference model
(float or quantized), and initiate both latency and accuracy validation for that
task. As single-stream represents the most common inference execution on mobile
devices, that is the default mode of inference measurement, with the results
showing the 90%-ile latency and the task-specific accuracy metric result (e.g.,
top-1 accuracy for image classification).

Several important mobile-specific considerations are addressed in the app:

*   Limited disk space - Certain datasets are quite large (multiple gigabytes),
    which makes an exhaustive evaluation difficult. By default, the app does not
    include the full dataset for validation. The client can optionally push part
    or all of the task validation datasets, depending on their use-case.
*   Device variability - The number of CPU, GPU and DSP/NPU hardware
    permutations in the mobile ecosystem is quite large. To this end, the app
    affords the option to customize hardware execution, e.g., adjusting the
    number of threads for CPU inference, enabling GPU acceleration, or NN API
    acceleration (Android’s ML abstraction layer for accelerating inference).

The initial version of the app builds off of a lightweight, C++ task evaluation
pipeline originally built for
[TensorFlow Lite](https://www.tensorflow.org/lite/). Most of the default MLPerf
inference reference implementations are built in Python, which is generally
incompatible with mobile deployment. This C++ evaluation pipeline has a minimal
set of dependencies for pre-processing datasets and post-processing, is
compatible with iOS and Android (as well as desktop platforms), and integrates
with the standard
[MLPerf LoadGen library](https://github.com/mlperf/inference/tree/master/loadgen).
While the initial version of the app uses TensorFlow Lite as the default
inference engine, the plan is to support addition of alternative inference
frameworks contributed by the broader MLPerf community.

## Requirements

*   Linux OS
*   [Bazel 2.x](https://docs.bazel.build/versions/master/install-ubuntu.html)
*   [Android SDK](https://developer.android.com/studio)
*   Android 10.0+ (API 29) w/
    [USB debugging enabled](https://developer.android.com/studio/debug/dev-options)

## Getting Started

There are three ways to build the app. If you want to make your own environment,
first make sure to download the SDK and NDK using the Android studio. The build
was tested with NDK r21e. Make sure the default system python is python3. Then, 
set the following environment variables:

```bash
export ANDROID_HOME=Path/to/SDK # Ex: $HOME/Android/Sdk
export ANDROID_NDK_HOME=Path/to/NDK # Ex: $ANDROID_HOME/ndk/(your version)
```

You will also need to install numpy and absl-py into your python environment:

```bash
pip3 install numpy absl-py
```

## Option 1 - Building Manually
The app can be built and installed with the following commands 
(execute from root directory `mobile_app_open`):

```bash
bazel build --config android_arm64 -c opt //android/java/org/mlperf/inference:mlperf_app

# Install the app with the command:
adb install -r bazel-bin/android/java/org/mlperf/inference/mlperf_app.apk
```


## Option 2 - Building Using the Prebuilt Docker Image
On the other hand, you can use a docker image to build the app:

```
make app
```
or to enable the QTI backend:
```
make WITH_QTI=1 app
```
or to enable multiple backends:
```
make WITH_QTI=1 WITH_MEDIATEK=1 app
```
Note: Follow the instruction of the backend vendor (in `mobile_back_*` folders at root level) to build for that backend.
Some backends may contain proprietary code. In that case, please contact MLCommons to get help.

# Install the app with the command:
adb install -r build/mlperf_app.apk
```

If you want to build an instrumented test APK you should add target `//androidTest:mlperf_test_app`, i.e. :
```bash
make test_app

adb install -r build/mlperf_test_app.apk
```

## Option 3 - Building With Android Studio
Make sure you follow the Getting Started steps listed above to set up your SDK/NDK paths and python dependencies. Once that's done, follow these steps:

1. Install Android Studio (version >= 4.0)
2. In Android Studio, navigate to File > Settings > Plugins
3. Search for and install the 'Bazel' plugin
4. Restart Android Studio
5. File > Import Bazel Project and follow the instructions
6. Run > Edit Configurations...
7. Click the + button in the upper-left corner and select 'Bazel Command' from the list
8. Click the + button in the 'Target expression' box and enter ```//android/java/org/mlperf/inference:mlperf_app```
9. Select 'Build' from the 'Bazel command' dropdown
10. Enter the following into the 'Bazel flags' box:
```
--cxxopt='--std=c++14'
--host_cxxopt='--std=c++14'
--fat_apk_cpu=x86_64,arm64-v8a
```
11. Run > Run 'inference:mlperf_app'

Please see [these instructions](docs/guides/installation.md) for installing and using the app.

## Adding a new Backend

The following steps are required to add your backend:

1. Create a mobile_back_[vendor] top level directory that builds your backend using android/cpp/c/backend_c.h
2. Implement all the C API backend interfaces defined in android/cpp/c/backend_c.h
3. Add a bazel BUILD file to create lib[vendor]backend.so shared library

You can look at the [default TFLite backend implementation](../../mobile_back_tflite) for a reference.
The following steps are required to add your backend:

### Unified app changes
In java/org/mlperf/inference/BUILD, add the following replacing
_vendor_ with the actual vendor/backend name:

```
string_flag(name = "with_vendor", build_setting_default = "1")
config_setting(
    name = "use_vendor",
    flag_values = {
        ":with_vendor": "1",
    },
)

```
Replace "vendor" in the the code below with your backend name in equivalent sections
of java/org/mlperf/inference/BUILD:
```
    deps = [":evaluation_app_lib"] +
           select({
             ":use_vendor": ["@vendorbackend"],
             "//conditions:default": ["@vendormockbackend"],
    }),
```
Modify java/org/mlperf/inference/Backends.java.in to add your backend and
modify the genrule in java/org/mlperf/inference/BUILD to enable your
backend if it is enabled in the build. **The order is important! Backends will be
probed in the order of occurence in the list and the TFLite backend must be last.**

Modify the Makefile to change release-app to use your backend when
building with --//android/java/org/mlperf/inference:with_vendor="1"
substituting your backend name for vendor.

## FAQ

#### Will this be available in the app store(s)?

Yes, eventually, but not with the 0.7 release.

#### When will an iOS version be avilable?

This is a priority for the community but requires some additional resourcing.

#### Will the app support all MLPerf Inference tasks?

That is the eventual goal. To start, it supports only those tasks specifically
targeting mobile and/or edge use-cases (e.g., Classification w/ MobileNet,
Detection w/ SSD MobileNet).

#### Will the app support more than just TensorFlow Lite for inference?

Yes, that is the plan, though this is largely dependent on contributions from
teams and organizations who desire this.

Please search
https://groups.google.com/forum/#!forum/mlperf-inference-submitters for
additional help and related questions.
