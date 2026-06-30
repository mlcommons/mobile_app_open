# Copyright (c) 2020-2025 Qualcomm Innovation Center, Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

# This file must be called from top level Makefile

include ../make/builddir.mk

DATASETS:=${TOPDIR}/datasets
DATASETS_OUT:=${DATASETS}/output
CALIBRATION_DATA:=${DATASETS_OUT}/calibration_set
MLPERF_DATASETS:=${DATASETS_OUT}/mlperf_datasets

all: ${DATASETS_OUT}/state/calibration.stamp htp-dlc

# Download and build datasets and calibration files
${DATASETS_OUT}/state/calibration.stamp:
	@(cd ${TOPDIR}/datasets && make)

.PHONY: htp-quantized-dlc htp-dlc docker/htp-dlc \
	docker/mobilenet_v4 docker/mobilenet_v4_O2 \
	docker/mobiledet docker/mobiledet_O2 \
	docker/mosaic docker/mosaic_O2 \
	docker/mobilebert docker/mobilebert_O2 \
	docker/snusr docker/snusr_O2 \
	docker/mobilenet_v4_batched docker/mobilenet_v4_batched_O2 \
	mobilenet_v4 mobilenet_v4_O2 \
	mobiledet mobiledet_O2 \
	mosaic mosaic_O2 \
	mobilebert mobilebert_O2 \
	snusr snusr_O2 \
	mobilenet_v4_batched nodock/mobilenet_v4_batched_O2 \
	dependencies gen-hta-dlc-info gen-htp-dlc-info clean

DLCBUILDDIR=${BUILDDIR}/DLC
MODEL_BASE_PATH=${DLCBUILDDIR}/mobile
MOBILENETV4_MODEL_PATH=${MODEL_BASE_PATH}/vision/mobilenetV4/MobileNetV4-Conv-Large-saved-model
MOBILEBERT_MODEL_PATH=${MODEL_BASE_PATH}/language/bert/models_and_code/checkpoints/quant/
MOBILEMOSAIC_MODEL_PATH=${MODEL_BASE_PATH}/vision/mosaic/models_and_checkpoints/R4/
SNUSR_MODEL_PATH = ${MODEL_BASE_PATH}/vision/edsr/models_and_checkpoints/checkpoints/f32b5/ckpt_qat/
TEXTENCODER_MODEL_PATH = ${DLCBUILDDIR}/stable_diffusion/text_encoder/
VAEDECODER_MODEL_PATH = ${DLCBUILDDIR}/stable_diffusion/vae_decoder/
UNET_MODEL_PATH = ${DLCBUILDDIR}/stable_diffusion/unet/
SNUSR_CALIBRATION_PATH=${MODEL_BASE_PATH}/calibration/OpenImages
MLPERF_MODELS_PATH = ${DLCBUILDDIR}/mlperf_models/
MOBILEDETSSDQAT_MODEL_PATH = ${DLCBUILDDIR}/mobile/vision/mobiledet/uint8/export_inference_graph/

##########################################################################
# USE_DOCKER controls whether recipes wrap commands in "docker run" or
# execute them directly (for use inside an external docker container).
#
#   make docker/htp-dlc   -> USE_DOCKER=1  (default, runs via docker)
#   make htp-dlc          -> USE_DOCKER=0  (run bare, called from inside docker)
##########################################################################
##########################################################################
# USE_DOCKER=0 by default so that "make mobilebert SNPE_SDK=..." works
# directly on the host (e.g. inside mlg-docker / snpe_venv).
# Set USE_DOCKER=1 (or use the docker/<model> shortcuts) to wrap every
# command in "docker run mlperf_dlc_prepare ...".
##########################################################################
USE_DOCKER ?= 0

ifeq ($(USE_DOCKER),1)
INTERNAL_DOCKER?=0
ifeq ($(INTERNAL_DOCKER),1)
$(info "Using internal docker")
include load_internal_docker.mk
else
$(info "Using normal docker")
include load_normal_docker.mk
endif
endif

##########################################################################
# Normalize SNPE_SDK to an absolute path so Docker -v mounts work even
# when the caller passes a relative path (e.g. SNPE_SDK=../qairt/2.43.0.260128/)
##########################################################################
SNPE_SDK := $(abspath $(SNPE_SDK))

##########################################################################
# Docker run prefixes
# When USE_DOCKER=0 these expand to empty string so the tool is called
# directly.  When USE_DOCKER=1 they expand to the full "docker run ..." 
# invocation up to (but not including) the actual tool binary.
##########################################################################
ifeq ($(USE_DOCKER),1)

# Prefix for SNPE-based targets (snpe-tensorflow-to-dlc, snpe-dlc-quant,
# snpe-dlc-graph-prepare, snpe-dlc-info)
define DOCKER_RUN_SNPE
docker run \
	-e PYTHONPATH=/snpe_sdk/lib/python \
	-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
	-v ${SNPE_SDK}:/snpe_sdk \
	-v ${DLCBUILDDIR}:/output \
	-v ${MODEL_BASE_PATH}:/models \
	-v ${MOBILENETV4_MODEL_PATH}:/mobilenet_v4_model \
	-v ${MOBILEDETSSDQAT_MODEL_PATH}:/mobiledet_model \
	-v ${MOBILEMOSAIC_MODEL_PATH}:/mobile_mosaic \
	-v ${MOBILEBERT_MODEL_PATH}:/mobilebert_models \
	-v ${SNUSR_MODEL_PATH}:/snusr_models \
	-w /output \
	-u ${USERID}:${GROUPID} \
	mlperf_dlc_prepare
endef

define DOCKER_RUN_SNPE_IMAGENET_RESIZE 
docker run \
	-e PYTHONPATH=/snpe_sdk/lib/python \
	-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
	-v ${SNPE_SDK}:/snpe_sdk \
	-v ${DLCBUILDDIR}:/output \
	-v ${MODEL_BASE_PATH}:/models \
	-v ${TOPDIR}/datasets/util:/util \
	-v ${CALIBRATION_DATA}/imagenet:/imagenet \
	-u ${USERID}:${GROUPID} \
	mlperf_dlc_prepare
endef

# Prefix for SNPE graph-prepare targets that also mount imagenet
define DOCKER_RUN_SNPE_IMAGENET
docker run \
	-e PYTHONPATH=/snpe_sdk/lib/python \
	-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
	-v ${SNPE_SDK}:/snpe_sdk \
	-v ${DLCBUILDDIR}:/output \
	-v ${DLCBUILDDIR}/imagenet:/imagenet \
	-w /imagenet \
	-u ${USERID}:${GROUPID} \
	mlperf_dlc_prepare
endef

# Prefix for SNPE graph-prepare targets that also mount coco
define DOCKER_RUN_SNPE_COCO
docker run \
	-e PYTHONPATH=/snpe_sdk/lib/python \
	-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
	-v ${SNPE_SDK}:/snpe_sdk \
	-v ${DLCBUILDDIR}:/output \
	-v ${DLCBUILDDIR}/coco:/coco-out \
	-w /coco-out \
	-u ${USERID}:${GROUPID} \
	mlperf_dlc_prepare
endef

# Prefix for SNPE graph-prepare targets that also mount ade20k
define DOCKER_RUN_SNPE_ADE20K
docker run \
	-e PYTHONPATH=/snpe_sdk/lib/python \
	-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
	-v ${SNPE_SDK}:/snpe_sdk \
	-v ${DLCBUILDDIR}:/output \
	-v ${DLCBUILDDIR}/ade20k:/ade20k \
	-w /ade20k \
	-u ${USERID}:${GROUPID} \
	mlperf_dlc_prepare
endef

# Prefix for SNPE graph-prepare targets that also mount snu_sr
define DOCKER_RUN_SNPE_SNUSR
docker run \
	-e PYTHONPATH=/snpe_sdk/lib/python \
	-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
	-v ${SNPE_SDK}:/snpe_sdk \
	-v ${DLCBUILDDIR}:/output \
	-v ${DLCBUILDDIR}/snusr/:/snusr \
	-w /snusr \
	-u ${USERID}:${GROUPID} \
	mlperf_dlc_prepare
endef

# Prefix for SNPE graph-prepare targets that also mount snu_sr
define DOCKER_RUN_SNPE_SQUAD
docker run \
	-e PYTHONPATH=/snpe_sdk/lib/python \
	-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
	-v ${SNPE_SDK}:/snpe_sdk \
	-v ${DLCBUILDDIR}:/output \
	-v ${CALIBRATION_DATA}/squad:/squad-out \
	-w /squad-out \
	-u ${USERID}:${GROUPID} \
	mlperf_dlc_prepare
endef

# Prefix for QNN-based targets (qnn-onnx-converter, qnn-model-lib-generator,
# qnn-context-binary-generator)
# The mcp_config_v*.json files (generated by AIMET copy_configs_to_ouptut) live in
# per-model subdirs of stable_diffusion/ and internally reference
# /models/graph_config_v*.json.  We therefore provide three per-model variants
# that each mount the appropriate subdirectory as /models.
# DOCKER_RUN_QNN is the base variant (no /models mount) used for conversion and
# lib-generation steps that do not need the graph config.
define DOCKER_RUN_QNN
docker run \
	-v ${SNPE_SDK}:/qnn_sdk \
	-e PYTHONPATH=/qnn_sdk/lib/python \
	-e LD_LIBRARY_PATH=/usr/local/clang-9.0.0/lib:qnn_sdk/lib/x86_64-linux-clang:${LD_LIBRARY_PATH} \
	-v ${DLCBUILDDIR}:/output \
	-v ${TOPDIR}/mobile_back_qti/DLC/util/StableDiffusion:/util \
	-v ${DLCBUILDDIR}/stable_diffusion/:/stable_diffusion \
	-w /stable_diffusion \
	-u ${USERID}:${GROUPID} \
	mlperf_dlc_prepare
endef

# Variant for text_encoder versioned context-binary-generator targets:
# mounts output/DLC/stable_diffusion/text_encoder as /models so that
# graph_config_v*.json (referenced inside mcp_config_v*.json) is found.
define DOCKER_RUN_QNN_TEXT_ENCODER
docker run \
	-v ${SNPE_SDK}:/qnn_sdk \
	-e PYTHONPATH=/qnn_sdk/lib/python \
	-e LD_LIBRARY_PATH=/usr/local/clang-9.0.0/lib:qnn_sdk/lib/x86_64-linux-clang:${LD_LIBRARY_PATH} \
	-v ${DLCBUILDDIR}:/output \
	-v ${DLCBUILDDIR}/stable_diffusion/text_encoder:/models \
	-v ${TOPDIR}/mobile_back_qti/DLC/util/StableDiffusion:/util \
	-v ${DLCBUILDDIR}/stable_diffusion/:/stable_diffusion \
	-w /stable_diffusion \
	-u ${USERID}:${GROUPID} \
	mlperf_dlc_prepare
endef

# Variant for vae_decoder versioned context-binary-generator targets.
define DOCKER_RUN_QNN_VAE_DECODER
docker run \
	-v ${SNPE_SDK}:/qnn_sdk \
	-e PYTHONPATH=/qnn_sdk/lib/python \
	-e LD_LIBRARY_PATH=/usr/local/clang-9.0.0/lib:qnn_sdk/lib/x86_64-linux-clang:${LD_LIBRARY_PATH} \
	-v ${DLCBUILDDIR}:/output \
	-v ${DLCBUILDDIR}/stable_diffusion/vae_decoder:/models \
	-v ${TOPDIR}/mobile_back_qti/DLC/util/StableDiffusion:/util \
	-v ${DLCBUILDDIR}/stable_diffusion/:/stable_diffusion \
	-w /stable_diffusion \
	-u ${USERID}:${GROUPID} \
	mlperf_dlc_prepare
