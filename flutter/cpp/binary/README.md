# MLPerf backends

This directory provides a main binary file which can be used to evaluate the
mlperf benchmark with a specific set of (backend, dataset).

## Clone the desired branch of the TFLite backend

```bash
cd mlperf_app
git clone https://github.com/mlcommons/mobile_back_tflite -b v1_0 tflite_backend
```

## Run the TFLite backend

For example, the following command to run TFLite with the dummy dataset:

```bash
bazel run -c opt -- \
  //cpp/binary:main TFLITE DUMMY \
  --mode=SubmissionRun \
  --model_file=<path to the model file> \
  --num_threads=4 \
  --delegate=None \
  --output_dir=<mlperf output directory>
```

Each set of (backend, dataset) has a different set of arguments, so please use
`--help` argument to check which flags are available. Ex:

```bash
bazel run -c opt -- \
  //cpp/binary:main TFLITE IMAGENET --help
```

The supported backends and datasets for this binary is listed in the enum
BackendType and DatasetType in main.cc.

## On device evaluation example

Build the CLI and backend library, and push them to device:

```shell
bazel build -c opt --config android_arm64 //cpp/binary:tflitebackend //cpp/binary:main
adb push bazel-bin/tflite_backend/libtflitebackend.so /data/local/tmp
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
adb shell LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/mlperf_main_neuron EXTERNAL SQUAD \
  --mode=AccuracyOnly \
  --output_dir=/data/local/tmp/mobilebert_output \
  --model_file=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/mobilebert_float_384_gpu.tflite \
  --input_file=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/squad_eval_mini.tfrecord \
  --groundtruth_file=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/squad_groundtruth.tfrecord \
  --lib_path=/data/local/tmp/libtflitebackend.so
```
