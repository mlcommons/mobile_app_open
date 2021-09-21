## Getting Started

Due to restrictions in dataset redistribution, some amount of manual retrieval, processing
and packaging is required to use certain task datasets for accuracy evaluation.

If you're primarily interested in testing inference latency, skip directly to
the [App Execution](#app-execution) section. The app can run MLPerf latency measurements
without the need for full dataset installation.

### Dataset installation

A tasks-specific dataset is required for measuring accuracy.

This currently requires some manual work to prepare and distribute the relevant datasets.
Note that the full datasets can be extremely large; a subset of the full dataset can be
prepared to give a useful proxy for task-specific accuracy. When prepared, each task
directory should be pushed to `/sdcard/mlperf_datasets/...`.

You can follow [this guide](https://github.com/mlcommons/mobile_app_open/blob/master/android/cpp/datasets/README.md) 
to download the datasets.

## <a name="app-execution"></a> App Execution

After the app has been installed, and the optional dataset(s), simply open
**`MLPerf Mobile`** from the Android launcher. The app UI allows selection of supported
tasks, and execution of the supported models (float/quantized) for each task.

If **`Submission mode`** is enabled, the task-specific accuracy will be measured, otherwise it
will only report latency results.

