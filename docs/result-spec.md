# Result format

This file explains the full format for results generated by this app.

This format is used when saving results locally on device, when uploading them to the online database,
and when fetching them from the online database.

## General structure

Results are serialized as JSON.

Results must be a map with the following items in the root level:

* `meta`: map
  * `uuid`: string  
  UUID is generated by the app when all benchmarks are finished.
  * `upload_date`: string  
  Datetime of the moment when web service received the upload request
  Format is Iso 8601 in UTC timezone: `2022-04-14T03:54:54.687Z`
* `results`: list of maps. See [List of benchmark-specific results](#benchmark-specific-results) section
* `environment_info`: map. See [Environment info](#environment-info) section
* `build_info`: map. See [Build info](#application-build-info) section

## Benchmark-specific results

Each benchmark generates one map with results.

For example, if you select 3 benchmarks, you will get a list with 3 items, each describing specific benchmark.

Almost all of the settings are the same for performance and accuracy runs so they are united.

If Submission mode is disabled, `accuracy_run` will be null for all results.
If you enable Submission mode, both `performance_run` and `accuracy_run` values will be filled.

* `benchmark_id`: string
* `benchmark_name`: string  
  Value from `task.model.name` for this benchmark from selected tasks.pbtxt file.
* `loadgen_scenario`: string enum  
  See [`::mlperf::TestScenario`](https://github.com/mlcommons/inference/blob/a67f9f34bcc4439af4740095958c23380f9b284b/loadgen/test_settings.h#L38).  
  Allowed values:
  * `SingleStream`
  * `Offline`
* `backend_settings`: map  
  Settings defined by selected backend for this benchmark.
  * `accelerator_code`: string
  * `accelerator_desc`: string
  * `framework`: string
  * `model_path`: string
  * `batch_size`: integer number
  * `extra_settings`: list of maps  
    Extra settings that can vary between different benchmarks and backends.  
    Here must be stored values set by backend in `common_setting`.  
    `shards_num` value for TFLite backend should be located here.  
    Map structure:
    * `id`: string. Value from `setting.id` that is passed to backend
    * `name`: string. Value from `setting.name` that is passed to backend
    * `value`: string. Value from `setting.value.value` that is passed to backend
    * `value_name`: string. Value from `setting.value.name` that is passed to backend
* `performance_run`: map  
  May be null if performance was not tested in this benchmark.
  * `throughput`: floating point number  
    Throughput value for this run of the benchmark.
    May be null for an accuracy run.
  * `accuracy`: map
    May be null for a performance run if groundtruth file is not provided.
    * `normalized`: floating point number  
      Accuracy value for this run of the benchmark.
      Value must be normalized between `0.0` and `1.0`.
    * `formatted`: string  
      Formatted accuracy string, often with measuring unit suffix
  * `measured_duration`: floating point number  
    Actual duration of the benchmark in seconds from start to finish.
  * `measured_samples`: integer number  
    Actual number of samples evaluated during the benchmark
  * `loadgen_info`: map  
    Info provided by loadgen. May be null for accuracy runs.
    * `validity`: bool  
      Indicates whether all constraints were satisfied or not.
    * `duration`: floating point number  
      Duration of the benchmark without loadgen overhead in seconds.
  * `start_datetime`: string  
    Datetime of the moment when benchmark started  
    Format is Iso 8601 in UTC timezone: `2022-04-14T03:54:54.687Z`
  * `dataset`: map  
    Dataset info for this benchmark from selected `tasks.pbtxt` file.
    * `name`: string
    * `type`: string enum  
      Allowed values (this list may be extended when we add support for more datasets):
      * `IMAGENET`
      * `COCO`
      * `ADE20K`
      * `SQUAD`
    * `data_path`: string
    * `groundtruth_path`: string
* `accuracy_run`: map  
  Same as `performance_run`.
  May be null if accuracy was not tested in this benchmark.
* `min_duration`: floating point number  
  Value from `task.min_duration` for this benchmark from selected tasks.pbtxt file.
* `min_samples`: integer number  
  Value from `task.min_query_count` for this benchmark from selected tasks.pbtxt file.
* `backend_info`: map
  * `filename`: string  
    Actual filename of the backend
  * `backend_name`: string  
    Backend name reported by backend
  * `vendor_name`: string  
    Vendor name reported by backend
  * `accelerator_name`: string  
    Backend-defined string describing actual accelerator used during this benchmark.  
    Should typically match `accelerator_desc` from the `backend_settings` map but may be different in case of accelerator fallback.

## Environment info

Info about environment the app is running in. May change when you update your OS, change device hardware, or use another device.

* `platform`: string  
  Used to determine which `info` entry should be used.  
  Currently device type simply maps to supported OS list but this may change in the future.  
  Allowed values:  
  * `android`
  * `ios`
  * `windows`
* `value`: map  
  Info about device software and underlying hardware.
  Should contain exactly one valid field, according to device type.
  * `android`: map  
    Must be null if device type is not Android.
    * `os_version`: string  
      Must be obtained from environment.
    * `manufacturer`: string. Manufacturer of the device  
      <!-- markdown-link-check-disable-next-line -->
      Value of `manufacturer` from [Android build constants](https://developer.android.com/reference/android/os/Build#MANUFACTURER)
    * `model_code`: string. Manufacturer-defined model code  
      <!-- markdown-link-check-disable-next-line -->
      Value of `model` from [Android build constants](https://developer.android.com/reference/android/os/Build#MODEL)  
      For example: `SM-G981U1`.
    * `model_name`: string. Human-readable model name  
      Marketing name that corresponds to `model_code`.  
      For example: `Galaxy S20 5G`.
    * `board_code`: string  
      <!-- markdown-link-check-disable-next-line -->
      Value of `board` from [Android build constants](https://developer.android.com/reference/android/os/Build#BOARD)
    * `proc_cpuinfo_soc_name`: string  
      SoC name obtained from `/proc/cpuinfo` file.  
      This field may serve as a backup option if SoC name can't be determined via other methods.  
      May be null.
    * `props`: array of maps  
      Contains data obtained via `getprop` Android util.
      May be extended by adding vendor-specific properties.
      * `type`: string  
        Type of data entry. Possible values: `soc_manufacturer`, `soc_model`. The list may be extended in the future.
      * `name`: string  
        Name of the property. Main purpose is to gather statistics which values can actually be seen on a phone.
        Currently known values:
        * `ro.soc.model`
        * `ro.soc.manufacturer`
      * `value`: string  
        Result of `getprop <name>`
  * `ios`: map  
    Must be null is device type is not iOS.
    * `os_version`: string  
      Must be obtained from environment.
    * `model_code`: string. Manufacturer-defined model code  
      Apple Machine name. For example: `iPhone14,5`.  
    * `model_name`: string. Human-readable model name  
      Marketing name that corresponds to `model_code`. For example: `iPhone 13`.
    * `soc_name`: string  
      Full SoC name.
  * `windows`: map  
    Must be null is device type is not Windows.
    * `os_version`: string  
      Must be obtained from environment.
    * `cpu_full_name`: string  
      Should contain CPU name as reported by CPU.

## Application build info

Constant info for this build of the app. The only way to change it is to use a different version of the app.

* `version`: string
* `build_number`: string
* `official_release_flag`: bool  
  Indicates if the official release flag was set for this build
* `dev_test_flag`: bool  
  Indicated if development test flag was set for this build
* `backend_list`: list of strings  
  Must contain actual list of backends that are included into this version of the app.
* `git_branch`: string
* `git_commit`: string
* `git_dirty_flag`: bool  
  Indicates if there are any local changes compared to the commit specified in `git_commit`.