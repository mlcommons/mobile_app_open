# Copyright (c) The MLPerf Authors. All Rights Reserved.
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

# Get absolute path of makefile
MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))

# Specify directories
ROOTDIR := $(abspath $(shell dirname ${MK_PATH})/..)
BUILDDIR := ${ROOTDIR}/datasets

.PHONY: clean clean-ade20k clean-coco clean-imagenet clean-squad doc

USERID = $(shell id -u)
GROUPID = $(shell id -g)

# Ade20k calibration vars
A_SRC_DIR := ${BUILDDIR}/downloads/ADEChallengeData2016/images/training
A_FILES = $(shell cat ${BUILDDIR}/util/ade20k/ade20k_calibration_files.txt)
A_SRC = $(addprefix $(A_SRC_DIR)/,$(A_FILES))
A_DEST_DIR := ${BUILDDIR}/output/calibration_set/ade20k/images

A_TARGS := $(patsubst $(A_SRC_DIR)/%.jpg,$(A_DEST_DIR)/%.jpg,$(A_SRC))

# Coco calibration vars
C_SRC_DIR := ${BUILDDIR}/downloads/val2014
C_FILES = $(shell cat ${BUILDDIR}/util/coco/coco_calibration_files.txt)
C_SRC = $(addprefix $(C_SRC_DIR)/COCO_val2014_,$(C_FILES))
C_DEST_DIR := ${BUILDDIR}/output/calibration_set/coco/images

C_TARGS := $(patsubst $(C_SRC_DIR)/%.jpg,$(C_DEST_DIR)/%.jpg,$(C_SRC))

all: ${BUILDDIR}/datasets.stamp

print-% : ; @echo $* = $($*)

${BUILDDIR}/datasets.stamp: ${BUILDDIR}/ade20k.stamp ${BUILDDIR}/coco.stamp \
			${BUILDDIR}/imagenet.stamp ${BUILDDIR}/squad.stamp
	@touch $@

# Ade20k
${BUILDDIR}/ade20k.stamp: ${BUILDDIR}/ade20k_helper.stamp
	@touch $@

${BUILDDIR}/ade20k_helper.stamp: ${BUILDDIR}/downloads/ade20k.zip.stamp \
		${BUILDDIR}/downloads/v2.3.0.zip.stamp ${BUILDDIR}/mlperf_docker_1.1.stamp \
		${BUILDDIR}/ade20k_calibration.stamp
	@echo "Generating ADE20K test set"
	@rm -rf ${BUILDDIR}/output/mlperf_datasets/ade20k/images
	@rm -rf ${BUILDDIR}/output/mlperf_datasets/ade20k/annotations
	@mkdir -p ${BUILDDIR}/output/mlperf_datasets/ade20k/images
	@mkdir -p ${BUILDDIR}/output/mlperf_datasets/ade20k/annotations
	@docker run \
		-e PYTHONPATH=/workspace/downloads/models-2.3.0/research/slim:/workspace/downloads/models-2.3.0/research/deeplab:/workspace/downloads/models-2.3.0/research \
		-v $(BUILDDIR):/workspace/ \
	  	-u ${USERID}:${GROUPID} \
	  	mlperf_mobile:1.1 \
		python3 /workspace/util/ade20k/resize_512_ade20k.py /workspace/downloads/ADEChallengeData2016/ \
			/workspace/output/mlperf_datasets/ade20k/
	@touch $@

${BUILDDIR}/downloads/ade20k.zip.stamp: ${BUILDDIR}/downloads/ADEChallengeData2016.zip
	@(unzip $< -d ${BUILDDIR}/downloads/)
	@touch $@

${BUILDDIR}/downloads/v2.3.0.zip.stamp: ${BUILDDIR}/downloads/v2.3.0.zip
	@(unzip $< -d ${BUILDDIR}/downloads/)
	@touch $@

