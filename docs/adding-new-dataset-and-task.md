# Adding a new dataset and a new task

1. Add code for the new dataset [1] (e.g. `snu_sr.{cc,h}`) to `flutter/cpp/datasets` and corresponding building rule(s) in `flutter/cpp/datasets/BUILD`.
2. Add corresponding code to `flutter/cpp/binary/main.cc` so we can test the dataset with CLI.
3. Add a new task specific `benchmark_setting` to backends so that accelerators could be used. E.g. adding a new setting to TFLite backend by modifying `mobile_back_tflite/cpp/backend_tflite/tflite_settings_android.h`, for an example see [d23d16c](https://github.com/mlcommons/mobile_app_open/commit/d23d16c6cec110786379fa8d3a5e2b49e1b80b0e)
4. Add a new task to the `flutter/assets/tasks.pbtxt` file.
5. Add a new task to Flutter code part.
6. Add an integration test for the new task.

See the [PR #574](https://github.com/mlcommons/mobile_app_open/pull/574) for an example how to add the new SNUSR dataset.

See the [PR #608](https://github.com/mlcommons/mobile_app_open/pull/608) for an example how to add the new super resolution task.

See the [issue #595](https://github.com/mlcommons/mobile_app_open/issues/595) for a complete discussion how we added the new super resolution task.

[1] What does "a dataset" mean? A dataset here is to implement LoadGen's Query Sample Library (QSL). Actually, we can share some common methods and the MLPerfDriver uses interface defined in `flutter/cpp/dataset.h`, all the datasets in `flutter/cpp/datasets/` now inherit the `mlperf::mobile::Dataset`.