endef

# Variant for unet versioned context-binary-generator targets.
define DOCKER_RUN_QNN_UNET
docker run \
	-v ${SNPE_SDK}:/qnn_sdk \
	-e PYTHONPATH=/qnn_sdk/lib/python \
	-e LD_LIBRARY_PATH=/usr/local/clang-9.0.0/lib:qnn_sdk/lib/x86_64-linux-clang:${LD_LIBRARY_PATH} \
	-v ${DLCBUILDDIR}:/output \
	-v ${DLCBUILDDIR}/stable_diffusion/unet:/models \
	-v ${TOPDIR}/mobile_back_qti/DLC/util/StableDiffusion:/util \
	-v ${DLCBUILDDIR}/stable_diffusion/:/stable_diffusion \
	-w /stable_diffusion \
	-u ${USERID}:${GROUPID} \
	mlperf_dlc_prepare
endef

# Prefix for mlperf_mobile:1.1 utility targets (resize, jpg_to_raw, etc.)
define DOCKER_RUN_MOBILE
docker run \
	-u ${USERID}:${GROUPID} \
	mlperf_mobile:1.1
endef

define DOCKER_RUN_MOBILE_QUANTDATA_384
docker run \
	-v ${TOPDIR}/datasets/util:/util \
	-v ${DLCBUILDDIR}/imagenet:/imagenet \
	-u ${USERID}:${GROUPID} \
	mlperf_mobile:1.1
endef

define DOCKER_RUN_MOBILE_RESIZE
docker run \
	-v ${CALIBRATION_DATA}/coco:/coco \
	-v ${DLCBUILDDIR}/coco/resized:/resized \
	-v ${DLCBUILDDIR}/coco:/coco_out \
	-v ${TOPDIR}/datasets/util:/util \
	-u ${USERID}:${GROUPID} \
	mlperf_mobile:1.1
endef

define DOCKER_RUN_MOBILE_RESIZE_ADE20K
docker run \
	-e PYTHONPATH=/input/models-2.3.0/research/slim:/input/models-2.3.0/research/deeplab:/input/models-2.3.0/research \
	-v $(CURDIR)/util/ade20k:/util/ \
	-v ${TOPDIR}/datasets/util/ade20k:/util2 \
	-v ${DATASETS_OUT}/ade20k:/input/ \
	-v $(DLCBUILDDIR)/ade20k:/output/ \
	-v ${TOPDIR}/datasets/util/common:/common_util \
	-v ${DLCBUILDDIR}/ade20k:/ade20k \
	-u ${USERID}:${GROUPID} \
	mlperf_mobile:1.1
endef

define DOCKER_RUN_MOBILE_RESIZE_SNUSR
docker run \
	-v ${CALIBRATION_DATA}/coco:/coco \
	-v ${DLCBUILDDIR}/coco/resized:/resized \
	-v ${DLCBUILDDIR}/coco:/coco_out \
	-v ${TOPDIR}/mobile_back_qti/DLC/util/:/util \
	-v ${DATASETS}/output/snusr/:/snusr \
	-v ${SNUSR_CALIBRATION_PATH}:/calibration \
	-u ${USERID}:${GROUPID} \
	mlperf_mobile:1.1
endef

