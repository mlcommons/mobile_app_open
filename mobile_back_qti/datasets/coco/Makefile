# Copyright (c) 2020-2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

include ../../make/builddir.mk

COCO_OUT=${BUILDDIR}/coco/coco_out
DOWNLOADS=${BUILDDIR}/coco/downloads

all: ${COCO_OUT}/.stamp

include ../../make/docker.mk

${DOWNLOADS}/val2014.zip:
	@mkdir -p ${DOWNLOADS}
	@echo "Fetching Coco val2014.zip"
	@echo rm -f ${DOWNLOADS}/zip.part??
	@python3 ${TOPDIR}/utils/chunkdownload.py http://images.cocodataset.org/zips/val2014.zip 6645013297 ${DOWNLOADS}/zip
	@cat ${DOWNLOADS}/zip.part?? > $@

# 2014 images are used for calibration
${COCO_OUT}/orig/val2014.stamp: ${DOWNLOADS}/val2014.zip
	@rm -rf ${COCO_OUT}/orig/val2014
	@mkdir -p ${COCO_OUT}/orig
	@(cd ${COCO_OUT}/orig && unzip -q ${DOWNLOADS}/val2014.zip)
	@touch $@

${DOWNLOADS}/coco_cal_images_list.txt:
	@mkdir -p ${DOWNLOADS}
	@curl -o $@ -L https://raw.githubusercontent.com/mlperf/inference/master/calibration/COCO/coco_cal_images_list.txt

# Create scaled images used for calibration
# ${COCO_OUT}/calibration/val2014 has the rescaled calibration images
${COCO_OUT}/calibration/val2014.stamp: \
		${DOWNLOADS}/coco_cal_images_list.txt \
		${COCO_OUT}/orig/val2014.stamp \
		${BUILDDIR}/mlperf_mobile_docker_1_0.stamp
	@echo "Scaling Coco images to 320x320"
	@rm -rf ${COCO_OUT}/calibration
	@mkdir -p ${COCO_OUT}/calibration
	@docker run \
	  -v ${COCO_OUT}:/coco-out \
	  -v $(CURDIR):/coco \
	  -v ${DOWNLOADS}:/downloads \
	  -u ${USERID}:${GROUPID} \
	  mlcommons/mlperf_mobile:1.0 \
	    python3 /coco/upscale_coco.py --inputs /coco-out/orig --outputs /coco-out/calibration --size 320 320
	@touch $@

# Create the raw files used by SNPE for calibration/quantization
${COCO_OUT}/quantdata.stamp: \
		${COCO_OUT}/calibration/val2014.stamp \
		${BUILDDIR}/mlperf_mobile_docker_1_0.stamp
	@echo "Generating coco quantization data for SNPE"
	@rm -rf ${COCO_OUT}/calibration/val2014_raw
	@mkdir -p ${COCO_OUT}/calibration/val2014_raw
	@docker run \
		-v $(CURDIR)/..:/datasets \
		-v ${COCO_OUT}:/coco_out \
	  	-u ${USERID}:${GROUPID} \
		mlcommons/mlperf_mobile:1.0 \
		/bin/bash -c "python3 /datasets/jpg_to_raw.py /coco_out/calibration/val2014"
	@rm -rf ${COCO_OUT}/quantdata
	@mv ${COCO_OUT}/calibration/val2014_raw ${COCO_OUT}/quantdata
	@(cd ${COCO_OUT}/quantdata && ls > ../tmp_coco_image_list.txt)
	@(cd ${COCO_OUT} && awk '{print "quantdata/"$$0}' tmp_coco_image_list.txt > coco_image_list.txt)
	@rm -f ${COCO_OUT}/tmp_coco_image_list.txt
	@touch $@

${COCO_OUT}/.stamp: ${COCO_OUT}/quantdata.stamp
	touch $@

clean:
	@rm -rf ${COCO_OUT}

clean_downloads:
	@rm -rf ${DOWNLOADS}
