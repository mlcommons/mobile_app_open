# This Makefile is used for development purpose.
# Image files can be downloaded from here:
# https://github.com/mlcommons/mobile_models/raw/main/v0_7/datasets
# Groundtruth files can be downloaded from here:
# https://github.com/mlcommons/mobile_models/raw/main/v1_0/assets


REPO_ROOT_DIR="/Users/anh/dev/mlcommons/mobile_app_open"
#REPO_ROOT_DIR="/Users/freedom/work/mobile_app_open"

export ANDROID_NDK_VERSION=25
export ANDROID_NDK_API_LEVEL=33

tflite: tflite-build tflite-run-sd
coreml: coreml-build coreml-run-ic-v2 coreml-run-ic-offline-v2
tflite-all: tflite-build tflite-run-ic tflite-run-ic-offline tflite-run-od tflite-run-is tflite-run-lu tflite-run-sr tflite-run-ic-v2 tflite-run-ic-offline-v2
coreml-all: coreml-build coreml-run-ic coreml-run-ic-offline coreml-run-od coreml-run-is coreml-run-lu coreml-run-sr coreml-run-ic-v2 coreml-run-ic-offline-v2

backend=WITH_APPLE=1 WITH_TFLITE=0
app:
	cd ${REPO_ROOT_DIR} && make flutter/prepare ${backend} && make flutter/ios ${backend}
	cd ${REPO_ROOT_DIR}/flutter && flutter build ios
	open ${REPO_ROOT_DIR}/flutter/ios/Runner.xcworkspace

tflite-build:
	cd ${REPO_ROOT_DIR} && \
	bazel build -c opt --cxxopt=-std=c++17 --host_cxxopt=-std=c++17 //flutter/cpp/binary:main //mobile_back_tflite:tflitebackend

tflite-run-ic:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL image_classification \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobilenet_edgetpu/mobilenet_edgetpu_224_1.0_float.tflite" \
		--lib_path="bazel-bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/images" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/imagenet_val.txt" \
		--offset=1 \
		--max_duration_ms=600000 \
		--min_duration_ms=100

tflite-run-ic-offline:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL image_classification_offline \
		--mode=PerformanceOnly \
		--scenario=Offline \
		--batch_size=32 \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobilenet_edgetpu/mobilenet_edgetpu_224_1.0_float.tflite" \
		--lib_path="bazel-bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/images" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/imagenet_val.txt" \
		--offset=1 \
		--max_duration_ms=600000 \
		--min_duration_ms=100

tflite-run-ic-v2:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL image_classification_v2 \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobilenet_v4/MobileNetV4-Conv-Large-fp32.tflite" \
		--image_width=384 \
		--image_height=384 \
		--lib_path="bazel-bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/images" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/imagenet_val.txt" \
		--offset=0

tflite-run-ic-offline-v2:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL image_classification_offline_v2 \
		--mode=PerformanceOnly \
		--scenario=Offline \
		--batch_size=32 \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobilenet_v4/MobileNetV4-Conv-Large-fp32.tflite" \
		--image_width=384 \
		--image_height=384 \
		--lib_path="bazel-bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/images" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/imagenet_val.txt" \
		--offset=0

tflite-run-od:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL object_detection \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobiledet/mobiledet.tflite" \
		--lib_path="bazel-bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/coco/images" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/coco/coco_val.pbtxt" \
		--offset=1 \
		--num_classes=91

tflite-run-is:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL image_segmentation_v2 \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mosaic/mobile_segmenter_r4_argmax_f32.tflite" \
		--lib_path="bazel-bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/ade20k/images" \
		--ground_truth_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/ade20k/annotations"

tflite-run-lu:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL natural_language_processing \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobilebert/mobilebert_float_384_gpu.tflite" \
		--lib_path="bazel-bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so" \
		--input_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/squad/squad_eval_mini.tfrecord" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/squad/squad_groundtruth.tfrecord"

tflite-run-sr:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL super_resolution \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/edsr_final/converted/edsr_f32b5_fp32.tflite" \
		--lib_path="bazel-bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/psnr/LR" \
		--ground_truth_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/psnr/HR" \
		--max_duration_ms=60000 \
		--min_duration_ms=100

tflite-run-sd:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL stable_diffusion \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/stable_diffusion/sd-models" \
		--lib_path="bazel-bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so" \
		--custom_config="stable_diffusion_num_steps:20,stable_diffusion_seed:633994880" \
		--input_tfrecord="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/stable_diffusion/coco_gen_full.tfrecord" \
		--input_clip_model="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/stable_diffusion/clip_model_512x512.tflite" \
		--min_query_count=5

coreml-build:
	cd ${REPO_ROOT_DIR} && \
	bazel build -c opt --cxxopt=-fobjc-arc --cxxopt=-xobjective-c++ --cxxopt=-std=c++17 --host_cxxopt=-std=c++17 \
	//flutter/cpp/binary:main //mobile_back_apple:coremlbackend

coreml-run-ic:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL image_classification \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobilenet_edgetpu/MobilenetEdgeTPU.mlmodel" \
		--lib_path="bazel-bin/mobile_back_apple/cpp/backend_coreml/libcoremlbackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/images" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/imagenet_val.txt" \
		--offset=1

coreml-run-ic-offline:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL image_classification_offline \
		--mode=PerformanceOnly \
		--scenario=Offline \
		--batch_size=32 \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobilenet_edgetpu/MobilenetEdgeTPU.mlmodel" \
		--lib_path="bazel-bin/mobile_back_apple/cpp/backend_coreml/libcoremlbackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/images" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/imagenet_val.txt" \
		--offset=1

coreml-run-ic-v2:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL image_classification_v2 \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobilenet_v4/mobilenetv4_fp32_NCHW.mlpackage" \
		--image_width=384 \
		--image_height=384 \
		--lib_path="bazel-bin/mobile_back_apple/cpp/backend_coreml/libcoremlbackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/images" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/imagenet_val.txt" \
		--offset=0

coreml-run-ic-offline-v2:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL image_classification_offline_v2 \
		--mode=PerformanceOnly \
		--scenario=Offline \
		--batch_size=32 \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobilenet_v4/MobilenetV4_Large.mlmodel" \
		--image_width=384 \
		--image_height=384 \
		--lib_path="bazel-bin/mobile_back_apple/cpp/backend_coreml/libcoremlbackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/images" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/imagenet_val.txt" \
		--offset=0

coreml-run-od:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL object_detection \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobiledet/MobileDet.mlmodel" \
		--lib_path="bazel-bin/mobile_back_apple/cpp/backend_coreml/libcoremlbackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/coco/images" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/coco/coco_val.pbtxt" \
		--offset=1 \
		--num_classes=91

coreml-run-is:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL image_segmentation_v2 \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mosaic/Mosaic.mlmodel" \
		--lib_path="bazel-bin/mobile_back_apple/cpp/backend_coreml/libcoremlbackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/ade20k/images" \
		--ground_truth_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/ade20k/annotations"

coreml-run-lu:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL natural_language_processing \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobilebert/MobileBERT.mlmodel" \
		--lib_path="bazel-bin/mobile_back_apple/cpp/backend_coreml/libcoremlbackend.so" \
		--input_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/squad/squad_eval_mini.tfrecord" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/squad/squad_groundtruth.tfrecord"

coreml-run-sr:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL super_resolution \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/edsr_final/converted/edsr_f32b5_fp32.mlmodel" \
		--lib_path="bazel-bin/mobile_back_apple/cpp/backend_coreml/libcoremlbackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/psnr/LR_jpg" \
		--ground_truth_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/psnr/HR_jpg"
