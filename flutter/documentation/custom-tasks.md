
This file describes how to configure the application to use non-default benchmark settings.

# Contents

* [Using custom tasks.pbtxt file](#using-custom-taskspbtxt-file)
* [Resources in the tasks.pbtxt](#resources-in-the-taskspbtxt)
* [Using external resources on an iPhone](#using-external-resources-on-an-iphone)

# Using custom tasks.pbtxt file

Create file named `benchmarksConfigurations.json`
and put it into root application directory.
Add new paths to the file `tasks.pbtxt` in following format:

`${Name of option}: ${path}`

`tasks.pbtxt` file is a resource.
See [Resources in the tasks.pbtxt](#resources-in-the-taskspbtxt) block
for details on different ways you can specify resources.

After adding new option into the `benchmarksConfigurations.json` file
run application and open setting screen.  
Tap on `Benchmarks configuration path` option
and choose added item. Current chosen path highlighted blue.

# Resources in the tasks.pbtxt

You can specify several types of resources in a `tasks.pbtxt` file:
1. URL of a file  
File will be automatically downloaded.
2. URL of a .zip archive  
Archive will be automatically downloaded and unzipped.
3. External resource  
You can specify a path starting with `/sdcard/mlperf_datasets/`.  
For example: `/sdcard/mlperf_datasets/some/folders/test.txt`.  
The app will expect to find a file with path `some/folders/test.txt`
relative to the external resources directory.  
Location of the external resources directory is platform dependent.

# Using external resources on an iPhone

On iOS an application resource folder can be found in `On My iPhone` -> `<app name>`.
The `external_resources` folder corresponds to the `mlperf_datasets` folder in path specified in `tasks.pbtxt`.
So, if you have `/sdcard/mlperf_datasets/some/folders/test.txt` path in your config file,
then the path on your iPhone would be: `MLPerf/external_resources/some/folders/test.txt`.

Note, that iPhone's `Files` app has issues with moving folders with big number of files into application folders.
So if you have an archive with big dataset and want to place it in the app resources folder,
you should copy (or download) this archive into the app folder, and then unpack it in place.
Unpacking may also be unstable, when your archive contains dozens of thousands of files,
but you shouldn't have any issues if you don't touch your phone while unpacking is in progress.

Also note, that sometimes iPhone's `Files` app may prevent you from copying files with exotic extensions (like `.pbtxt`)
from some external file managers and cloud storage clients.
Instead of copying your file using `Files`, you can use Share button in your external app and choose "Save to Files".
This method doesn't have restrictions on file extension.

# Using external resources on an Android
On Android the resource folder is located at `/Android/data/org.mlcommons.android.mlperfbench/files`.
On Android 11, the folder `/Android/data/` is inaccessible using the default File Manager app. 
It stills accessible through 3rd party File Manager apps, though. 
Or using a File Manager on a desktop computer and access the files via USB also works.

So, if you have `/sdcard/mlperf_datasets/some/folders/test.txt` path in your config file,
then the path on your Android phone would be:
`/Android/data/org.mlcommons.android.mlperfbench/files/external_resources/some/folders/test.txt`.
