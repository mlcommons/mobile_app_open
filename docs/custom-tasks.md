# Custom task config

This file describes how to configure the application to use non-default task settings.

## Contents

* [Using custom tasks.pbtxt file](#using-custom-taskspbtxt-file)
* [How to specify a resource](#how-to-specify-a-resource)
* [Overriding default folders](#overriding-default-folders)
* [Using external resources on an iPhone](#using-external-resources-on-an-iphone)
* [Using external resources on an Android](#using-external-resources-on-an-android)
* [Using external resources on an Windows](#using-external-resources-on-an-windows)

## Using custom tasks.pbtxt file

When the app starts for the first time, it creates a file named `benchmarksConfigurations.json`
in the root application directory.
Each entry in the `benchmarksConfigurations.json` is the name and location of a configuration file.
You can have multiple configurations stored in the `benchmarksConfigurations.json` and switch
between them in the app settings.
The `benchmarksConfigurations.json` looks like this:

```json
{
   "default": "asset://assets/tasks.pbtxt",
   "Custom config name": "path/to/custom/tasks.pbtxt"
}
```

See [How to specify a resource](#how-to-specify-a-resource) for details on how to specify a path to the config.

The `tasks.pbtxt` file is a text file in [protobuf](https://developers.google.com/protocol-buffers) format.
The format specification for this app is [here](../flutter/cpp/proto/mlperf_task.proto).
The easiest way to create a custom config is to copy and modify [the default config](../flutter/assets/tasks.pbtxt).

After adding new option into the `benchmarksConfigurations.json` file
run application and open setting screen.  
Tap on `Task configuration` option
and choose added item. Current chosen path is highlighted with blue color.

## How to specify a resource

You can use several types of resources:

1. URL of a file  
File will be automatically downloaded.  
Allowed schemes: `http`, `https`.
2. URL of a .zip archive  
Archive will be automatically downloaded and unzipped.
3. Local resource in a data folder  
You can specify a path starting with `local://`. For example: `local:///mlperf_datasets/some/folders/test.txt` (note the 3 slashes).  
The app will expect to find a file with path `mlperf_datasets/some/folders/test.txt` relative to the data directory.  
Data directory can be changed in settings, except for iOS, and its possible locations depend on the platform.
4. Local resource with an absolute path  
You can specify the full path, which will not depend on the data folder.  
Absolute path is only available for the task config path. All local resources in the task config must point to the data folder.

## Using external resources on an iPhone

On iOS an application resource folder can be found in `On My iPhone` -> `<app name>`.
So, if you have `local:///mlperf_datasets/some/folders/test.txt` path in your config file,
then the path on your iPhone would be: `MLPerf/mlperf_datasets/some/folders/test.txt`.

Note, that iPhone's `Files` app has issues with moving folders with big number of files into application folders.
So if you have an archive with big dataset and want to place it in the app resources folder,
you should copy (or download) this archive into the app folder, and then unpack it in place.
Unpacking may also be unstable, when your archive contains dozens of thousands of files,
but you shouldn't have any issues if you don't touch your phone while unpacking is in progress.

Also note, that sometimes iPhone's `Files` app may prevent you from copying files with exotic extensions (like `.pbtxt`)
from some external file managers and cloud storage clients.
Instead of copying your file using `Files`, you can use Share button in your external app and choose "Save to Files".
This method doesn't have restrictions on file extension.

## Using external resources on an Android

In the Android version of the app the data folder points to the app folder by default.

On Android the application folder is located at
`/storage/emulated/0/Android/data/org.mlcommons.android.mlperfbench/files`.  
If you have `local:///mlperf_datasets/some/folders/test.txt` path in your config file,
then the path on your Android phone would be
`/storage/emulated/0/Android/data/org.mlcommons.android.mlperfbench/files/mlperf_datasets/some/folders/test.txt`.

You may want to change the data folder to a custom one for several reasons:
issues accessing the `/Android/data/` folder,
data persistence between app reinstalls.

Adjust the file paths according to the folder you choose.  
For example, if you selected the `Documents` standard folder
the resulting path for the resource mentioned above would instead be
`/storage/emulated/0/Documents/mlperf_datasets/some/folders/test.txt`.

## Using external resources on an Windows

On Windows the application folder is located at `%USERPROFILE%/Documents/MLCommons/MLPerfBench/`.
If you have `local:///mlperf_datasets/some/folders/test.txt` path in your config file,
then the path on your Windows device would be
`%USERPROFILE%/Documents/MLCommons/MLPerfBench/mlperf_datasets/some/folders/test.txt`.

You can choose a custom data folder. Adjust paths accordingly.