# Docker stamp dependencies (only needed when running via docker)
DOCKER_STAMP = ${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp
DOCKER_MOBILE_STAMP = ${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp
# Path variables for Docker mode (container-internal paths)
SNPE_BIN             := /snpe_sdk/bin/x86_64-linux-clang
SNPE_OUT             := /output
SNPE_MOBILENETV4_DIR := /mobilenet_v4_model
SNPE_MOBILEDET_DIR   := /mobiledet_model
SNPE_MOSAIC_DIR      := /mobile_mosaic
SNPE_MOBILEBERT_DIR  := /mobilebert_models
SNPE_SNUSR_DIR       := /snusr_models
UTIL_DIR             := /util
IMAGENET_CALIB_DIR   := /imagenet
IMAGENET_BUILD_DIR   := /imagenet
COCO_BUILD_DIR       := /coco-out
ADE20K_BUILD_DIR     := /ade20k
SNUSR_BUILD_DIR      := /snusr
SQUAD_BUILD_DIR      := /squad-out
# cd prefixes for Docker mode: empty because -w already sets the working dir
SNPE_IMAGENET_CD     :=
SNPE_COCO_CD         :=
SNPE_ADE20K_CD       :=
SNPE_SNUSR_CD        :=
SNPE_SQUAD_CD        :=
# Path variables for MOBILE_RESIZE/MOBILE_RESIZE_ADE20K/MOBILE_RESIZE_SNUSR
# (Docker mode — container-internal paths matching the -v mounts above)
COCO_UTIL_DIR          := /util/coco
COCO_IMAGES_DIR        := /coco/images
COCO_RESIZED_DIR       := /resized
COCO_OUT_DIR           := /coco_out
COMMON_UTIL_DIR        := /util/common
ADE20K_UTIL_DIR        := /util
ADE20K_UTIL2_DIR       := /util2
ADE20K_INPUT_DIR       := /input
ADE20K_OUT_DIR         := /output
ADE20K_COMMON_UTIL_DIR := /common_util
SNUSR_UTIL_DIR         := /util/snusr
SNUSR_RAW_DATA_DIR     := /snusr
SNUSR_CALIB_DIR        := /calibration
# Docker mode: call upscale_coco.py directly (its hardcoded /util/coco/... path is valid inside the container)
COCO_RESIZE_CMD        = python3 $(COCO_UTIL_DIR)/upscale_coco.py --inputs $(COCO_IMAGES_DIR) --outputs $(COCO_RESIZED_DIR) --size 320 320
# Docker sets PYTHONPATH for deeplab via -e in DOCKER_RUN_MOBILE_RESIZE_ADE20K
ADE20K_PYTHONPATH      :=
# Docker mode: use the original deeplab-based script (TF1 is available in the image)
ADE20K_RESIZE_CMD      = $(ADE20K_PYTHONPATH) python3 $(ADE20K_UTIL_DIR)/resize_512_ade20k_calibration.py
# Docker mode: image list uses absolute container-internal paths
ADE20K_QUANTDATA_PREFIX := /ade20k/quantdata/
# Docker mode: freeze_graph.py is at a fixed path inside the container
FREEZE_GRAPH_CMD       := python3 /usr/local/lib/python3.10/dist-packages/tensorflow/python/tools/freeze_graph.py

else
# USE_DOCKER=0: running bare on the host (e.g. inside mlg-docker or a venv).
# No docker wrapping, no docker image build stamps needed.
DOCKER_RUN_SNPE          :=
DOCKER_RUN_SNPE_IMAGENET :=
DOCKER_RUN_QNN           :=
DOCKER_RUN_MOBILE        :=
DOCKER_STAMP             :=
DOCKER_MOBILE_STAMP      :=
# Path variables for native (non-Docker) execution — use real host paths
SNPE_BIN             := ${SNPE_SDK}/bin/x86_64-linux-clang
SNPE_OUT             := ${DLCBUILDDIR}
SNPE_MOBILENETV4_DIR := ${MOBILENETV4_MODEL_PATH}
SNPE_MOBILEDET_DIR   := ${MOBILEDETSSDQAT_MODEL_PATH}
SNPE_MOSAIC_DIR      := ${MOBILEMOSAIC_MODEL_PATH}
SNPE_MOBILEBERT_DIR  := ${MOBILEBERT_MODEL_PATH}
SNPE_SNUSR_DIR       := ${SNUSR_MODEL_PATH}
UTIL_DIR             := ${TOPDIR}/datasets/util
IMAGENET_CALIB_DIR   := ${CALIBRATION_DATA}/imagenet
IMAGENET_BUILD_DIR   := ${DLCBUILDDIR}/imagenet
COCO_BUILD_DIR       := ${DLCBUILDDIR}/coco
ADE20K_BUILD_DIR     := ${DLCBUILDDIR}/ade20k
SNUSR_BUILD_DIR      := ${DLCBUILDDIR}/snusr
SQUAD_BUILD_DIR      := ${CALIBRATION_DATA}/squad
# cd prefixes for non-Docker mode: change to the dataset dir so relative
# paths inside the input list files resolve correctly
SNPE_IMAGENET_CD     := cd ${DLCBUILDDIR}/imagenet &&
SNPE_COCO_CD         := cd ${DLCBUILDDIR}/coco &&
SNPE_ADE20K_CD       := cd ${DLCBUILDDIR}/ade20k &&
SNPE_SNUSR_CD        := cd ${DLCBUILDDIR}/snusr &&
SNPE_SQUAD_CD        := cd ${CALIBRATION_DATA}/squad &&
# Non-Docker mode: use the module invocation so it works regardless of TF install path
FREEZE_GRAPH_CMD     := python3 -m tensorflow.python.tools.freeze_graph
# Path variables for MOBILE_RESIZE/MOBILE_RESIZE_ADE20K/MOBILE_RESIZE_SNUSR
# (non-Docker mode — real host paths matching the Docker -v mount sources)
COCO_UTIL_DIR          := ${TOPDIR}/datasets/util/coco
COCO_IMAGES_DIR        := ${CALIBRATION_DATA}/coco/images
COCO_RESIZED_DIR       := ${DLCBUILDDIR}/coco/resized
COCO_OUT_DIR           := ${DLCBUILDDIR}/coco
COMMON_UTIL_DIR        := ${TOPDIR}/datasets/util/common
ADE20K_UTIL_DIR        := $(CURDIR)/util/ade20k
ADE20K_UTIL2_DIR       := ${TOPDIR}/datasets/util/ade20k
ADE20K_INPUT_DIR       := ${DATASETS_OUT}/ade20k
ADE20K_OUT_DIR         := ${DLCBUILDDIR}/ade20k
ADE20K_COMMON_UTIL_DIR := ${TOPDIR}/datasets/util/common
SNUSR_UTIL_DIR         := ${TOPDIR}/mobile_back_qti/DLC/util/snusr
SNUSR_RAW_DATA_DIR     := ${DATASETS}/output/snusr
SNUSR_CALIB_DIR        := ${SNUSR_CALIBRATION_PATH}
# Non-Docker: inline Python one-liner replaces upscale_coco.py entirely so that
# script is left unmodified.  Uses cv2 (same as the script) and reads the
# calibration file list from the real host path.
COCO_RESIZE_CMD        = python3 -c "import os,cv2; [cv2.imwrite(os.path.join('$(COCO_RESIZED_DIR)',f.split('.')[0]+'.jpg'), cv2.resize(i if len(i.shape)==3 and i.shape[2]==3 else cv2.cvtColor(i,cv2.COLOR_GRAY2RGB),(320,320),interpolation=cv2.INTER_LINEAR)) for f in open('$(TOPDIR)/datasets/util/coco/coco_calibration_files.txt').read().split('\n') if f for i in [cv2.imread(os.path.join('$(COCO_IMAGES_DIR)',f))]]"
# Non-Docker mode: PIL one-liner replaces the TF1/deeplab-based resize script
ADE20K_RESIZE_CMD      = python3 -c "import os,sys; from PIL import Image as _I; [_I.open(os.path.join(dp,fn)).convert('RGB').resize((512,512),_I.LANCZOS).save(os.path.join(sys.argv[2],fn)) for fn in open(sys.argv[3]).read().split('\n') if fn for dp,_x,fns in os.walk(sys.argv[1]) if fn in fns]"
# Non-Docker mode: snpe-dlc-quant runs with cd ${DLCBUILDDIR}/ade20k, so paths must be relative
ADE20K_QUANTDATA_PREFIX := quantdata/
export PYTHONPATH    := ${SNPE_SDK}/lib/python
export LD_LIBRARY_PATH := ${SNPE_SDK}/lib/x86_64-linux-clang
endif

# Convenience: "make docker/htp-dlc" forces USE_DOCKER=1 and builds htp-dlc
docker/htp-dlc:
	$(MAKE) htp-dlc USE_DOCKER=1

# Per-model docker shortcuts: "make docker/<model>" forces USE_DOCKER=1
# Use these when running from outside a container (docker wraps each command).
docker/mobilenet_v4:
	$(MAKE) mobilenet_v4 USE_DOCKER=1

docker/mobilenet_v4_O2:
	$(MAKE) mobilenet_v4_O2 USE_DOCKER=1

docker/mobiledet:
	$(MAKE) mobiledet USE_DOCKER=1

docker/mobiledet_O2:
	$(MAKE) mobiledet_O2 USE_DOCKER=1

docker/mosaic:
	$(MAKE) mosaic USE_DOCKER=1

docker/mosaic_O2:
	$(MAKE) mosaic_O2 USE_DOCKER=1

docker/mobilebert:
	$(MAKE) mobilebert USE_DOCKER=1

docker/mobilebert_O2:
	$(MAKE) mobilebert_O2 USE_DOCKER=1

docker/snusr:
	$(MAKE) snusr USE_DOCKER=1

docker/snusr_O2:
	$(MAKE) snusr_O2 USE_DOCKER=1

docker/mobilenet_v4_batched:
	$(MAKE) mobilenet_v4_batched USE_DOCKER=1

docker/mobilenet_v4_batched_O2:
	$(MAKE) mobilenet_v4_batched_O2 USE_DOCKER=1

htp-dlc: mobilenet_v4 \
	mobilenet_v4_O2 \
	mobiledet \
	mobiledet_O2 \
	mosaic \
	mosaic_O2 \
	mobilebert \
	mobilebert_O2 \
	snusr \
	snusr_O2 \
	mobilenet_v4_batched \
	mobilenet_v4_batched_O2


ifeq ($(MAKECMDGOALS),$(filter $(MAKECMDGOALS),generate-apirec \
mobilenet_v4_apirec mobilenet_v4_batched_apirec mosaic_apirec mobilebert_apirec mobiledet_apirec snusr_apirec \
docker/generate-apirec))
include ../make/apirec.mk
endif

docker/generate-apirec:
	$(MAKE) generate-apirec USE_DOCKER=1

generate-apirec: mobilenet_v4_apirec \
	mobilenet_v4_batched_apirec \
	mosaic_apirec \
	mobiledet_apirec \
	mobilebert_apirec \
	snusr_apirec

mobilenet_v4_batched: \
    ${DLCBUILDDIR}/mobilenet_v4_htp_batched_8_sm8850.stamp \
    ${DLCBUILDDIR}/mobilenet_v4_htp_batched_4.stamp

mobilenet_v4_batched_O2: \
    ${DLCBUILDDIR}/mobilenet_v4_htp_batched_8_O2_sm8850.stamp \
    ${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2.stamp

mobilenet_v4_batched_apirec: \
	${DLCBUILDDIR}/generate_mobilenet_v4_batched_4_apirec

mosaic: \
	${DLCBUILDDIR}/mobile_mosaic_htp.stamp

mosaic_O2: \
	${DLCBUILDDIR}/mobile_mosaic_htp_O2.stamp

mosaic_apirec: \
	${DLCBUILDDIR}/generate_mosaic_apirec

mobilenet_v4: \
	${DLCBUILDDIR}/mobilenet_v4_htp.stamp

mobilenet_v4_O2: \
	${DLCBUILDDIR}/mobilenet_v4_htp_O2.stamp

mobilenet_v4_apirec: \
	${DLCBUILDDIR}/generate_mobilenet_v4_apirec

mobiledet: \
	${DLCBUILDDIR}/ssd_mobiledet_qat_htp.stamp

mobiledet_O2: \
	${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2.stamp

mobiledet_apirec: \
	${DLCBUILDDIR}/generate_mobiledet_apirec

mobilebert: \
	${DLCBUILDDIR}/mobilebert_htp.stamp

mobilebert_O2: \
	${DLCBUILDDIR}/mobilebert_htp_O2.stamp

mobilebert_apirec: \
	${DLCBUILDDIR}/generate_mobilebert_apirec

snusr : \
	${DLCBUILDDIR}/snusr_htp.stamp

snusr_O2 : \
	${DLCBUILDDIR}/snusr_htp_O2.stamp

snusr_apirec: \
	${DLCBUILDDIR}/generate_snusr_apirec

# Convenience: "make docker/stable_diffusion_qnn" forces USE_DOCKER=1 and builds stable_diffusion_qnn
docker/stable_diffusion_qnn:
	$(MAKE) stable_diffusion_qnn USE_DOCKER=1

stable_diffusion_qnn: \
	sd_precompute_data \
	text_encoder \
	vae_decoder \
	unet

sd_precompute_data: \
	${DLCBUILDDIR}/sd_precompute_data.tar \

text_encoder: \
	${DLCBUILDDIR}/text_encoder_qnn_context_binary_generator.stamp \
	${DLCBUILDDIR}/text_encoder_qnn_context_binary_generator_v75.stamp \
	${DLCBUILDDIR}/text_encoder_qnn_context_binary_generator_v79.stamp \
	${DLCBUILDDIR}/text_encoder_qnn_context_binary_generator_v73_sd8sG4.stamp \
	${DLCBUILDDIR}/text_encoder_qnn_context_binary_generator_v73_sd7G4.stamp \
	${DLCBUILDDIR}/text_encoder_qnn_context_binary_generator_v81_sd8eliteG5.stamp \
	${DLCBUILDDIR}/text_encoder_qnn_context_binary_generator_v81_sd8G5.stamp \

vae_decoder: \
	${DLCBUILDDIR}/vae_decoder_qnn_context_binary_generator.stamp \
	${DLCBUILDDIR}/vae_decoder_qnn_context_binary_generator_v75.stamp \
	${DLCBUILDDIR}/vae_decoder_qnn_context_binary_generator_v79.stamp \
	${DLCBUILDDIR}/vae_decoder_qnn_context_binary_generator_v73_sd8sG4.stamp \
	${DLCBUILDDIR}/vae_decoder_qnn_context_binary_generator_v73_sd7G4.stamp \
	${DLCBUILDDIR}/vae_decoder_qnn_context_binary_generator_v81_sd8eliteG5.stamp \
	${DLCBUILDDIR}/vae_decoder_qnn_context_binary_generator_v81_sd8G5.stamp \


unet: \
	${DLCBUILDDIR}/unet_qnn_context_binary_generator.stamp \
	${DLCBUILDDIR}/unet_qnn_context_binary_generator_v75.stamp \
	${DLCBUILDDIR}/unet_qnn_context_binary_generator_v79.stamp \
	${DLCBUILDDIR}/unet_qnn_context_binary_generator_v73_sd8sG4.stamp \
	${DLCBUILDDIR}/unet_qnn_context_binary_generator_v73_sd7G4.stamp \
	${DLCBUILDDIR}/unet_qnn_context_binary_generator_v81_sd8eliteG5.stamp \
	${DLCBUILDDIR}/unet_qnn_context_binary_generator_v81_sd8G5.stamp \

mlperf_models: \
	${DLCBUILDDIR}/mlperf_models.stamp

${BUILDDIR}/datasets.stamp:
	(cd ../datasets && make)
	touch $@

${DLCBUILDDIR}/mobile/.stamp:
	# Downloading mobile
	mkdir -p ${DLCBUILDDIR}
	(cd ${DLCBUILDDIR} && git clone --depth=1 https://github.com/mlcommons/mobile_open.git ${DLCBUILDDIR}/mobile)
	(rm -rf ${DLCBUILDDIR}/mobile/vision/mobilenetV4)
	(cd ${DLCBUILDDIR} && wget https://github.com/mlcommons/mobile_open/releases/download/model_upload/MobileNetV4-Conv-Large-saved-model.zip && chmod -R 777 *)
	(cd ${DLCBUILDDIR} && unzip MobileNetV4-Conv-Large-saved-model.zip -d mobile/vision/mobilenetV4)
	(cd ${DLCBUILDDIR} && rm -rf MobileNetV4-Conv-Large-saved-model.zip)
	touch $@

${DLCBUILDDIR}/mlperf_models.stamp:
	# Creating folder to store required models
	(mkdir -p ${MLPERF_MODELS_PATH})
	touch $@


${DLCBUILDDIR}/mobilenet_v4_float.dlc: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# MobilenetV4 TPU model conversion ....
	# Float model
	mkdir -p ${DLCBUILDDIR}
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-tensorflow-to-dlc \
			-i $(SNPE_MOBILENETV4_DIR)/ \
			-d inputs "1,384,384,3" --out_node "probs" \
			-o $(SNPE_OUT)/mobilenet_v4_float.dlc

${DLCBUILDDIR}/imagenet/imagenet_image_384_list.txt: ${DLCBUILDDIR}/imagenet/state/quantdata_384.stamp
	cat ${TOPDIR}/datasets/downloads/imagenet/cal_image_list_option_1.txt | sed "s!^!quantdata_384/!" | sed "s!JPEG!raw!" > $@

${DLCBUILDDIR}/mobilenet_v4_quant.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobilenet_v4_float.dlc \
		${DLCBUILDDIR}/imagenet/imagenet_image_384_list.txt
	# Quantization of MobilenetV4 DLC for HTP
	$(SNPE_IMAGENET_CD) $(DOCKER_RUN_SNPE_IMAGENET) \
		$(SNPE_BIN)/snpe-dlc-quant \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_float.dlc \
			--input_list=$(IMAGENET_BUILD_DIR)/imagenet_image_384_list.txt \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_quant.dlc
	# Mobilenetedge TPU model conversion completed
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_sm8750.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobilenet_v4_quant.stamp \
	# Offline prepare of MobilenetV4 DLC for HTP
	$(DOCKER_RUN_SNPE_IMAGENET) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_quant.dlc \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm8750,sm8845
	# MobilenetV4 TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_sm7550.stamp: \
		$(DOCKER_STAMP) \
	# Offline prepare of MobilenetV4 DLC for HTP
	$(DOCKER_RUN_SNPE_IMAGENET) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_htp.dlc \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm7550,sm6650,sm7635
	# MobilenetV4 TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_sm8850.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobilenet_v4_quant.stamp \
	# Offline prepare of MobilenetV4 DLC for HTP
	$(DOCKER_RUN_SNPE_IMAGENET) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=8 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_htp.dlc \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm8850
	# MobilenetV4 TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobilenet_v4_htp_sm8750.stamp \
		${DLCBUILDDIR}/mobilenet_v4_htp_sm7550.stamp \
		${DLCBUILDDIR}/mobilenet_v4_htp_sm8850.stamp \
	# Offline prepare of MobilenetV4 DLC for HTP
	$(DOCKER_RUN_SNPE_IMAGENET) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_htp.dlc \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm8650,sm7750,sm8735,sm8635,sm8350,sm7325,sm8450,sm8550,sm7475,sc8380xp,sc8280x
	# MobilenetV4 TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_O2_sm8750.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobilenet_v4_quant.stamp \
	# Offline prepare of MobilenetEdgeTPU DLC for HTP
	$(DOCKER_RUN_SNPE_IMAGENET) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_quant.dlc \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8750,sm8845
	# Mobilenetedge TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_O2_sm7550.stamp: \
		$(DOCKER_STAMP) \
	# Offline prepare of MobilenetEdgeTPU DLC for HTP
	$(DOCKER_RUN_SNPE_IMAGENET) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_htp_O2.dlc \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm7550,sm6650,sm7635
	# Mobilenetedge TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_O2_sm8850.stamp: \
		$(DOCKER_STAMP) \
	# Offline prepare of MobilenetV4 DLC for HTP
	$(DOCKER_RUN_SNPE_IMAGENET) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=8 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_htp_O2.dlc \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8850 \
			 --memorymapped_buffer_hint=true
	# MobilenetV4 TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_O2.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobilenet_v4_htp_O2_sm8750.stamp \
		${DLCBUILDDIR}/mobilenet_v4_htp_O2_sm7550.stamp \
		${DLCBUILDDIR}/mobilenet_v4_htp_O2_sm8850.stamp \
	# Offline prepare of MobilenetEdgeTPU DLC for HTP
	$(DOCKER_RUN_SNPE_IMAGENET) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_htp_O2.dlc \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8650,sm7750,sm8735,sm8635,sm8350,sm7325,sm8450,sm8550,sm7475,sc8380xp,sc8280x
	# Mobilenetedge TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_float_batched_8.dlc: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# MobilenetV4 TPU model conversion ....
	# Batched Float model
	mkdir -p ${DLCBUILDDIR}
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-tensorflow-to-dlc \
			-i $(SNPE_MOBILENETV4_DIR)/ \
			-d inputs "8,384,384,3" --out_node "probs" \
			-o $(SNPE_OUT)/mobilenet_v4_float_batched_8.dlc
	touch $@

${DLCBUILDDIR}/mobilenet_v4_quant_batched_8.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/imagenet/imagenet_image_384_list.txt \
		${DLCBUILDDIR}/mobilenet_v4_float_batched_8.dlc
	# Quantization of MobilenetV4 Batched DLC for HTP
	$(SNPE_IMAGENET_CD) $(DOCKER_RUN_SNPE_IMAGENET) \
		$(SNPE_BIN)/snpe-dlc-quant \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_float_batched_8.dlc \
			--input_list=$(IMAGENET_BUILD_DIR)/imagenet_image_384_list.txt \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_quant_batched_8.dlc
	# MobilenetV4 model conversion completed
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_batched_8_sm8850.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobilenet_v4_quant_batched_8.stamp
	# Offline prepare of MobilenetV4 Batched DLC for HTP
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=8 \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_quant_batched_8.dlc \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_htp_batched_8.dlc \
			--optimization_level 3 \
			--htp_socs=sm8850 \
			--htp_dlbc=true \
			--memorymapped_buffer_hint=true
	# MobilenetV4 model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_batched_8.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_batched_8_O2_sm8850.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobilenet_v4_quant_batched_8.stamp
	# Offline prepare of MobilenetV4 Batched DLC for HTP
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=8 \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_quant_batched_8.dlc \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_htp_batched_8_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8850 \
			--htp_dlbc=true \
			--memorymapped_buffer_hint=true
	# MobilenetV4 model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_batched_8_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_float_batched_4.dlc: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# MobilenetV4 TPU model conversion ....
	# Batched Float model
	mkdir -p ${DLCBUILDDIR}
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-tensorflow-to-dlc \
			-i $(SNPE_MOBILENETV4_DIR)/ \
			-d inputs "4,384,384,3" --out_node "probs" \
			-o $(SNPE_OUT)/mobilenet_v4_float_batched_4.dlc
	touch $@

${DLCBUILDDIR}/mobilenet_v4_quant_batched_4.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/imagenet/imagenet_image_384_list.txt \
		${DLCBUILDDIR}/mobilenet_v4_float_batched_4.dlc
	# Quantization of MobilenetV4 Batched DLC for HTP
	$(SNPE_IMAGENET_CD) $(DOCKER_RUN_SNPE_IMAGENET) \
		$(SNPE_BIN)/snpe-dlc-quant \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_float_batched_4.dlc \
			--input_list=$(IMAGENET_BUILD_DIR)/imagenet_image_384_list.txt \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_quant_batched_4.dlc
	touch $@
	# MobilenetV4 model conversion completed

${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_sm8750.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobilenet_v4_quant_batched_4.stamp
	# Offline prepare of MobilenetV4 Batched DLC for HTP
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_quant_batched_4.dlc \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_htp_batched_4.dlc \
			--optimization_level 3 \
			--htp_socs=sm8750,sm8845
	# MobilenetV4 model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_batched_4.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_sm7550.stamp: \
		$(DOCKER_STAMP) \
	# Offline prepare of MobilenetV4 Batched DLC for HTP
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_htp_batched_4.dlc \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_htp_batched_4.dlc \
			--optimization_level 3 \
			--htp_socs=sm7550,sm6650,sm7635
	# MobilenetV4 model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_batched_4.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_sm8850.stamp: \
		$(DOCKER_STAMP) \
	# Offline prepare of MobilenetV4 Batched DLC for HTP
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=8 \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_htp_batched_4.dlc \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_htp_batched_4.dlc \
			--optimization_level 3 \
			--htp_socs=sm8850
	# MobilenetV4 model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_batched_4.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_batched_4.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_sm8750.stamp \
		${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_sm7550.stamp \
		${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_sm8850.stamp \
	# Offline prepare of MobilenetV4 Batched DLC for HTP
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_htp_batched_4.dlc \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_htp_batched_4.dlc \
			--optimization_level 3 \
			--htp_socs=sm8650,sm7750,sm8735,sm8635,sm8350,sm7325,sm8450,sm8550,sm7475,sc8380xp,sc8280x
	# MobilenetV4 model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_batched_4.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2_sm8750.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobilenet_v4_quant_batched_4.stamp
	# Offline prepare of MobilenetV4 Batched DLC for HTP
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_quant_batched_4.dlc \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_htp_batched_4_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8750,sm8845
	# MobilenetV4 model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2_sm7550.stamp: \
		$(DOCKER_STAMP) \
	# Offline prepare of MobilenetV4 Batched DLC for HTP
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_htp_batched_4_O2.dlc \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_htp_batched_4_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm7550,sm6650,sm7635
	# MobilenetV4 model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2_sm8850.stamp: \
		$(DOCKER_STAMP) \
	# Offline prepare of MobilenetV4 Batched DLC for HTP
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=8 \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_htp_batched_4_O2.dlc \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_htp_batched_4_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8850
	# MobilenetV4 model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2_sm8750.stamp \
		${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2_sm7550.stamp \
		${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2_sm8850.stamp \
	# Offline prepare of MobilenetV4 Batched DLC for HTP
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
			--input_dlc=$(SNPE_OUT)/mobilenet_v4_htp_batched_4_O2.dlc \
			--output_dlc=$(SNPE_OUT)/mobilenet_v4_htp_batched_4_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8650,sm7750,sm8735,sm8635,sm8350,sm7325,sm8450,sm8550,sm7475,sc8380xp,sc8280x
	# MobilenetV4 model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/ssd_mobiledet_qat.dlc: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-tensorflow-to-dlc \
			-i $(SNPE_MOBILEDET_DIR)/frozen_inference_graph.pb \
			-d Preprocessor/map/TensorArrayStack/TensorArrayGatherV3 "1,320,320,3" \
			--out_node "detection_classes" \
			--out_node "detection_boxes" \
			--out_node "detection_scores" \
			-o $(SNPE_OUT)/ssd_mobiledet_qat.dlc

${DLCBUILDDIR}/coco/coco_image_list.txt: ${DLCBUILDDIR}/coco/state/quantdata.stamp
	cat ${TOPDIR}/datasets/util/coco/coco_calibration_files.txt | sed "s!^!quantdata/!" | sed "s!jpg!raw!" > ${DLCBUILDDIR}/coco/coco_image_list.txt

${DLCBUILDDIR}/ssd_mobiledet_qat_quant.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/coco/coco_image_list.txt \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp \
		${DLCBUILDDIR}/ssd_mobiledet_qat.dlc
	# Quantization of MobileDET SSD DLC for HTP
	$(SNPE_COCO_CD) $(DOCKER_RUN_SNPE_COCO) \
		$(SNPE_BIN)/snpe-dlc-quant \
			--input_dlc=$(SNPE_OUT)/ssd_mobiledet_qat.dlc \
			--input_list=$(COCO_BUILD_DIR)/coco_image_list.txt \
			--output_dlc=$(SNPE_OUT)/ssd_mobiledet_qat_quant.dlc
	# SSD MobileDET model quantization completed

${DLCBUILDDIR}/ssd_mobiledet_qat_htp_sm8750.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/ssd_mobiledet_qat_quant.stamp
	# Offline prepare of MobileDET SSD DLC for HTP
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/ssd_mobiledet_qat_quant.dlc \
			--output_dlc=$(SNPE_OUT)/ssd_mobiledet_qat_htp.dlc \
			--set_output_tensors="Postprocessor/BatchMultiClassNonMaxSuppression_classes,Postprocessor/BatchMultiClassNonMaxSuppression_num_detections,detection_scores:0,detection_boxes:0"\
			--optimization_level 3 \
			--htp_socs=sm8750,sm8845
	# SSD MobileDET model offline prepare for HTP completed
	cp	${DLCBUILDDIR}/ssd_mobiledet_qat_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/ssd_mobiledet_qat_htp_sm7550.stamp: \
		$(DOCKER_STAMP) \
	# Offline prepare of MobileDET SSD DLC for HTP
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/ssd_mobiledet_qat_htp.dlc \
			--output_dlc=$(SNPE_OUT)/ssd_mobiledet_qat_htp.dlc \
			--set_output_tensors="Postprocessor/BatchMultiClassNonMaxSuppression_classes,Postprocessor/BatchMultiClassNonMaxSuppression_num_detections,detection_scores:0,detection_boxes:0"\
			--optimization_level 3 \
			--htp_socs=sm7550,sm6650,sm7635 \
			--memorymapped_buffer_hint=true
	# SSD MobileDET model offline prepare for HTP completed
	cp	${DLCBUILDDIR}/ssd_mobiledet_qat_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/ssd_mobiledet_qat_htp_sm8850.stamp: \
		$(DOCKER_STAMP) \
	# Offline prepare of MobileDET SSD DLC for HTP
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=8 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/ssd_mobiledet_qat_htp.dlc \
			--output_dlc=$(SNPE_OUT)/ssd_mobiledet_qat_htp.dlc \
			--set_output_tensors="Postprocessor/BatchMultiClassNonMaxSuppression_classes,Postprocessor/BatchMultiClassNonMaxSuppression_num_detections,detection_scores:0,detection_boxes:0"\
			--optimization_level 3 \
			--htp_socs=sm8850
	# SSD MobileDET model offline prepare for HTP completed
	cp	${DLCBUILDDIR}/ssd_mobiledet_qat_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/ssd_mobiledet_qat_htp.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/ssd_mobiledet_qat_htp_sm8750.stamp \
		${DLCBUILDDIR}/ssd_mobiledet_qat_htp_sm7550.stamp \
		${DLCBUILDDIR}/ssd_mobiledet_qat_htp_sm8850.stamp \
	# Offline prepare of MobileDET SSD DLC for HTP
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
            --htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/ssd_mobiledet_qat_htp.dlc \
			--output_dlc=$(SNPE_OUT)/ssd_mobiledet_qat_htp.dlc \
			--set_output_tensors="Postprocessor/BatchMultiClassNonMaxSuppression_classes,Postprocessor/BatchMultiClassNonMaxSuppression_num_detections,detection_scores:0,detection_boxes:0"\
			--optimization_level 3 \
			--htp_socs=sm8650,sm7750,sm8735,sm8635,sm8350,sm7325,sm8450,sm8550,sm7475,sc8380xp,sc8280x
	# SSD MobileDET model offline prepare for HTP completed
	cp	${DLCBUILDDIR}/ssd_mobiledet_qat_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2_sm8750.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/ssd_mobiledet_qat_quant.stamp
	# Offline prepare of MobileDET SSD DLC for HTP
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--input_dlc=$(SNPE_OUT)/ssd_mobiledet_qat_quant.dlc \
			--output_dlc=$(SNPE_OUT)/ssd_mobiledet_qat_htp_O2.dlc \
			--set_output_tensors="Postprocessor/BatchMultiClassNonMaxSuppression_classes,Postprocessor/BatchMultiClassNonMaxSuppression_num_detections,detection_scores:0,detection_boxes:0"\
			--optimization_level 2 \
			--htp_socs=sm8750,sm8845
	# SSD MobileDET model offline prepare for HTP completed
	cp	${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2_sm7550.stamp: \
		$(DOCKER_STAMP) \
	# Offline prepare of MobileDET SSD DLC for HTP
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
			--input_dlc=$(SNPE_OUT)/ssd_mobiledet_qat_htp_O2.dlc \
			--output_dlc=$(SNPE_OUT)/ssd_mobiledet_qat_htp_O2.dlc \
			--set_output_tensors="Postprocessor/BatchMultiClassNonMaxSuppression_classes,Postprocessor/BatchMultiClassNonMaxSuppression_num_detections,detection_scores:0,detection_boxes:0"\
			--optimization_level 2 \
			--htp_socs=sm7550,sm6650,sm7635
	# SSD MobileDET model offline prepare for HTP completed
	cp	${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2_sm8850.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/ssd_mobiledet_qat_quant.stamp
	# Offline prepare of MobileDET SSD DLC for HTP
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=8 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/ssd_mobiledet_qat_htp_O2.dlc \
			--output_dlc=$(SNPE_OUT)/ssd_mobiledet_qat_htp_O2.dlc \
			--set_output_tensors="Postprocessor/BatchMultiClassNonMaxSuppression_classes,Postprocessor/BatchMultiClassNonMaxSuppression_num_detections,detection_scores:0,detection_boxes:0"\
			--optimization_level 2 \
			--htp_socs=sm8850
	# SSD MobileDET model offline prepare for HTP completed
	cp	${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2_sm8750.stamp \
		${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2_sm7550.stamp \
		${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2_sm8850.stamp \
	# Offline prepare of MobileDET SSD DLC for HTP
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
			--input_dlc=$(SNPE_OUT)/ssd_mobiledet_qat_htp_O2.dlc \
			--output_dlc=$(SNPE_OUT)/ssd_mobiledet_qat_htp_O2.dlc \
			--set_output_tensors="Postprocessor/BatchMultiClassNonMaxSuppression_classes,Postprocessor/BatchMultiClassNonMaxSuppression_num_detections,detection_scores:0,detection_boxes:0"\
			--optimization_level 2 \
			--htp_socs=sm8650,sm7750,sm8735,sm8635,sm8350,sm7325,sm8450,sm8550,sm7475,sc8380xp,sc8280x
	# SSD MobileDET model offline prepare for HTP completed
	cp	${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/ade20k/ade20k_image_list.txt: ${DLCBUILDDIR}/ade20k/state/quantdata.stamp
	mkdir -p ${DLCBUILDDIR}/ade20k
	cat ${TOPDIR}/datasets/util/ade20k/ade20k_calibration_files.txt | sed "s!^!$(ADE20K_QUANTDATA_PREFIX)!" | sed "s!jpg!raw!" > $@

${DLCBUILDDIR}/mobilebert_quant.pb: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# MobileBERT quant model freeze ....
	mkdir -p ${DLCBUILDDIR}
	$(DOCKER_RUN_SNPE) \
		$(FREEZE_GRAPH_CMD) \
			--input_graph=$(SNPE_MOBILEBERT_DIR)/saved_model.pb --input_checkpoint=$(SNPE_MOBILEBERT_DIR)/checkpoints/quant \
			--output_graph=$(SNPE_OUT)/mobilebert_quant.pb \
			--output_node_names=end_logits,start_logits \
			--input_binary= True \
			--input_saved_model_dir=$(SNPE_MOBILEBERT_DIR)/ saved_model_tags="serve"

${DLCBUILDDIR}/mobilebert_float.dlc: \
		${DLCBUILDDIR}/mobilebert_quant.pb \
		$(DOCKER_STAMP) \
		${DATASETS_OUT}/state/calibration.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# MobileBERT TF to DLC conversion
	mkdir -p ${DLCBUILDDIR}
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-tensorflow-to-dlc \
			--input_network $(SNPE_OUT)/mobilebert_quant.pb \
			--input_dim bert/embeddings/ExpandDims 1,384,1 \
			--input_dim input_mask 1,384 \
			--input_dim segment_ids 1,384 \
			--input_type bert/embeddings/ExpandDims opaque \
			--input_type input_mask default \
			--input_type segment_ids opaque \
			--out_node transpose \
			-o $(SNPE_OUT)/mobilebert_float.dlc
	touch $@
	# MobileBERT Float DLC conversion completed

${DLCBUILDDIR}/mobilebert_quant.stamp: \
		${DLCBUILDDIR}/mobilebert_float.dlc \
		$(DOCKER_STAMP) \
		${DATASETS}/output/state/squad_calibration.stamp
	# MobileBERT float model quantization ...
	mkdir -p ${DLCBUILDDIR}
	$(SNPE_SQUAD_CD) $(DOCKER_RUN_SNPE_SQUAD) \
		$(SNPE_BIN)/snpe-dlc-quant \
			--input_dlc=$(SNPE_OUT)/mobilebert_float.dlc \
			--input_list=$(SQUAD_BUILD_DIR)/input_list.txt \
			--weights_bitwidth 8 \
			--act_bitwidth 8 \
			--override_params \
			--output_dlc=$(SNPE_OUT)/mobilebert_quant.dlc
	touch $@

${DLCBUILDDIR}/mobilebert_htp_sm8750.stamp: \
		${DLCBUILDDIR}/mobilebert_quant.stamp \
		$(DOCKER_STAMP) \
	# MobileBERT quant model offline prepare ...
	mkdir -p ${DLCBUILDDIR}
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobilebert_quant.dlc \
			--output_dlc=$(SNPE_OUT)/mobilebert_quantized_htp.dlc \
			--set_output_tensors="transpose:0" \
			--optimization_level 3 \
			--htp_socs=sm8750,sm8845
	cp	${DLCBUILDDIR}/mobilebert_quantized_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilebert_htp_sm7550.stamp: \
		$(DOCKER_STAMP) \
	# MobileBERT quant model offline prepare ...
	mkdir -p ${DLCBUILDDIR}
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobilebert_quantized_htp.dlc \
			--output_dlc=$(SNPE_OUT)/mobilebert_quantized_htp.dlc \
			--set_output_tensors="transpose:0" \
			--optimization_level 3 \
			--htp_socs=sm7550,sm6650,sm7635
	cp	${DLCBUILDDIR}/mobilebert_quantized_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilebert_htp_sm8850.stamp: \
		$(DOCKER_STAMP) \
	# MobileBERT quant model offline prepare ...
	mkdir -p ${DLCBUILDDIR}
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=8 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobilebert_quantized_htp.dlc \
			--output_dlc=$(SNPE_OUT)/mobilebert_quantized_htp.dlc \
			--set_output_tensors="transpose:0" \
			--optimization_level 3 \
			--htp_socs=sm8850
	cp	${DLCBUILDDIR}/mobilebert_quantized_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilebert_htp.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobilebert_htp_sm8750.stamp \
		${DLCBUILDDIR}/mobilebert_htp_sm7550.stamp \
		${DLCBUILDDIR}/mobilebert_htp_sm8850.stamp \
	# MobileBERT quant model offline prepare ...
	mkdir -p ${DLCBUILDDIR}
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobilebert_quantized_htp.dlc \
			--output_dlc=$(SNPE_OUT)/mobilebert_quantized_htp.dlc \
			--set_output_tensors="transpose:0" \
			--optimization_level 3 \
			--htp_socs=sm8650,sm7750,sm8735,sm8635,sm7325,sm8350,sm8450,sm8550,sm7475,sc8380xp,sc8280x
	cp	${DLCBUILDDIR}/mobilebert_quantized_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilebert_htp_O2_sm8750.stamp: \
		${DLCBUILDDIR}/mobilebert_quant.stamp \
		$(DOCKER_STAMP) \
	# MobileBERT quant model offline prepare ...
	mkdir -p ${DLCBUILDDIR}
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobilebert_quant.dlc \
			--output_dlc=$(SNPE_OUT)/mobilebert_quantized_htp_O2.dlc \
			--set_output_tensors="transpose:0" \
			--optimization_level 2 \
			--htp_socs=sm8750,sm8845
	cp	${DLCBUILDDIR}/mobilebert_quantized_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilebert_htp_O2_sm7550.stamp: \
		$(DOCKER_STAMP) \
	# MobileBERT quant model offline prepare ...
	mkdir -p ${DLCBUILDDIR}
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
            --htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobilebert_quantized_htp_O2.dlc \
			--output_dlc=$(SNPE_OUT)/mobilebert_quantized_htp_O2.dlc \
			--set_output_tensors="transpose:0" \
			--optimization_level 2 \
			--htp_socs=sm7550,sm6650,sm7635
	cp	${DLCBUILDDIR}/mobilebert_quantized_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilebert_htp_O2_sm8850.stamp: \
		$(DOCKER_STAMP) \
	# MobileBERT quant model offline prepare ...
	mkdir -p ${DLCBUILDDIR}
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=8 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobilebert_quantized_htp_O2.dlc \
			--output_dlc=$(SNPE_OUT)/mobilebert_quantized_htp_O2.dlc \
			--set_output_tensors="transpose:0" \
			--optimization_level 2 \
			--htp_socs=sm8850
	cp	${DLCBUILDDIR}/mobilebert_quantized_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilebert_htp_O2.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobilebert_htp_O2_sm8750.stamp \
		${DLCBUILDDIR}/mobilebert_htp_O2_sm7550.stamp \
		${DLCBUILDDIR}/mobilebert_htp_O2_sm8850.stamp \
	# MobileBERT quant model offline prepare ...
	mkdir -p ${DLCBUILDDIR}
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobilebert_quantized_htp_O2.dlc \
			--output_dlc=$(SNPE_OUT)/mobilebert_quantized_htp_O2.dlc \
			--set_output_tensors="transpose:0" \
			--optimization_level 2 \
			--htp_socs=sm8650,sm7750,sm8735,sm8635,sm7325,sm8350,sm8450,sm8550,sm7475,sc8380xp,sc8280x
	cp	${DLCBUILDDIR}/mobilebert_quantized_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobile_mosaic_float.dlc: \
		${DLCBUILDDIR}/mobile/.stamp \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mlperf_models.stamp
	# Mobile Mosaic model conversion ....
	# Mobile Mosaic QAT model
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-tensorflow-to-dlc \
			-i $(SNPE_MOSAIC_DIR)/mobile_segmenter_r4_frozen_graph.pb \
			-d sub_2 "1,512,512,3" \
			--out_node ArgMax \
			-o $(SNPE_OUT)/mobile_mosaic_float.dlc
	# Generated DLC from mobile Mosaic QAT model
	touch $@

${DLCBUILDDIR}/mobile_mosaic_quant.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/ade20k/ade20k_image_list.txt \
		${DLCBUILDDIR}/mobile_mosaic_float.dlc
	# Quantization of Mobile Mosaic DLC
	$(SNPE_ADE20K_CD) $(DOCKER_RUN_SNPE_ADE20K) \
		$(SNPE_BIN)/snpe-dlc-quant \
			--input_dlc=$(SNPE_OUT)/mobile_mosaic_float.dlc \
			--input_list=$(ADE20K_BUILD_DIR)/ade20k_image_list.txt \
			--output_dlc=$(SNPE_OUT)/mobile_mosaic_quant.dlc \
			--optimizations cle
	#Mobile Mosaic model quantization completed
	touch $@

${DLCBUILDDIR}/mobile_mosaic_htp_sm8750.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile_mosaic_quant.stamp \
	# Offline prepare of Mobile Mosaic DLC
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobile_mosaic_quant.dlc \
			--output_dlc=$(SNPE_OUT)/mobile_mosaic_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm8750,sm8845
	#Mobile Mosaic HTP model offline prepare completed
	cp	${DLCBUILDDIR}/mobile_mosaic_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobile_mosaic_htp_sm7550.stamp: \
		$(DOCKER_STAMP) \
	# Offline prepare of Mobile Mosaic DLC
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
            --htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobile_mosaic_htp.dlc \
			--output_dlc=$(SNPE_OUT)/mobile_mosaic_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm7550,sm6650,sm7635
	#Mobile Mosaic HTP model offline prepare completed
	cp	${DLCBUILDDIR}/mobile_mosaic_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobile_mosaic_htp_sm8850.stamp: \
		$(DOCKER_STAMP) \
	# Offline prepare of Mobile Mosaic DLC
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=8 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobile_mosaic_htp.dlc \
			--output_dlc=$(SNPE_OUT)/mobile_mosaic_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm8850
	#Mobile Mosaic HTP model offline prepare completed
	cp	${DLCBUILDDIR}/mobile_mosaic_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobile_mosaic_htp.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile_mosaic_htp_sm8750.stamp \
		${DLCBUILDDIR}/mobile_mosaic_htp_sm7550.stamp \
		${DLCBUILDDIR}/mobile_mosaic_htp_sm8850.stamp \
	# Offline prepare of Mobile Mosaic DLC
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
            --htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobile_mosaic_htp.dlc \
			--output_dlc=$(SNPE_OUT)/mobile_mosaic_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm8650,sm7750,sm8735,sm8635,sm7325,sm8350,sm8450,sm8550,sm7475,sc8380xp,sc8280x
	#Mobile Mosaic HTP model offline prepare completed
	cp	${DLCBUILDDIR}/mobile_mosaic_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobile_mosaic_htp_O2_sm8750.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile_mosaic_quant.stamp \
	# Offline prepare of Mobile Mosaic DLC
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--input_dlc=$(SNPE_OUT)/mobile_mosaic_quant.dlc \
			--output_dlc=$(SNPE_OUT)/mobile_mosaic_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8750,sm8845
	#Mobile Mosaic HTP model offline prepare completed
	cp	${DLCBUILDDIR}/mobile_mosaic_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobile_mosaic_htp_O2_sm7550.stamp: \
		$(DOCKER_STAMP) \
	# Offline prepare of Mobile Mosaic DLC
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
			--input_dlc=$(SNPE_OUT)/mobile_mosaic_htp_O2.dlc \
			--output_dlc=$(SNPE_OUT)/mobile_mosaic_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm7550,sm6650,sm7635
	#Mobile Mosaic HTP model offline prepare completed
	cp	${DLCBUILDDIR}/mobile_mosaic_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobile_mosaic_htp_O2_sm8850.stamp: \
		$(DOCKER_STAMP) \
	# Offline prepare of Mobile Mosaic DLC
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=8 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/mobile_mosaic_htp_O2.dlc \
			--output_dlc=$(SNPE_OUT)/mobile_mosaic_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8850
	#Mobile Mosaic HTP model offline prepare completed
	cp	${DLCBUILDDIR}/mobile_mosaic_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobile_mosaic_htp_O2.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile_mosaic_htp_O2_sm8750.stamp \
		${DLCBUILDDIR}/mobile_mosaic_htp_O2_sm7550.stamp \
		${DLCBUILDDIR}/mobile_mosaic_htp_O2_sm8850.stamp \
	# Offline prepare of Mobile Mosaic DLC
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
			--input_dlc=$(SNPE_OUT)/mobile_mosaic_htp_O2.dlc \
			--output_dlc=$(SNPE_OUT)/mobile_mosaic_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8650,sm7750,sm8735,sm8635,sm7325,sm8350,sm8450,sm8550,sm7475,sc8380xp,sc8280x
	#Mobile Mosaic HTP model offline prepare completed
	cp	${DLCBUILDDIR}/mobile_mosaic_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/snusr_float.dlc: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# SNUSR model conversion ....
	# Float model
	mkdir -p ${DLCBUILDDIR}
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-tensorflow-to-dlc \
			-i $(SNPE_SNUSR_DIR) \
			-d input_1 "1,540,960,3" --out_node "lambda_1" \
			-o $(SNPE_OUT)/snusr_float.dlc
	# SNUSR float DLC completed
	touch $@

${DLCBUILDDIR}/snusr_quant.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/snusr_float.dlc \
		${DLCBUILDDIR}/snusr_calibration_list.txt
	# Offline prepare of SNUSR DLC
	$(SNPE_SNUSR_CD) $(DOCKER_RUN_SNPE_SNUSR) \
		$(SNPE_BIN)/snpe-dlc-quant \
			--input_dlc=$(SNPE_OUT)/snusr_float.dlc \
			--input_list=$(SNUSR_BUILD_DIR)/snusr_calibration_list.txt \
			--output_dlc=$(SNPE_OUT)/snusr_quant.dlc \
	#SNUSR offline prepare completed
	touch $@

${DLCBUILDDIR}/snusr_htp_sm8750.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/snusr_quant.stamp \
		${DLCBUILDDIR}/snusr_calibration_list.txt
	#HTP Graph prepare of SNUSR DLC
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/snusr_quant.dlc \
			--output_dlc=$(SNPE_OUT)/snusr_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm8750,sm8845 \
			--memorymapped_buffer_hint=true
	#HTP Graph prepare of SNUSR model completed
	cp	${DLCBUILDDIR}/snusr_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/snusr_htp_sm7550.stamp: \
		$(DOCKER_STAMP) \
	#HTP Graph prepare of SNUSR DLC
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/snusr_htp.dlc \
			--output_dlc=$(SNPE_OUT)/snusr_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm7550,sm6650,sm7635 \
			--memorymapped_buffer_hint=true
	#HTP Graph prepare of SNUSR model completed
	cp	${DLCBUILDDIR}/snusr_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/snusr_htp_sm8850.stamp: \
		$(DOCKER_STAMP) \
	#HTP Graph prepare of SNUSR DLC
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=8 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/snusr_htp.dlc \
			--output_dlc=$(SNPE_OUT)/snusr_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm8850 \
			--memorymapped_buffer_hint=true
	#HTP Graph prepare of SNUSR model completed
	cp	${DLCBUILDDIR}/snusr_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/snusr_htp.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/snusr_htp_sm8750.stamp \
		${DLCBUILDDIR}/snusr_htp_sm7550.stamp \
		${DLCBUILDDIR}/snusr_htp_sm8850.stamp \
	#HTP Graph prepare of SNUSR DLC
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/snusr_htp.dlc \
			--output_dlc=$(SNPE_OUT)/snusr_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm8650,sm7750,sm8735,sm8635,sm7325,sm8350,sm8450,sm8550,sm7475,sc8380xp,sc8280x \
			--memorymapped_buffer_hint=true
	#HTP Graph prepare of SNUSR model completed
	cp	${DLCBUILDDIR}/snusr_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/snusr_htp_O2_sm8750.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/snusr_quant.stamp \
		${DLCBUILDDIR}/snusr_calibration_list.txt
	#HTP Graph prepare of SNUSR DLC
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--input_dlc=$(SNPE_OUT)/snusr_quant.dlc \
			--output_dlc=$(SNPE_OUT)/snusr_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8750,sm8845 \
			--memorymapped_buffer_hint=true
	#HTP Graph prepare of SNUSR model completed
	cp	${DLCBUILDDIR}/snusr_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/snusr_htp_O2_sm7550.stamp: \
        $(DOCKER_STAMP) \
	#HTP Graph prepare of SNUSR DLC
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
			--input_dlc=$(SNPE_OUT)/snusr_htp_O2.dlc \
			--output_dlc=$(SNPE_OUT)/snusr_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm7550,sm6650,sm7635 \
			--memorymapped_buffer_hint=true
	#HTP Graph prepare of SNUSR model completed
	cp	${DLCBUILDDIR}/snusr_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/snusr_htp_O2_sm8850.stamp: \
		$(DOCKER_STAMP) \
	#HTP Graph prepare of SNUSR DLC
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=8 \
			--htp_dlbc=true \
			--input_dlc=$(SNPE_OUT)/snusr_htp_O2.dlc \
			--output_dlc=$(SNPE_OUT)/snusr_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8850 \
			--memorymapped_buffer_hint=true
	#HTP Graph prepare of SNUSR model completed
	cp	${DLCBUILDDIR}/snusr_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/snusr_htp_O2.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/snusr_htp_O2_sm8750.stamp \
		${DLCBUILDDIR}/snusr_htp_O2_sm7550.stamp \
		${DLCBUILDDIR}/snusr_htp_O2_sm8850.stamp \
	#HTP Graph prepare of SNUSR DLC
	$(DOCKER_RUN_SNPE) \
		$(SNPE_BIN)/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
			--input_dlc=$(SNPE_OUT)/snusr_htp_O2.dlc \
			--output_dlc=$(SNPE_OUT)/snusr_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8650,sm7750,sm8735,sm8635,sm7325,sm8350,sm8450,sm8550,sm7475,sc8380xp,sc8280x \
			--memorymapped_buffer_hint=true
	#HTP Graph prepare of SNUSR model completed
	cp	${DLCBUILDDIR}/snusr_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/sd_precompute_data.tar: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
	# Preparing sd_precompute_data.tar
	$(DOCKER_RUN_QNN) \
		python3 /util/flatten.py \
			--random_latent_init /stable_diffusion/random_latent_init/random_init_1.pkl \
            --time_step_embedding /stable_diffusion/time_step_embeddings/unet_time_step_embeddings_20.pkl \
            --time_step_list /stable_diffusion/scheduler/scheduler_time_steps_20.pkl \
            --unconditional_text_emb /stable_diffusion/unconditional_text_emb.pkl \
            --dumpdir /stable_diffusion/.
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v75
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v79
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd8sG4
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd7G4
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8eliteG5
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8G5
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v75
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v79
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd8sG4
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd7G4
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8eliteG5
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8G5
	cp ${DLCBUILDDIR}/stable_diffusion/sd_precompute_data.tar ${MLPERF_MODELS_PATH}/stable_diffusion/.
	cp ${DLCBUILDDIR}/stable_diffusion/scheduler/lambdas.bin ${MLPERF_MODELS_PATH}/stable_diffusion/.
	cp ${DLCBUILDDIR}/stable_diffusion/scheduler/betas.bin ${MLPERF_MODELS_PATH}/stable_diffusion/.
	cp ${DLCBUILDDIR}/stable_diffusion/sd_precompute_data.tar ${MLPERF_MODELS_PATH}/stable_diffusion_v75/.
	cp ${DLCBUILDDIR}/stable_diffusion/scheduler/lambdas.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v75/.
	cp ${DLCBUILDDIR}/stable_diffusion/scheduler/betas.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v75/.
	cp ${DLCBUILDDIR}/stable_diffusion/sd_precompute_data.tar ${MLPERF_MODELS_PATH}/stable_diffusion_v79/.
	cp ${DLCBUILDDIR}/stable_diffusion/scheduler/lambdas.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v79/.
	cp ${DLCBUILDDIR}/stable_diffusion/scheduler/betas.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v79/.
	cp ${DLCBUILDDIR}/stable_diffusion/sd_precompute_data.tar ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd8sG4/.
	cp ${DLCBUILDDIR}/stable_diffusion/scheduler/lambdas.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd8sG4/.
	cp ${DLCBUILDDIR}/stable_diffusion/scheduler/betas.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd8sG4/.
	cp ${DLCBUILDDIR}/stable_diffusion/sd_precompute_data.tar ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd7G4/.
	cp ${DLCBUILDDIR}/stable_diffusion/scheduler/lambdas.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd7G4/.
	cp ${DLCBUILDDIR}/stable_diffusion/scheduler/betas.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd7G4/.
	cp ${DLCBUILDDIR}/stable_diffusion/sd_precompute_data.tar ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8eliteG5/.
	cp ${DLCBUILDDIR}/stable_diffusion/scheduler/lambdas.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8eliteG5/.
	cp ${DLCBUILDDIR}/stable_diffusion/scheduler/betas.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8eliteG5/.
	cp ${DLCBUILDDIR}/stable_diffusion/sd_precompute_data.tar ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8G5/.
	cp ${DLCBUILDDIR}/stable_diffusion/scheduler/lambdas.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8G5/.
	cp ${DLCBUILDDIR}/stable_diffusion/scheduler/betas.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8G5/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v75
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v79
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd8sG4
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd7G4
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8eliteG5
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8G5
	touch $@

${DLCBUILDDIR}/text_encoder_qnn.cpp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# TEXT-ENCODER conversion and quantization
    # cpp & bin files
	$(DOCKER_RUN_QNN) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-onnx-converter \
			--input_network /stable_diffusion/text_encoder/text_encoder_onnx/text_encoder.onnx \
			--input_list /stable_diffusion/text_encoder/stable_diffusion_models/text_encoder_onnx/text_encoder_input_list.txt \
			--act_bitwidth 16 \
			--bias_bitwidth 32 \
			--quantization_overrides /stable_diffusion/text_encoder/text_encoder_onnx/text_encoder.encodings \
			--output_path /output/text_encoder.cpp
		touch $@

${DLCBUILDDIR}/text_encoder_qnn_model_generator.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/text_encoder_qnn.cpp
	# TEXT-ENCODER lib generation started
	$(DOCKER_RUN_QNN) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-model-lib-generator \
			-c /output/text_encoder.cpp \
			-b /output/text_encoder.bin \
			-o /output/model_libs \
			-t x86_64-linux-clang
	touch $@
	# Text-encoder lib generation completed

${DLCBUILDDIR}/text_encoder_qnn_context_binary_generator.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/text_encoder_qnn_model_generator.stamp
	# TEXT-ENCODER context-binary generation started
	$(DOCKER_RUN_QNN) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libtext_encoder.so \
			--binary_file /output/text_encoder.serialized
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion
	cp ${DLCBUILDDIR}/text_encoder.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion
	touch $@
	# TEXT-ENCODER context binary generation completed

${DLCBUILDDIR}/text_encoder_qnn_context_binary_generator_v75.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/text_encoder_qnn_model_generator.stamp
	# TEXT-ENCODER context-binary generation started
	$(DOCKER_RUN_QNN_TEXT_ENCODER) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libtext_encoder.so \
			--binary_file /output/text_encoder.serialized \
			--config_file /models/mcp_config_v75.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v75
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v75
	cp ${DLCBUILDDIR}/text_encoder.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v75/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v75
	touch $@
	# TEXT-ENCODER context binary generation for v75 completed

${DLCBUILDDIR}/text_encoder_qnn_context_binary_generator_v79.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/text_encoder_qnn_model_generator.stamp
	# TEXT-ENCODER context-binary generation started
	$(DOCKER_RUN_QNN_TEXT_ENCODER) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libtext_encoder.so \
			--binary_file /output/text_encoder.serialized \
			--config_file /models/mcp_config_v79.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v79
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v79
	cp ${DLCBUILDDIR}/text_encoder.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v79/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v79
	touch $@
	# TEXT-ENCODER context binary generation for v79 completed

${DLCBUILDDIR}/text_encoder_qnn_context_binary_generator_v73_sd8sG4.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/text_encoder_qnn_model_generator.stamp
	# TEXT-ENCODER context-binary generation started
	$(DOCKER_RUN_QNN_TEXT_ENCODER) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libtext_encoder.so \
			--binary_file /output/text_encoder.serialized \
			--config_file /models/mcp_config_v73_sd8sG4.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd8sG4
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd8sG4
	cp ${DLCBUILDDIR}/text_encoder.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd8sG4/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd8sG4
	# TEXT-ENCODER context binary generation for v73 sd8sG4 completed

${DLCBUILDDIR}/text_encoder_qnn_context_binary_generator_v73_sd7G4.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/text_encoder_qnn_model_generator.stamp
	# TEXT-ENCODER context-binary generation started
	$(DOCKER_RUN_QNN_TEXT_ENCODER) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libtext_encoder.so \
			--binary_file /output/text_encoder.serialized \
			--config_file /models/mcp_config_v73_sd7G4.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd7G4
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd7G4
	cp ${DLCBUILDDIR}/text_encoder.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd7G4/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd7G4
	# TEXT-ENCODER context binary generation for v73 sd7G4 completed

${DLCBUILDDIR}/text_encoder_qnn_context_binary_generator_v81_sd8eliteG5.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/text_encoder_qnn_model_generator.stamp
	# TEXT-ENCODER context-binary generation started
	$(DOCKER_RUN_QNN_TEXT_ENCODER) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libtext_encoder.so \
			--binary_file /output/text_encoder.serialized \
			--config_file /models/mcp_config_v81_sd8eliteG5.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8eliteG5
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8eliteG5
	cp ${DLCBUILDDIR}/text_encoder.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8eliteG5/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8eliteG5
	touch $@
	# TEXT-ENCODER context binary generation for v81 sd8eliteG5 completed

${DLCBUILDDIR}/text_encoder_qnn_context_binary_generator_v81_sd8G5.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/text_encoder_qnn_model_generator.stamp
	# TEXT-ENCODER context-binary generation started
	$(DOCKER_RUN_QNN_TEXT_ENCODER) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libtext_encoder.so \
			--binary_file /output/text_encoder.serialized \
			--config_file /models/mcp_config_v81_sd8G5.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8G5
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8G5
	cp ${DLCBUILDDIR}/text_encoder.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8G5/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8G5
	touch $@
	# TEXT-ENCODER context binary generation for v81 sd8G5 completed

${DLCBUILDDIR}/vae_decoder_qnn.cpp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# VAE-DECODER conversion and quantization
	# cpp & bin files
	$(DOCKER_RUN_QNN) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-onnx-converter \
			--input_network /stable_diffusion/vae_decoder/vae_decoder_onnx/vae_decoder.onnx \
			--input_list /stable_diffusion/vae_decoder/stable_diffusion_models/vae_decoder_onnx/vae_decoder_input_list.txt  \
			--act_bitwidth 16 \
			--bias_bitwidth 32 \
			--quantization_overrides /stable_diffusion/vae_decoder/vae_decoder_onnx/vae_decoder.encodings \
			--output_path /output/vae_decoder.cpp
	touch $@

${DLCBUILDDIR}/vae_decoder_qnn_model_generator.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/vae_decoder_qnn.cpp
	# VAE-DECODER lib generation started
	$(DOCKER_RUN_QNN) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-model-lib-generator \
			-c /output/vae_decoder.cpp \
			-b /output/vae_decoder.bin \
			-o /output/model_libs \
			-t x86_64-linux-clang
	touch $@
	# vae-decoder lib generation completed

${DLCBUILDDIR}/vae_decoder_qnn_context_binary_generator.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/vae_decoder_qnn_model_generator.stamp
	# VAE-DECODER lib generation started
	$(DOCKER_RUN_QNN) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libvae_decoder.so \
			--binary_file /output/vae_decoder.serialized
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion
	cp ${DLCBUILDDIR}/vae_decoder.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion
	touch $@
	# VAE context binary generation completed

${DLCBUILDDIR}/vae_decoder_qnn_context_binary_generator_v75.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/vae_decoder_qnn_model_generator.stamp
	# VAE-DECODER lib generation started
	$(DOCKER_RUN_QNN_VAE_DECODER) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libvae_decoder.so \
			--binary_file /output/vae_decoder.serialized \
			--config_file /models/mcp_config_v75.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v75
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v75
	cp ${DLCBUILDDIR}/vae_decoder.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v75/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v75
	touch $@
	# VAE context binary generation completed for v75

${DLCBUILDDIR}/vae_decoder_qnn_context_binary_generator_v79.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/vae_decoder_qnn_model_generator.stamp
	# VAE-DECODER lib generation started
	$(DOCKER_RUN_QNN_VAE_DECODER) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libvae_decoder.so \
			--binary_file /output/vae_decoder.serialized \
			--config_file /models/mcp_config_v79.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v79
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v79
	cp ${DLCBUILDDIR}/vae_decoder.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v79/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v79
	touch $@
	# VAE context binary generation completed for v79

${DLCBUILDDIR}/vae_decoder_qnn_context_binary_generator_v73_sd8sG4.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/vae_decoder_qnn_model_generator.stamp
	# VAE-DECODER lib generation started
	$(DOCKER_RUN_QNN_VAE_DECODER) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libvae_decoder.so \
			--binary_file /output/vae_decoder.serialized \
			--config_file /models/mcp_config_v73_sd8sG4.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd8sG4
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd8sG4
	cp ${DLCBUILDDIR}/vae_decoder.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd8sG4/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd8sG4
	# VAE context binary generation completed for v73 sd8sG4

${DLCBUILDDIR}/vae_decoder_qnn_context_binary_generator_v73_sd7G4.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/vae_decoder_qnn_model_generator.stamp
	# VAE-DECODER lib generation started
	$(DOCKER_RUN_QNN_VAE_DECODER) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libvae_decoder.so \
			--binary_file /output/vae_decoder.serialized \
			--config_file /models/mcp_config_v73_sd7G4.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd7G4
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd7G4
	cp ${DLCBUILDDIR}/vae_decoder.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd7G4/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd7G4
	# VAE context binary generation completed for v73 sd7G4

${DLCBUILDDIR}/vae_decoder_qnn_context_binary_generator_v81_sd8eliteG5.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/vae_decoder_qnn_model_generator.stamp
	# VAE-DECODER lib generation started
	$(DOCKER_RUN_QNN_VAE_DECODER) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libvae_decoder.so \
			--binary_file /output/vae_decoder.serialized \
			--config_file /models/mcp_config_v81_sd8eliteG5.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8eliteG5
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8eliteG5
	cp ${DLCBUILDDIR}/vae_decoder.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8eliteG5/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8eliteG5
	touch $@
	# VAE context binary generation completed for v81 sd8eliteG5

${DLCBUILDDIR}/vae_decoder_qnn_context_binary_generator_v81_sd8G5.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/vae_decoder_qnn_model_generator.stamp
	# VAE-DECODER lib generation started
	$(DOCKER_RUN_QNN_VAE_DECODER) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libvae_decoder.so \
			--binary_file /output/vae_decoder.serialized \
			--config_file /models/mcp_config_v81_sd8G5.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8G5
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8G5
	cp ${DLCBUILDDIR}/vae_decoder.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8G5/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8G5
	touch $@
	# VAE context binary generation completed for v81 sd8G5

${DLCBUILDDIR}/unet_qnn.cpp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# UNET conversion and quantization
    # cpp & bin files
	$(DOCKER_RUN_QNN) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-onnx-converter \
			--input_network /stable_diffusion/unet/unet_onnx_batch_1/unet.onnx \
			-l input_3 NONTRIVIAL \
			--input_list /stable_diffusion/unet/stable_diffusion_models/unet_onnx/unet_input_list.txt \
			--act_bitwidth 16 \
			--bias_bitwidth 32 \
			--quantization_overrides /stable_diffusion/unet/unet_onnx_batch_1/unet.encodings \
			--output_path /output/unet.cpp
	touch $@

${DLCBUILDDIR}/unet_qnn_model_generator.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/unet_qnn.cpp
	# UNET lib generation started
	$(DOCKER_RUN_QNN) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-model-lib-generator \
			-c /output/unet.cpp \
			-b /output/unet.bin \
			-o /output/model_libs \
			-t x86_64-linux-clang \
			-t aarch64-android
	touch $@
	# UNET lib generation completed

${DLCBUILDDIR}/unet_qnn_context_binary_generator.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/unet_qnn_model_generator.stamp
	# UNET context binary generation started
	$(DOCKER_RUN_QNN) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libunet.so \
			--binary_file /output/unet.serialized
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion
	cp ${DLCBUILDDIR}/unet.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion
	touch $@
	# UNET context binary generation completed

${DLCBUILDDIR}/unet_qnn_context_binary_generator_v75.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/unet_qnn_model_generator.stamp
	# UNET context binary generation started
	$(DOCKER_RUN_QNN_UNET) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libunet.so \
			--binary_file /output/unet.serialized \
			--config_file /models/mcp_config_v75.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v75
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v75
	cp ${DLCBUILDDIR}/unet.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v75/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v75
	touch $@
	# UNET context binary generation completed for v75

${DLCBUILDDIR}/unet_qnn_context_binary_generator_v79.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/unet_qnn_model_generator.stamp
	# UNET context binary generation started
	$(DOCKER_RUN_QNN_UNET) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libunet.so \
			--binary_file /output/unet.serialized \
			--config_file /models/mcp_config_v79.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v79
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v79
	cp ${DLCBUILDDIR}/unet.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v79/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v79
	touch $@
	# UNET context binary generation completed for v79

${DLCBUILDDIR}/unet_qnn_context_binary_generator_v73_sd8sG4.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/unet_qnn_model_generator.stamp
	# UNET context binary generation started
	$(DOCKER_RUN_QNN_UNET) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libunet.so \
			--binary_file /output/unet.serialized \
			--config_file /models/mcp_config_v73_sd8sG4.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd8sG4
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd8sG4
	cp ${DLCBUILDDIR}/unet.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd8sG4/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd8sG4
	# UNET context binary generation completed for v73 sd8sG4

${DLCBUILDDIR}/unet_qnn_context_binary_generator_v73_sd7G4.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/unet_qnn_model_generator.stamp
	# UNET context binary generation started
	$(DOCKER_RUN_QNN_UNET) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libunet.so \
			--binary_file /output/unet.serialized \
			--config_file /models/mcp_config_v73_sd7G4.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd7G4
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd7G4
	cp ${DLCBUILDDIR}/unet.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd7G4/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v73_sd7G4
	# UNET context binary generation completed for v73 sd7G4

${DLCBUILDDIR}/unet_qnn_context_binary_generator_v81_sd8eliteG5.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/unet_qnn_model_generator.stamp
	# UNET context binary generation started
	$(DOCKER_RUN_QNN_UNET) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libunet.so \
			--binary_file /output/unet.serialized \
			--config_file /models/mcp_config_v81_sd8eliteG5.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8eliteG5
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8eliteG5
	cp ${DLCBUILDDIR}/unet.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8eliteG5/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8eliteG5
	touch $@
	# UNET context binary generation completed for v81 sd8eliteG5

${DLCBUILDDIR}/unet_qnn_context_binary_generator_v81_sd8G5.stamp: \
		$(DOCKER_STAMP) \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/unet_qnn_model_generator.stamp
	# UNET context binary generation started
	$(DOCKER_RUN_QNN_UNET) \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libunet.so \
			--binary_file /output/unet.serialized \
			--config_file /models/mcp_config_v81_sd8G5.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8G5
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8G5
	cp ${DLCBUILDDIR}/unet.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8G5/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion_v81_sd8G5
	touch $@
	# UNET context binary generation completed for v81 sd8G5


####################################################################################
# CALIBRATION / QUANTIZATION
####################################################################################

# OpenSR calibration data preprocessing
${DLCBUILDDIR}/snusr_calibration_list.txt: \
			$(DOCKER_STAMP) \
			${DATASETS}/output/state/snusr_calibration.stamp
	# Preparing SNUSR dataset
	mkdir -p ${DATASETS}/output/snusr/_raws
	$(DOCKER_RUN_MOBILE_RESIZE_SNUSR) \
	python3 $(SNUSR_UTIL_DIR)/rgb8_to_raw.py $(SNUSR_RAW_DATA_DIR)/ $(SNUSR_CALIB_DIR)/OpenImages_calibration_rgb8.txt
	rm -rf ${DLCBUILDDIR}/snusr/_raws
	mkdir -p ${DLCBUILDDIR}/snusr
	mv ${DATASETS}/output/snusr/_raws ${DLCBUILDDIR}/snusr/
	touch ${DLCBUILDDIR}/snusr/snusr_calibration_list.txt
	ls ${DLCBUILDDIR}/snusr/_raws | sed "s!^!_raws/!" > ${DLCBUILDDIR}/snusr/snusr_calibration_list.txt
	touch $@

# ADE20K calibration data preprocessing
${DLCBUILDDIR}/ade20k/state/resized.stamp: \
		${DATASETS_OUT}/state/calibration.stamp \
		$(DOCKER_MOBILE_STAMP)
	# Resizing ADE20K calibration data
	rm -rf ${DLCBUILDDIR}/ade20k/resized
	mkdir -p ${DLCBUILDDIR}/ade20k/resized
	$(DOCKER_RUN_MOBILE_RESIZE_ADE20K) \
	$(ADE20K_RESIZE_CMD) $(ADE20K_INPUT_DIR)/ADEChallengeData2016/ $(ADE20K_OUT_DIR)/resized $(ADE20K_UTIL2_DIR)/ade20k_calibration_files.txt
	mkdir -p ${DLCBUILDDIR}/ade20k/state
	touch $@

${DLCBUILDDIR}/ade20k/state/quantdata.stamp: \
		${DLCBUILDDIR}/ade20k/state/resized.stamp \
		$(DOCKER_MOBILE_STAMP)
	# Generating ADE20K quantization data for SNPE
	rm -rf ${DLCBUILDDIR}/ade20k/resized_raw
	mkdir -p ${DLCBUILDDIR}/ade20k/resized_raw
	$(DOCKER_RUN_MOBILE_RESIZE_ADE20K) \
	python3 $(ADE20K_COMMON_UTIL_DIR)/jpg_to_raw.py $(ADE20K_BUILD_DIR)/resized
	rm -rf ${DLCBUILDDIR}/ade20k/quantdata
	mv ${DLCBUILDDIR}/ade20k/resized_raw ${DLCBUILDDIR}/ade20k/quantdata
	touch $@

# Imagenet 384x384 calibration data preprocessing
${DLCBUILDDIR}/imagenet/state/resized_384.stamp: \
		${DATASETS_OUT}/state/calibration.stamp \
		$(DOCKER_MOBILE_STAMP)
	# Scaling Imagenet images to 384x384
	rm -rf ${DLCBUILDDIR}/imagenet/resized_384
	mkdir -p ${DLCBUILDDIR}/imagenet/resized_384
	$(DOCKER_RUN_SNPE_IMAGENET_RESIZE) \
	/bin/bash -c "python3 $(UTIL_DIR)/imagenet/Resize384.py $(IMAGENET_CALIB_DIR)/images $(SNPE_OUT)/imagenet/resized_384"
	mkdir -p ${DLCBUILDDIR}/imagenet/state
	touch $@

${DLCBUILDDIR}/imagenet/state/quantdata_384.stamp: \
		${DLCBUILDDIR}/imagenet/state/resized_384.stamp \
		$(DOCKER_MOBILE_STAMP)
	# Generating Imagenet quantization data for SNPE
	rm -rf ${DLCBUILDDIR}/imagenet/resized_384_raw
	mkdir -p ${DLCBUILDDIR}/imagenet/resized_384_raw
	$(DOCKER_RUN_MOBILE_QUANTDATA_384) \
	/bin/bash -c "python3 $(UTIL_DIR)/common/jpg_to_raw.py $(IMAGENET_BUILD_DIR)/resized_384"
	rm -rf ${DLCBUILDDIR}/imagenet/quantdata_384
	mv ${DLCBUILDDIR}/imagenet/resized_384_raw ${DLCBUILDDIR}/imagenet/quantdata_384
	touch $@

# Coco calibration data preprocessing
${DLCBUILDDIR}/coco/state/resized.stamp: \
		${DATASETS_OUT}/state/calibration.stamp \
		$(DOCKER_MOBILE_STAMP)
	# Scaling Coco images to 320x320
	rm -rf ${DLCBUILDDIR}/coco/resized
	mkdir -p ${DLCBUILDDIR}/coco/resized
	$(DOCKER_RUN_MOBILE_RESIZE) \
		$(COCO_RESIZE_CMD)
	mkdir -p ${DLCBUILDDIR}/coco/state
	touch $@

# Create the raw files used by SNPE for calibration/quantization
${DLCBUILDDIR}/coco/state/quantdata.stamp: \
		${DLCBUILDDIR}/coco/state/resized.stamp \
		$(DOCKER_MOBILE_STAMP)
	# Generating Coco quantization data for SNPE
	rm -rf ${DLCBUILDDIR}/coco/resized_raw
	mkdir -p ${DLCBUILDDIR}/coco/resized_raw
	$(DOCKER_RUN_MOBILE_RESIZE) \
	python3 $(COMMON_UTIL_DIR)/jpg_to_raw.py $(COCO_OUT_DIR)/resized
	rm -rf ${DLCBUILDDIR}/coco/quantdata
	mv ${DLCBUILDDIR}/coco/resized_raw ${DLCBUILDDIR}/coco/quantdata
	touch $@

${DATASETS}/output/state/squad_calibration.stamp:
	(cd ${DATASETS} && make squad_calibration)

${DATASETS}/output/state/snusr_calibration.stamp:
	(cd ${DATASETS} && make snusr_calibration)

####################################################################################
# DLC Info
####################################################################################
gen-htp-dlc-info: \
		$(DOCKER_MOBILE_STAMP) \
		htp-dlc
	$(DOCKER_RUN_SNPE) \
		-v ${DLCBUILDDIR}:/dlc \
		/bin/bash -c '\
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobilenet_v4_htp.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/ssd_mobiledet_qat_htp.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobilenet_v4_htp_batched_4.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobilenet_v4_htp_batched_8_sm8850.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobile_mosaic_htp.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/snusr_htp.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobilebert_quantized_htp.dlc'

####################################################################################
# Clean
####################################################################################
clean:
	rm -rf ${BUILDDIR}/DLC
