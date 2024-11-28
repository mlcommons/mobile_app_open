# Copyright (c) 2020-2021 The MLPerf Authors. All rights reserved.
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

.PHONY: clean clean-ade20k clean-coco clean-imagenet clean-squad

USERID = $(shell id -u)
GROUPID = $(shell id -g)

# Ade20k calibration vars
A_SRC_DIR := ${BUILDDIR}/output/ade20k/ADEChallengeData2016/images/training
A_DEST_DIR := ${BUILDDIR}/output/calibration_set/ade20k/images

# Coco calibration vars
C_SRC_DIR := ${BUILDDIR}/output/coco/val2014
C_DEST_DIR := ${BUILDDIR}/output/calibration_set/coco/images

C_TARGS := $(patsubst $(C_SRC_DIR)/%.jpg,$(C_DEST_DIR)/%.jpg,$(C_SRC))

# SQuAD calibration vars
S_DEST_DIR := ${BUILDDIR}/output/calibration_set/squad

all: ${BUILDDIR}/output/state/datasets.stamp ${BUILDDIR}/output/state/calibration.stamp
print-% : ; @echo $* = $($*)

${BUILDDIR}/output/state/datasets.stamp: ${BUILDDIR}/output/state/ade20k.stamp ${BUILDDIR}/output/state/coco.stamp \
			${BUILDDIR}/output/state/imagenet.stamp ${BUILDDIR}/output/state/squad.stamp
	touch $@

${BUILDDIR}/output/state/calibration.stamp: ${BUILDDIR}/output/state/ade20k_calibration.stamp ${BUILDDIR}/output/state/coco_calibration.stamp \
			${BUILDDIR}/output/state/imagenet_calibration.stamp ${BUILDDIR}/output/state/squad_calibration.stamp ${BUILDDIR}/output/state/snusr_calibration.stamp
	touch $@
	
snusr_calibration : ${BUILDDIR}/output/state/snusr_calibration.stamp

${BUILDDIR}/output/state/snusr_calibration.stamp: ${BUILDDIR}/downloads/snusr/SNUSR.zip
	mkdir -p ${BUILDDIR}/output/snusr
	unzip $< -d ${BUILDDIR}/output/snusr
	mkdir -p ${BUILDDIR}/output/state
	touch $@

${BUILDDIR}/downloads/snusr/SNUSR.zip: 
	# Fetching SNUSR.zip
	mkdir -p ${BUILDDIR}/downloads/snusr
	curl -o $@ -L https://github.com/mlcommons/mobile_models/raw/main/v3_0/datasets/snusr_lr.zip
	touch $@

# Ade20k
${BUILDDIR}/output/state/ade20k.stamp: ${BUILDDIR}/output/state/ade20k_unzip.stamp \
		${BUILDDIR}/output/state/ade20k_v2.3.0.stamp ${BUILDDIR}/output/state/mlperf_docker_1.1.stamp
	# Generating ADE20K test set
	rm -rf ${BUILDDIR}/output/mlperf_datasets/ade20k/images
	rm -rf ${BUILDDIR}/output/mlperf_datasets/ade20k/annotations
	mkdir -p ${BUILDDIR}/output/mlperf_datasets/ade20k/images
	mkdir -p ${BUILDDIR}/output/mlperf_datasets/ade20k/annotations
	docker run \
		-e PYTHONPATH=/workspace/output/ade20k/models-2.3.0/research/slim:/workspace/output/ade20k/models-2.3.0/research/deeplab:/workspace/output/ade20k/models-2.3.0/research \
		-v $(BUILDDIR):/workspace/ \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		python3 /workspace/util/ade20k/resize_512_ade20k.py /workspace/output/ade20k/ADEChallengeData2016/ \
			/workspace/output/mlperf_datasets/ade20k/
	touch $@

${BUILDDIR}/output/state/ade20k_unzip.stamp: ${BUILDDIR}/downloads/ade20k/ADEChallengeData2016.zip
	mkdir -p ${BUILDDIR}/output/ade20k
	unzip $< -d ${BUILDDIR}/output/ade20k
	mkdir -p ${BUILDDIR}/output/state
	touch $@

${BUILDDIR}/output/state/ade20k_v2.3.0.stamp: ${BUILDDIR}/downloads/v2.3.0.zip
	mkdir -p ${BUILDDIR}/output/ade20k
	unzip $< -d ${BUILDDIR}/output/ade20k
	mkdir -p ${BUILDDIR}/output/state
	touch $@

${BUILDDIR}/downloads/ade20k/ADEChallengeData2016.zip: 
	# Fetching ADEChallengeData2016.zip
	mkdir -p ${BUILDDIR}/downloads/ade20k
	curl -o $@ -L http://data.csail.mit.edu/places/ADEchallenge/ADEChallengeData2016.zip

${BUILDDIR}/downloads/v2.3.0.zip: 
	# Fetching tensorflow/models
	mkdir -p ${BUILDDIR}/downloads/
	(curl -o $@ -L https://github.com/tensorflow/models/archive/v2.3.0.zip)

ifeq ($(INTERNAL_DOCKER),1)
include load_from_image.mk
else
include load_from_dockerfile.mk
endif

# No preprocessing is applied to the files
${BUILDDIR}/output/state/ade20k_calibration.stamp: ${BUILDDIR}/output/state/ade20k.stamp
	# Generating ADE20K calibration set
	rm -rf ${A_DEST_DIR}
	mkdir -p ${A_DEST_DIR}
	mkdir -p ${BUILDDIR}/output/state
	for f in $(shell cat ${BUILDDIR}/util/ade20k/ade20k_calibration_files.txt) ; do \
		cp ${A_SRC_DIR}/$${f} ${A_DEST_DIR}; \
	done
	touch $@

# Coco
${BUILDDIR}/output/state/coco.stamp: ${BUILDDIR}/downloads/coco/val2017.zip
	# Generating COCO test set
	mkdir -p ${BUILDDIR}/output/coco 
	unzip $< -d ${BUILDDIR}/output/coco
	mkdir -p ${BUILDDIR}/output/mlperf_datasets/coco/img
	mkdir -p ${BUILDDIR}/output/state
	mv ${BUILDDIR}/output/coco/val2017/* ${BUILDDIR}/output/mlperf_datasets/coco/img/
	touch $@

${BUILDDIR}/downloads/coco/val2017.zip:
	# Fetching Coco val2017.zip
	mkdir -p ${BUILDDIR}/downloads/coco
	rm -f ${BUILDDIR}/downloads/coco/zip17.part??
	python3 ${BUILDDIR}/util/chunkdownload.py http://images.cocodataset.org/zips/val2017.zip 815585330 ${BUILDDIR}/downloads/coco/zip17
	cat ${BUILDDIR}/downloads/coco/zip17.part?? > $@
	rm -f ${BUILDDIR}/downloads/coco/zip17.part??

${BUILDDIR}/output/state/coco_val2014.stamp: ${BUILDDIR}/downloads/coco/val2014.zip
	mkdir -p ${BUILDDIR}/output/coco 
	unzip $< -d ${BUILDDIR}/output/coco
	mkdir -p ${BUILDDIR}/downloads/stamp
	touch $@

${BUILDDIR}/downloads/coco/val2014.zip:
	# Fetching COCO val2014.zip
	mkdir -p ${BUILDDIR}/downloads/coco
	rm -f ${BUILDDIR}/downloads/coco/zip14.part??
	python3 ${BUILDDIR}/util/chunkdownload.py http://images.cocodataset.org/zips/val2014.zip 6645013297 ${BUILDDIR}/downloads/coco/zip14
	cat ${BUILDDIR}/downloads/coco/zip14.part?? > $@
	rm -f ${BUILDDIR}/downloads/coco/zip14.part??

# No preprocessing is applied to the calibration images
${BUILDDIR}/output/state/coco_calibration.stamp: ${BUILDDIR}/output/state/coco_val2014.stamp
	# Generating COCO calibration set
	rm -rf ${C_DEST_DIR}
	mkdir -p ${C_DEST_DIR}
	mkdir -p ${BUILDDIR}/output/state
	for f in $(shell cat ${BUILDDIR}/util/coco/coco_calibration_files.txt) ; do \
		cp ${C_SRC_DIR}/COCO_val2014_$${f} ${C_DEST_DIR}/$${f}; \
	done
	touch $@

# imagenet
${BUILDDIR}/downloads/imagenet/ILSVRC2012_img_val.tar:
	# Manually copy ILSVRC2012_img_val.tar to $@
	false

${BUILDDIR}/downloads/imagenet/cal_image_list_option_1.txt:
	mkdir -p ${BUILDDIR}/downloads/imagenet
	curl -o $@ -L https://github.com/mlcommons/inference/raw/master/calibration/ImageNet/cal_image_list_option_1.txt

${BUILDDIR}/output/state/imagenet.stamp: \
		${BUILDDIR}/downloads/imagenet/ILSVRC2012_img_val.tar \
		${BUILDDIR}/downloads/imagenet/cal_image_list_option_1.txt
	# Extracting Imagenet images
	rm -rf ${BUILDDIR}/output/mlperf_datasets/imagenet ${BUILDDIR}/output/imagenet
	mkdir -p ${BUILDDIR}/output/mlperf_datasets/imagenet/img
	(cd ${BUILDDIR}/output/mlperf_datasets/imagenet/img && tar xf ${BUILDDIR}/downloads/imagenet/ILSVRC2012_img_val.tar)
	touch $@

# No preprocessing is applied to the calibration images
${BUILDDIR}/output/state/imagenet_calibration.stamp: ${BUILDDIR}/output/state/imagenet.stamp
	mkdir -p ${BUILDDIR}/output/calibration_set/imagenet/images
	for f in `cat ${BUILDDIR}/downloads/imagenet/cal_image_list_option_1.txt` ; do \
		cp ${BUILDDIR}/output/mlperf_datasets/imagenet/img/$${f} ${BUILDDIR}/output/calibration_set/imagenet/images; \
	done
	touch $@

# squad
${BUILDDIR}/downloads/mobilebert.tar.gz:
	# Fetching SQuAD vocab file
	mkdir -p ${BUILDDIR}/downloads
	curl -o $@ -L https://storage.googleapis.com/cloud-tpu-checkpoints/mobilebert/uncased_L-24_H-128_B-512_A-4_F-4_OPT.tar.gz

${BUILDDIR}/downloads/mobilebert/.stamp: ${BUILDDIR}/downloads/mobilebert.tar.gz
	(cd ${BUILDDIR}/downloads/ && tar -xvf $<)
	chmod 777 -R ${BUILDDIR}/downloads/mobilebert
	touch $@

${BUILDDIR}/downloads/mobilebert/dev-v1.1.json: ${BUILDDIR}/downloads/mobilebert/.stamp
	# Fetching SQuAD dataset dev-v1.1.json
	curl -o ${BUILDDIR}/downloads/mobilebert/dev-v1.1.json -L https://rajpurkar.github.io/SQuAD-explorer/dataset/dev-v1.1.json

# Calibration data is pre-processed to a consumable format for calibration
squad_calibration: ${BUILDDIR}/output/state/squad_calibration.stamp
${BUILDDIR}/output/state/squad_calibration.stamp: \
		${BUILDDIR}/downloads/mobilebert/dev-v1.1.json \
		${BUILDDIR}/output/state/mlperf_docker_1.1.stamp
	# Generating SQuAD calibration data
	rm -rf ${S_DEST_DIR}
	mkdir -p ${S_DEST_DIR}
	touch ${S_DEST_DIR}/input_list.txt
	chmod 777 ${S_DEST_DIR}/input_list.txt
	docker run \
		-v $(CURDIR)/util/squad/:/squad_tools \
		-v ${S_DEST_DIR}:/squad_out \
		-v ${BUILDDIR}/downloads/mobilebert/:/mobilebert \
	  	-u ${USERID}:${GROUPID} \
	  	mlperf_mobile:1.1 \
		/bin/bash -c "python3 /squad_tools/convert.py --test_file /mobilebert/dev-v1.1.json --vocab_file /mobilebert/vocab.txt --output_dir /squad_out --input_list_dir /squad_out --num_samples 400"
	touch $@


${BUILDDIR}/output/mlperf_datasets/squad/squad_eval_mini.tfrecord:
	mkdir -p ${BUILDDIR}/output/mlperf_datasets/squad
	curl -o $@ -L https://mobile.mlcommons-storage.org/app-resources/datasets/v1_0/squad_eval_mini.tfrecord

${BUILDDIR}/output/mlperf_datasets/squad/squad_groundtruth.tfrecord:
	mkdir -p ${BUILDDIR}/output/mlperf_datasets/squad
	curl -o $@ -L https://mobile.mlcommons-storage.org/app-resources/datasets/v1_0/squad_groundtruth.tfrecord

${BUILDDIR}/output/state/squad.stamp: \
		${BUILDDIR}/output/mlperf_datasets/squad/squad_eval_mini.tfrecord \
		${BUILDDIR}/output/mlperf_datasets/squad/squad_groundtruth.tfrecord


clean:
	rm -rf downloads output

clean-ade20k:
	rm -rf downloads/ade20k output/calibration_set/ade20k output/state/ade20k*.stamp

clean-coco:
	rm -rf downloads/coco output/calibration_set/coco output/state/coco*.stamp

clean-imagenet:
	rm -rf downloads/imagenet output/calibration_set/imagenet output/state/imagenet*.stamp

clear-squad:
	rm -rf downloads/mobilebert ${S_DEST_DIR} output/state/squad*.stamp

