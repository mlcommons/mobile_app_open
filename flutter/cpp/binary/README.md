# MLPerf backends

This directory provides a main binary file which can be used to evaluate the
mlperf benchmark with a specific set of (backend, dataset).

## Build the TFLite backend

```bash
bazel build -c opt --cxxopt=-std=c++17 --host_cxxopt=-std=c++17 \
  //mobile_back_tflite:tflitebackend \
  --spawn_strategy=standalone
```

## Run the TFLite backend

For example, use the following command to run TFLite with the ImageNet dataset:

1. build the command programm `//flutter/cpp/binary:main`
```bash
bazel build -c opt --cxxopt=-std=c++17 --host_cxxopt=-std=c++17 \
  --spawn_strategy=standalone //flutter/cpp/binary:main
``` 
2. run it
```bash
bazel-bin/flutter/cpp/binary/main EXTERNAL IMAGENET
  --mode=SubmissionRun \
  --model_file=<path to the model file> \
  --output_dir=<mlperf output directory>
  ....
```

Each set of (backend, dataset) has a different set of arguments, so please use
`--help` argument to check which flags are available. E.g., with

```bash
bazel-bin/flutter/cpp/binary/main EXTERNAL IMAGENET --help
```
we get
```
usage: bazel-bin/flutter/cpp/binary/main EXTERNAL IMAGENET <flags>
Flags:
	--mode=                                    	string	required	Mode is one among PerformanceOnly, AccuracyOnly, SubmissionRun.
	--output_dir=                              	string	required	The output directory of mlperf.
	--model_file=                              	string	required	Path to model file.
	--images_directory=                        	string	required	Path to ground truth images.
	--offset=1                                 	int32	required	The offset of the first meaningful class in the classification model.
	--groundtruth_file=                        	string	required	Path to the imagenet ground truth file.
	--min_query_count=100                      	int32	optional	The test will guarantee to run at least this number of samples in performance mode.
	--min_duration=100                         	int32	optional	The test will guarantee to run at least this duration in performance mode. The duration is in ms.
	--single_stream_expected_latency_ns=1000000	int32	optional	single_stream_expected_latency_ns
	--lib_path=                                	string	optional	Path to the backend library .so file.
	--native_lib_path=                         	string	optional	Path to the additioal .so files for the backend.
	--scenario=SingleStream                    	string	optional	Scenario to run the benchmark.
	--image_width=224                          	int32	optional	The width of the processed image.
	--image_height=224                         	int32	optional	The height of the processed image.
	--scenario=SingleStream                    	string	optional	Scenario to run the benchmark.
	--batch_size=1                             	int32	optional	Batch size.

```

The supported backends and datasets for this binary is listed in the enum
BackendType and DatasetType in main.cc.

## On device evaluation example

Build the CLI and backend library, and push them to device:

```shell
bazel build -c opt --config android_arm64 //mobile_back_tflite:tflitebackend //flutter/cpp/binary:main --spawn_strategy=standalone
adb push bazel-bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so /data/local/tmp
adb push bazel-bin/cpp/binary/main /data/local/tmp/mlperf_main
```

Assuming

1. the `mlperf_app` apk was installed so that models are in
   `/sdcard/Android/data/org.mlperf.inference/files/cache/cache/`
   (Surely you can put models to the path you like and change related model
   file settings accordingly), and
2. ImageNet, COCO 2017, and ADE20K are in `/sdcard/mlperf_datasets/`

Evaluating top-1 accuracy of quantized MobileNet EdgeTPU tflite with ImageNet validation set.

```shell
adb shell LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/mlperf_main EXTERNAL IMAGENET \
  --mode=AccuracyOnly \
  --output_dir=/data/local/tmp/mobilenet_edgetpu_output \
  --model_file=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/mobilenet_edgetpu_224_1.0_uint8.tflite \
  --images_directory=/sdcard/mlperf_datasets/imagenet/img \
  --groundtruth_file=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/imagenet_val_full.txt \
  --offset=1 \
  --lib_path=/data/local/tmp/libtflitebackend.so
```

Evaluating accuracy of quaintized MobileDet tflite with COCO 2017 validation set.

```shell
adb shell LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/mlperf_main EXTERNAL COCO \
  --mode=AccuracyOnly \
  --output_dir=/data/local/tmp/coco_output \
  --model_file=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/mobiledet_qat.tflite \
  --images_directory=/sdcard/mlperf_datasets/coco/img \
  --groundtruth_file=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/coco_val_full.pbtxt \
  --image_width=320 --image_height=320 \
  --offset=1 --num_classes=91 \
  --lib_path=/data/local/tmp/libtflitebackend.so
```

Evaluating accuracy of quaintized DeepLabv3 tflite with ADE20K validation set.

```shell
adb shell LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/mlperf_main EXTERNAL ADE20K \
  --mode=AccuracyOnly \
  --output_dir=/data/local/tmp/ade20k_output \
  --model_file=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/deeplabv3_mnv2_ade20k_uint8.tflite \
  --images_directory=/sdcard/mlperf_datasets/ade20k/images \
  --ground_truth_directory=/sdcard/mlperf_datasets/ade20k/annotations \
  --lib_path=/data/local/tmp/libtflitebackend.so
```

Evaluating accuracy of floating point MobileBERT tflite with SQuAD V1.1 mini evaluation set.

```shell
adb shell LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/mlperf_main EXTERNAL SQUAD \
  --mode=AccuracyOnly \
  --output_dir=/data/local/tmp/mobilebert_output \
  --model_file=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/mobilebert_float_384_gpu.tflite \
  --input_file=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/squad_eval_mini.tfrecord \
  --groundtruth_file=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/squad_groundtruth.tfrecord \
  --lib_path=/data/local/tmp/libtflitebackend.so
```
