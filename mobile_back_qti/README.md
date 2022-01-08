# MLPerf Mobile App SNPE Backend

This subdirectory contains the QTI backend for the MLPerf mobile app, an app-based
implementationn of [MLPerf Inference](https://github.com/mlperf/inference) tasks.

## Overview

This repository builds the libqtibackend.so backend and prepares the libraries and
SNPE DLC files for integration with the MLPerf app. These DLC files have been
uploaded with the other submission files to here: `<path where needs to be uploaded>`

## Requirements

* [SNPE SDK](https://developer.qualcomm.com/software/qualcomm-neural-processing-sdk)
  * Version 1.54.2
* Linux machine capable of running Ubuntu 18.04 docker images

After downloading and unzipping the SNPE SDK, make sure to set SNPE_SDK to its location:

```shell
cd /opt
unzip snpe-1.54.2.2899.zip
export SNPE_SDK=/opt/snpe-1.54.2.2899
```

### Optional

If you wish to rebuild the DLC files yourself, you will have these additional requirements:

* Imagenet dataset (LSVRC2012_img_val.tar) put in the build/imagenet/downloads directory
* Linux machine also capable of running Tensorflow debian based docker images

Use your browser to download the SNPE SDK using the links above.

Create your Github personal access token.

```shell
export GITHUB_TOKEN=<your-personal-access-token>
cd DLC/ && make
```

It will take a couple hours on an 8-core Xeon workstation to generate the DLC files.

## Building the MLPerf app with the QTI backend

Manual steps:

* Extract the SNPE SDK (from Requirements above) and set SNPE_SDK to its location.

* If you have an HTTP proxy, you may need the following

```shell
sudo apt install ca-certificates-java
export USE_PROXY_WORKAROUND=1
```

Clone mlperf_app_open and build with the following build commands.

```shell
git clone https://github.com/mlcommons/mobile_app_open
cd mobile_app_open
make WITH_QTI=1 android/app
```

This will build the QTI backend into the MLPerf app.

## Building the QTI backend lib

To build only the QTI backend:

```shell
git clone https://github.com/mlcommons/mobile_app_open
make WITH_QTI=1 libqtibackend
```

## Backend Specific Task Config file

The task config settings are embedded in libqtibackend.so. These settings contain the
backend specific data for each task to be run. This backend assumes a few things about
the settings:

1. All the models use "SNPE" for the configuration name and use "snpe_aip", "snpe_dsp",
   "psnpe_aip", or "psnpe_dsp" for the accelerator value when using SNPE.

## FAQ

### Do I need to build the DLC files?

No, the information to build the DLC files is only to show how they are created.

### What devices does this backend support?

This backend only supports SDM865/SDM865 Pro ,SDM888/SDM888 Pro and SDM778G devices. Other Snapdragon
based devices will not run the MLPerf app. Future updates of the app will provide
additional device support.

### Is SNPE used to run all the models?

Yes. All the models use SNPE for execution.