${BUILDDIR}/downloads/ADEChallengeData2016.zip: 
	@echo "Fetching ADEChallengeData2016.zip"
	@mkdir -p ${BUILDDIR}/downloads/
	@(curl -o $@ -L http://data.csail.mit.edu/places/ADEchallenge/ADEChallengeData2016.zip)

${BUILDDIR}/downloads/v2.3.0.zip: 
	@echo "Fetching tensorflow/models"
	@mkdir -p ${BUILDDIR}/downloads/
	@(curl -o $@ -L https://github.com/tensorflow/models/archive/v2.3.0.zip)

${BUILDDIR}/mlperf_docker_1.1.stamp:
	@echo "Building mlperf_mobile docker"
	@docker image build -t mlperf_mobile:1.1 $(dir $(abspath ${ROOTDIR}/android/docker/mlperf_mobile/Dockerfile))
	@touch $@

${A_DEST_DIR}/%.jpg: ${A_SRC_DIR}/%.jpg 
	@(cp $< $@)

${BUILDDIR}/ade20k_calibration.stamp: ${BUILDDIR}/ade20k_calibration_helper.stamp $(A_TARGS)
	@touch $@

${BUILDDIR}/ade20k_calibration_helper.stamp:
	@echo "Generating ADE20K calibration set"
	@rm -rf ${BUILDDIR}/output/calibration_set/ade20k/images
	@mkdir -p ${BUILDDIR}/output/calibration_set/ade20k/images
	@touch $@

# Coco
${BUILDDIR}/coco.stamp: ${BUILDDIR}/coco_helper.stamp ${BUILDDIR}/downloads/val2017.zip.stamp
	@touch $@

${BUILDDIR}/coco_helper.stamp: ${BUILDDIR}/downloads/val2014.zip.stamp \
		${BUILDDIR}/downloads/coco_cal_images_list.txt ${BUILDDIR}/coco_calibration.stamp 
	@touch $@

${BUILDDIR}/downloads/coco_cal_images_list.txt:
	@mkdir -p ${BUILDDIR}/downloads/
	@curl -o $@ -L https://raw.githubusercontent.com/mlperf/inference/master/calibration/COCO/coco_cal_images_list.txt 

${BUILDDIR}/downloads/val2017.zip.stamp: ${BUILDDIR}/downloads/val2017.zip
	@echo "Generating COCO test set"
	@(unzip $< -d ${BUILDDIR}/downloads/ && \
		mv ${BUILDDIR}/downloads/val2017/* ${BUILDDIR}/output/mlperf_datasets/coco/img/)
	@touch $@

${BUILDDIR}/downloads/val2017.zip:
	@echo "Fetching COCO val2017.zip"
	@mkdir -p ${BUILDDIR}/downloads/
	@(curl -o $@ -L http://images.cocodataset.org/zips/val2017.zip)
	
${BUILDDIR}/downloads/val2014.zip.stamp: ${BUILDDIR}/downloads/val2014.zip
	@(unzip $< -d ${BUILDDIR}/downloads/)
	@touch $@

${BUILDDIR}/downloads/val2014.zip:
	@echo "Fetching COCO val2014.zip"
	@mkdir -p ${BUILDDIR}/downloads/
	@(curl -o $@ -L http://images.cocodataset.org/zips/val2014.zip)


${C_DEST_DIR}/%.jpg: ${C_SRC_DIR}/%.jpg 
	@(cp $< $@)

${BUILDDIR}/coco_calibration.stamp: ${BUILDDIR}/coco_calibration_helper.stamp $(C_TARGS)
	@touch $@

${BUILDDIR}/coco_calibration_helper.stamp:
	@echo "Generating COCO calibration set"
	@rm -rf ${BUILDDIR}/output/calibration_set/coco/images
	@mkdir -p ${BUILDDIR}/output/calibration_set/coco/images
	@touch $@

# TODO 
${BUILDDIR}/imagenet.stamp:
	@echo "ImageNet not implemented"
	@touch $@

${BUILDDIR}/squad.stamp:
	@echo "Squad not implemented"
	@touch $@


clean:

clean-ade20k:

clean-coco:

clean-imagenet:

clear-squad:

