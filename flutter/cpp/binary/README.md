# MLPerf backends

This directory provides a main binary file which can be used to evaluate the
MLPerf benchmark with a specific set of (backend, benchmark_id).

## Build the TFLite backend

```bash
bazel build -c opt --cxxopt=-std=c++17 --host_cxxopt=-std=c++17 \
  //mobile_back_tflite:tflitebackend \
  --spawn_strategy=standalone
```

## Run the TFLite backend

For example, use the following command to run TFLite with the `image_classification` benchmark:

1. build the command program `//flutter/cpp/binary:main`

   ```bash
   bazel build -c opt --cxxopt=-std=c++17 --host_cxxopt=-std=c++17 \
     --spawn_strategy=standalone //flutter/cpp/binary:main
   ```

2. run it

   ```bash
   bazel-bin/flutter/cpp/binary/main EXTERNAL image_classification
     --mode=SubmissionRun \
     --model_file=<path to the model file> \
     --output_dir=<mlperf output directory>
     ....
   ```

Each set of (backend, benchmark_id) has a different set of arguments, so please use
`--help` argument to check which flags are available. E.g., with

```bash
bazel-bin/flutter/cpp/binary/main EXTERNAL image_classification --help
```

we get

```shell
usage: bazel-bin/flutter/cpp/binary/main EXTERNAL image_classification <flags>
Flags:
  --mode=                                         string  required        Mode is one among PerformanceOnly, AccuracyOnly, SubmissionRun.
  --output_dir=                                   string  required        The output directory of mlperf.
  --model_file=                                   string  required        Path to model file.
  --images_directory=                             string  required        Path to ground truth images.
  --offset=1                                      int32   required        The offset of the first meaningful class in the classification model.
  --groundtruth_file=                             string  required        Path to the imagenet ground truth file.
  --min_query_count=100                           int32   optional        The test will guarantee to run at least this number of samples in performance mode.
  --min_duration_ms=100                           int32   optional        The test will guarantee to run at least this duration in performance mode. The duration is in ms.
  --max_duration_ms=600000                        int32   optional        The test will early exit when max duration is reached.The duration is in ms.
  --single_stream_expected_latency_ns=1000000     int32   optional        A hint used by the loadgen to pre-generate enough samples to meet the minimum test duration.
  --lib_path=                                     string  optional        Path to the backend library .so file.
  --native_lib_path=                              string  optional        Path to the additional .so files for the backend.
  --scenario=SingleStream                         string  optional        Scenario to run the benchmark.
  --batch_size=1                                  int32   optional        Batch size.
  --image_width=224                               int32   optional        The width of the processed image.
  --image_height=224                              int32   optional        The height of the processed image.
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

Evaluating top-1 accuracy of quantized MobileNet EdgeTPU tflite with ImageNet
validation set.

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

Evaluating accuracy of quantized MobileDet tflite with COCO 2017 validation set.

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

Evaluating accuracy of quantized DeepLabV3 tflite with ADE20K validation set.

```shell
adb shell LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/mlperf_main EXTERNAL ADE20K \
  --mode=AccuracyOnly \
  --output_dir=/data/local/tmp/ade20k_output \
  --model_file=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/deeplabv3_mnv2_ade20k_uint8.tflite \
  --images_directory=/sdcard/mlperf_datasets/ade20k/images \
  --ground_truth_directory=/sdcard/mlperf_datasets/ade20k/annotations \
  --lib_path=/data/local/tmp/libtflitebackend.so
```

Evaluating accuracy of floating point MobileBERT tflite with SQuAD V1.1 mini
evaluation set.

```shell
adb shell LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/mlperf_main EXTERNAL SQUAD \
  --mode=AccuracyOnly \
  --output_dir=/data/local/tmp/mobilebert_output \
  --model_file=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/mobilebert_float_384_gpu.tflite \
  --input_file=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/squad_eval_mini.tfrecord \
  --groundtruth_file=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/squad_groundtruth.tfrecord \
  --lib_path=/data/local/tmp/libtflitebackend.so
```

Evaluating accuracy of LLama3 with TinyMMLU evaluation set.

```shell
adb shell LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/mlperf_main EXTERNAL llm-1b \
--mode=AccuracyOnly \
--output_dir=/data/local/tmp/llm_output \
--zero-shot=false \
--model_file=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/llama_1b_q8_ekv1280.tflite \
--input_tfrecord=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/tinymmlu.tfrecord \
--lib_path=/data/local/tmp/libtflitebackend.so \
--sp_path=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/llama3_1b.spm.model 
```

Evaluating accuracy of LLama3 with IFEval evaluation set.

```shell
adb shell LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/mlperf_main EXTERNAL llm-1b-instruct \
--mode=AccuracyOnly \
--output_dir=/data/local/tmp/llm_output \
--model_file=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/llama_1b_q8_ekv1280.tflite \
--input_tfrecord=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/tinyifeval.tfrecord \
--lib_path=/data/local/tmp/libtflitebackend.so \
--sp_path=/sdcard/Android/data/org.mlperf.inference/files/cache/cache/llama3_1b.spm.model 
```

## Running LiteRT NPU and TFLite NPU on Google Tensor (Pixel)

### Building the CLI and Backend Libraries

Build the command-line binary `main` along with both `liblitertbackend.so` (bundled with the Google Tensor compiler plugin and dispatch `.so`) and `libtflitebackend.so`:

```shell
make WITH_LITERT=1 cmdline/android/bins/release
```

Push all generated binaries to the device:

```shell
adb push output/android-cli/* /data/local/tmp/
adb shell "chmod 755 /data/local/tmp/main /data/local/tmp/*.so"
```

Define the resources path (where models and datasets reside from the app or download):

```shell
RESOURCES_DIR=/sdcard/Android/data/org.mlcommons.android.mlperfbench/files/loaded_resources
```

---

### 1. Object Detection (`mobiledet_qat.tflite`)

* **LiteRT NPU (JIT)**:
```shell
adb shell "mkdir -p /data/local/tmp/coco_litert_npu && \
  LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/main EXTERNAL object_detection \
  --mode=PerformanceOnly \
  --output_dir=/data/local/tmp/coco_litert_npu \
  --model_file=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/models/v1_0/mobiledet_qat.tflite \
  --images_directory=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/datasets/v0_7/coco \
  --groundtruth_file=/data/local/tmp/dummy.pbtxt \
  --image_width=320 --image_height=320 --offset=1 --num_classes=91 \
  --lib_path=/data/local/tmp/liblitertbackend.so \
  --native_lib_path=/data/local/tmp \
  --min_query_count=64 --min_duration_ms=2500"
```

* **TFLite NPU (NNAPI)**:
```shell
adb shell "mkdir -p /data/local/tmp/coco_tflite_npu && \
  LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/main EXTERNAL object_detection \
  --mode=PerformanceOnly \
  --output_dir=/data/local/tmp/coco_tflite_npu \
  --model_file=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/models/v1_0/mobiledet_qat.tflite \
  --images_directory=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/datasets/v0_7/coco \
  --groundtruth_file=/data/local/tmp/dummy.pbtxt \
  --image_width=320 --image_height=320 --offset=1 --num_classes=91 \
  --lib_path=/data/local/tmp/libtflitebackend.so \
  --native_lib_path=/data/local/tmp \
  --min_query_count=64 --min_duration_ms=2500"
```

---

### 2. Natural Language Processing (`mobilebert_int8_384_nnapi.tflite`)

* **LiteRT NPU (JIT)**:
```shell
adb shell "mkdir -p /data/local/tmp/nlp_litert_npu && \
  LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/main EXTERNAL natural_language_processing \
  --mode=PerformanceOnly \
  --output_dir=/data/local/tmp/nlp_litert_npu \
  --model_file=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/models/v0_7/mobilebert_int8_384_nnapi.tflite \
  --input_file=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/datasets/v1_0/squad_eval_mini.tfrecord \
  --groundtruth_file=/data/local/tmp/squad_groundtruth.tfrecord \
  --lib_path=/data/local/tmp/liblitertbackend.so \
  --native_lib_path=/data/local/tmp \
  --min_query_count=64 --min_duration_ms=2500"
```

* **TFLite NPU (NNAPI)**:
```shell
adb shell "mkdir -p /data/local/tmp/nlp_tflite_npu && \
  TFLITE_DELEGATE=NNAPI LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/main EXTERNAL natural_language_processing \
  --mode=PerformanceOnly \
  --output_dir=/data/local/tmp/nlp_tflite_npu \
  --model_file=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/models/v0_7/mobilebert_int8_384_nnapi.tflite \
  --input_file=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/datasets/v1_0/squad_eval_mini.tfrecord \
  --groundtruth_file=/data/local/tmp/squad_groundtruth.tfrecord \
  --lib_path=/data/local/tmp/libtflitebackend.so \
  --native_lib_path=/data/local/tmp \
  --min_query_count=64 --min_duration_ms=2500"
```

---

### 3. Image Segmentation v2 (`mobile_segmenter_r4_quant_argmax_uint8.tflite`)

* **LiteRT NPU (JIT)**:
```shell
adb shell "mkdir -p /data/local/tmp/mosaic_litert_npu && \
  LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/main EXTERNAL image_segmentation_v2 \
  --mode=PerformanceOnly \
  --output_dir=/data/local/tmp/mosaic_litert_npu \
  --model_file=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/models/v2_0/mobile_segmenter_r4_quant_argmax_uint8.tflite \
  --images_directory=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/datasets/v0_7/ade20k \
  --ground_truth_directory=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/datasets/v0_7/ade20k \
  --num_class=31 --image_width=512 --image_height=512 \
  --lib_path=/data/local/tmp/liblitertbackend.so \
  --native_lib_path=/data/local/tmp \
  --min_query_count=64 --min_duration_ms=2500"
```

* **TFLite NPU (NNAPI)**:
```shell
adb shell "mkdir -p /data/local/tmp/mosaic_tflite_npu && \
  TFLITE_DELEGATE=NNAPI LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/main EXTERNAL image_segmentation_v2 \
  --mode=PerformanceOnly \
  --output_dir=/data/local/tmp/mosaic_tflite_npu \
  --model_file=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/models/v2_0/mobile_segmenter_r4_quant_argmax_uint8.tflite \
  --images_directory=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/datasets/v0_7/ade20k \
  --ground_truth_directory=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/datasets/v0_7/ade20k \
  --num_class=31 --image_width=512 --image_height=512 \
  --lib_path=/data/local/tmp/libtflitebackend.so \
  --native_lib_path=/data/local/tmp \
  --min_query_count=64 --min_duration_ms=2500"
```

---

### 4. Super Resolution (`edsr_f32b5_full_qint8.tflite`)

*Note: Pass an empty ground truth directory (e.g., `/data/local/tmp/empty_gt`) in PerformanceOnly mode to bypass PSNR ground truth dimension checks.*

* **LiteRT NPU (JIT)**:
```shell
adb shell "mkdir -p /data/local/tmp/empty_gt /data/local/tmp/edsr_litert_npu && \
  LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/main EXTERNAL super_resolution \
  --mode=PerformanceOnly \
  --output_dir=/data/local/tmp/edsr_litert_npu \
  --model_file=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/models/v3_0/edsr_f32b5_full_qint8.tflite \
  --images_directory=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/datasets/v3_1/snusr_lr_png \
  --ground_truth_directory=/data/local/tmp/empty_gt \
  --lib_path=/data/local/tmp/liblitertbackend.so \
  --native_lib_path=/data/local/tmp \
  --min_query_count=32 --min_duration_ms=2500"
```

* **TFLite NPU (NNAPI)**:
```shell
adb shell "mkdir -p /data/local/tmp/empty_gt /data/local/tmp/edsr_tflite_npu && \
  TFLITE_DELEGATE=NNAPI LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/main EXTERNAL super_resolution \
  --mode=PerformanceOnly \
  --output_dir=/data/local/tmp/edsr_tflite_npu \
  --model_file=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/models/v3_0/edsr_f32b5_full_qint8.tflite \
  --images_directory=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/datasets/v3_1/snusr_lr_png \
  --ground_truth_directory=/data/local/tmp/empty_gt \
  --lib_path=/data/local/tmp/libtflitebackend.so \
  --native_lib_path=/data/local/tmp \
  --min_query_count=32 --min_duration_ms=2500"
```

---

### 5. Image Classification v2 (`MobileNetV4-Conv-Large-int8-ptq.tflite` / `fp32`)

* **LiteRT GPU / CPU**:
```shell
adb shell "mkdir -p /data/local/tmp/imgcls_litert_gpu && \
  LITERT_DELEGATE=GPU LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/main EXTERNAL image_classification_v2 \
  --mode=PerformanceOnly \
  --output_dir=/data/local/tmp/imgcls_litert_gpu \
  --model_file=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/models/v4_0/tflite/MobileNetV4-Conv-Large-fp32.tflite \
  --images_directory=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/datasets/v0_7/imagenet \
  --groundtruth_file=/data/local/tmp/dummy.pbtxt \
  --image_width=384 --image_height=384 --offset=0 \
  --lib_path=/data/local/tmp/liblitertbackend.so \
  --native_lib_path=/data/local/tmp \
  --min_query_count=64 --min_duration_ms=2500"
```

* **TFLite NPU (NNAPI)**:
```shell
adb shell "mkdir -p /data/local/tmp/imgcls_tflite_npu && \
  TFLITE_DELEGATE=NNAPI LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/main EXTERNAL image_classification_v2 \
  --mode=PerformanceOnly \
  --output_dir=/data/local/tmp/imgcls_tflite_npu \
  --model_file=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/models/v4_0/tflite/MobileNetV4-Conv-Large-int8-ptq.tflite \
  --images_directory=${RESOURCES_DIR}/mobile.mlcommons-storage.org/app-resources/datasets/v0_7/imagenet \
  --groundtruth_file=/data/local/tmp/dummy.pbtxt \
  --image_width=384 --image_height=384 --offset=0 \
  --lib_path=/data/local/tmp/libtflitebackend.so \
  --native_lib_path=/data/local/tmp \
  --min_query_count=64 --min_duration_ms=2500"
```
