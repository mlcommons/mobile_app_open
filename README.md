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

## Folder Structure

* android - Contains the Android version of the app
* flutter - Contains the Flutter (cross-platform) version of the app
* mobile_back_qti - QTI backend for Android
* mobile_back_samsung - Samsung backend for Android
* mobile_back_pixel - Google Pixel backend for Android
* mobile_back_tflite - Combined TFLite / MediaTek backends for Android
* datasets - contains scripts to prepare test and calibration data used for accuracy evaluation and model quantization.
