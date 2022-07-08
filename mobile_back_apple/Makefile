# This Makefile is used for development purpose.
# It will be removed after finishing integrating Core ML backend.


REPO_ROOT_DIR="/Users/anh/dev/mlcommons/mobile_app_open"
#REPO_ROOT_DIR="/Users/freedom/work/mobile_app_open"

tflite: tflite-build tflite-run
coreml: coreml-build coreml-run


tflite-build:
	cd ${REPO_ROOT_DIR} && \
	bazel build -c opt --cxxopt=-std=c++14 --host_cxxopt=-std=c++14 //flutter/cpp/binary:main //mobile_back_tflite:tflitebackend

tflite-run:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL IMAGENET \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/mobilenet_edgetpu_224_1.0_float.tflite" \
		--lib_path="bazel-bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet_val.txt" \
		--offset=1


coreml-build:
	cd ${REPO_ROOT_DIR} && \
	bazel build -c opt --cxxopt=-std=c++14 --host_cxxopt=-std=c++14 //flutter/cpp/binary:main //mobile_back_apple:coremlbackend \
        --cxxopt=-xobjective-c++

coreml-run:
	cd ${REPO_ROOT_DIR} && \
	bazel-bin/flutter/cpp/binary/main EXTERNAL IMAGENET \
		--mode=PerformanceOnly \
		--output_dir="${REPO_ROOT_DIR}/output" \
		--model_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/MobilenetEdgeTPU.mlmodel" \
		--lib_path="bazel-bin/mobile_back_apple/cpp/backend_coreml/libcoremlbackend.so" \
		--images_directory="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet" \
		--groundtruth_file="${REPO_ROOT_DIR}/mobile_back_apple/dev-resources/imagenet_val.txt" \
		--offset=1
