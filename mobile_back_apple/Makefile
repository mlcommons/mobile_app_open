# This Makefile is used for development purpose.
# It will be removed after finishing integrating Core ML backend.
# Image files can be downloaded from here:
# https://github.com/mlcommons/mobile_models/raw/main/v0_7/datasets
# Groundtruth files can be downloaded from here:
# https://github.com/mlcommons/mobile_models/raw/main/v1_0/assets


REPO_ROOT_DIR="/Users/anh/dev/mlcommons/mobile_app_open"
#REPO_ROOT_DIR="/Users/freedom/work/mobile_app_open"

tflite: tflite-build tflite-run-od
coreml: coreml-build coreml-run-od

app:
	cd ${REPO_ROOT_DIR} && make flutter/prepare && make flutter/ios
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
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobilenet_edgetpu_224_1.0_float.tflite" \
		--lib_path="bazel-bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet_val.txt" \
		--offset=1

tflite-run-od:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL COCO \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobiledet.tflite" \
		--lib_path="bazel-bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/coco" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/coco_val.pbtxt" \
		--offset=1 \
		--num_classes=91

coreml-build:
	cd ${REPO_ROOT_DIR} && \
	bazel build -c opt --cxxopt=-std=c++14 --host_cxxopt=-std=c++14 //flutter/cpp/binary:main //mobile_back_apple:coremlbackend \
        --cxxopt=-xobjective-c++

coreml-run-ic:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL IMAGENET \
		--mode=SubmissionRun \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/MobilenetEdgeTPU_multi_array_with_shape.mlmodel" \
		--lib_path="bazel-bin/mobile_back_apple/cpp/backend_coreml/libcoremlbackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet_val.txt" \
		--offset=1

coreml-run-od:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL COCO \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/dummy_od.mlmodel" \
		--lib_path="bazel-bin/mobile_back_apple/cpp/backend_coreml/libcoremlbackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/coco" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/coco_val.pbtxt" \
		--offset=1 \
		--num_classes=91