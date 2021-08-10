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

IMAGNET_OUT=${BUILDDIR}/imagenet/imagenet_out
DOWNLOADS=${BUILDDIR}/imagenet/downloads

all: ${IMAGNET_OUT}/.stamp

include ../../make/docker.mk

${DOWNLOADS}/LSVRC2012_img_val.tar:
	@echo "ERROR: Download the imagenet validation data (LSVRC2012_img_val.tar) and place it in this directory" && false

# TODO See if option 1 or option 2 generates better accuracy
${DOWNLOADS}/cal_image_list_option_1.txt:
	curl -o $@ -L https://raw.githubusercontent.com/mlperf/inference/master/calibration/ImageNet/cal_image_list_option_1.txt

${DOWNLOADS}/cal_image_list_option_2.txt:
	curl -o $@ -L https://raw.githubusercontent.com/mlperf/inference/master/calibration/ImageNet/cal_image_list_option_2.txt

${IMAGNET_OUT}/imagenet.stamp: \
		${DOWNLOADS}/LSVRC2012_img_val.tar \
		${DOWNLOADS}/cal_image_list_option_1.txt
	@echo "Extracting Imagenet images for quantization"
	@rm -rf ${IMAGNET_OUT}/imagenet ${IMAGNET_OUT}/orig
	@mkdir -p ${IMAGNET_OUT}/imagenet
	@mkdir -p ${IMAGNET_OUT}/orig
	@(cd ${IMAGNET_OUT}/orig && tar xf ${DOWNLOADS}/LSVRC2012_img_val.tar)
	@for f in `cat ${DOWNLOADS}/cal_image_list_option_1.txt` ; do \
                cp ${IMAGNET_OUT}/orig/$${f} ${IMAGNET_OUT}/imagenet; \
        done
	@touch $@

${IMAGNET_OUT}/resized.stamp: \
		${IMAGNET_OUT}/imagenet.stamp \
		${BUILDDIR}/mlperf_mobile_docker_1_0.stamp
	@echo "Scaling Imagenet images to 224x224"
	@mkdir -p ${IMAGNET_OUT}/resized
	@docker run \
	  -v $(CURDIR):/imagenet \
	  -v ${IMAGNET_OUT}:/imagenet_out \
	  -u ${USERID}:${GROUPID} \
	  mlcommons/mlperf_mobile:1.0 \
	  /bin/bash -c "python3 /imagenet/Resize224.py /imagenet_out/imagenet /imagenet_out/resized"
	@touch $@

${IMAGNET_OUT}/quantdata.stamp: \
		${IMAGNET_OUT}/resized.stamp \
		${BUILDDIR}/mlperf_mobile_docker_1_0.stamp
	@echo "Generating Imagenet quantization data for SNPE"
	@mkdir -p ${IMAGNET_OUT}/resized_raw
	@docker run \
		-v $(CURDIR)/..:/datasets \
		-v ${IMAGNET_OUT}:/imagenet_out \
		-u ${USERID}:${GROUPID} \
		mlcommons/mlperf_mobile:1.0 \
		/bin/bash -c "python3 /datasets/jpg_to_raw.py /imagenet_out/resized"
	@rm -rf ${IMAGNET_OUT}/quantdata
	@mv ${IMAGNET_OUT}/resized_raw ${IMAGNET_OUT}/quantdata
	@(cd ${IMAGNET_OUT}/quantdata && ls > ../tmp_imagenet_image_list.txt)
	@(cd ${IMAGNET_OUT} && awk '{print "quantdata/"$$0}' tmp_imagenet_image_list.txt > imagenet_image_list.txt)
	@rm -f ${IMAGNET_OUT}/tmp_imagenet_image_list.txt
	@touch $@

${IMAGNET_OUT}/.stamp: ${IMAGNET_OUT}/imagenet.stamp ${IMAGNET_OUT}/quantdata.stamp
	@touch $@

clean:
	@rm -rf ${IMAGNET_OUT}

clean_downloads:
	@echo "Warning: Not removing imagenet dataset"
