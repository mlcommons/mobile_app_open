# This Makefile is used for development purpose.
# Image files can be downloaded from here:
# https://github.com/mlcommons/mobile_models/raw/main/v0_7/datasets
# Groundtruth files can be downloaded from here:
# https://github.com/mlcommons/mobile_models/raw/main/v1_0/assets


REPO_ROOT_DIR="/Users/anh/dev/mlcommons/mobile_app_open"
#REPO_ROOT_DIR="/Users/freedom/work/mobile_app_open"

tflite: tflite-build tflite-run-ic-offline
coreml: coreml-build coreml-run-od
coreml-all: coreml-build coreml-run-ic coreml-run-ic-offline coreml-run-od coreml-run-is coreml-run-lu

backend=WITH_APPLE=1 WITH_TFLITE=0
app:
	cd ${REPO_ROOT_DIR} && make flutter/prepare ${backend} && make flutter/ios ${backend}
	cd ${REPO_ROOT_DIR}/flutter && flutter build ios
	open ${REPO_ROOT_DIR}/flutter/ios/Runner.xcworkspace


tflite-build:
	cd ${REPO_ROOT_DIR} && \
	bazel build -c opt --cxxopt=-std=c++14 --host_cxxopt=-std=c++14 //flutter/cpp/binary:main //mobile_back_tflite:tflitebackend

tflite-run-ic:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL IMAGENET \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobilenet_edgetpu/mobilenet_edgetpu_224_1.0_float.tflite" \
		--lib_path="bazel-bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/images" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/imagenet_val.txt" \
		--offset=1

tflite-run-ic-offline:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL IMAGENET \
		--mode=PerformanceOnly \
		--scenario=Offline \
		--batch_size=8 \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobilenet_edgetpu/mobilenet_edgetpu_224_1.0_float.tflite" \
		--lib_path="bazel-bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/images" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/imagenet_val.txt" \
		--offset=1

tflite-run-od:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL COCO \
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
	bazel-bin/flutter/cpp/binary/main EXTERNAL ADE20K \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mosaic/mobile_segmenter_r4_argmax_f32.tflite" \
		--lib_path="bazel-bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/ade20k/images" \
		--ground_truth_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/ade20k/annotations"

tflite-run-lu:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL SQUAD \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobilebert/mobilebert_float_384_gpu.tflite" \
		--lib_path="bazel-bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so" \
		--input_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/squad/squad_eval_mini.tfrecord" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/squad/squad_groundtruth.tfrecord"

coreml-build:
	cd ${REPO_ROOT_DIR} && \
	bazel build -c opt --cxxopt=-fobjc-arc --cxxopt=-xobjective-c++ --cxxopt=-std=c++14 --host_cxxopt=-std=c++14 \
	//flutter/cpp/binary:main //mobile_back_apple:coremlbackend

coreml-run-ic:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL IMAGENET \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobilenet_edgetpu/MobilenetEdgeTPU.mlmodel" \
		--lib_path="bazel-bin/mobile_back_apple/cpp/backend_coreml/libcoremlbackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/images" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/imagenet_val.txt" \
		--offset=1

# batch_size defined here must match with batch_size in coreml_settings.h
coreml-run-ic-offline:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL IMAGENET \
		--mode=PerformanceOnly \
		--scenario=Offline \
		--batch_size=32 \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobilenet_edgetpu/MobilenetEdgeTPU.mlmodel" \
		--lib_path="bazel-bin/mobile_back_apple/cpp/backend_coreml/libcoremlbackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/images" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet/imagenet_val.txt" \
		--offset=1

coreml-run-od:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL COCO \
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
	bazel-bin/flutter/cpp/binary/main EXTERNAL ADE20K \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mosaic/Mosaic.mlmodel" \
		--lib_path="bazel-bin/mobile_back_apple/cpp/backend_coreml/libcoremlbackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/ade20k/images" \
		--ground_truth_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/ade20k/annotations"

coreml-run-lu:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL SQUAD \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobilebert/MobileBERT.mlmodel" \
		--lib_path="bazel-bin/mobile_back_apple/cpp/backend_coreml/libcoremlbackend.so" \
		--input_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/squad/squad_eval_mini.tfrecord" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/squad/squad_groundtruth.tfrecord"