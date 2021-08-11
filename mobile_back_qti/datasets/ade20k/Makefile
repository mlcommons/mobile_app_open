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

ADE20K_OUT=${BUILDDIR}/ade20k/ade20k_out
DOWNLOADS=${BUILDDIR}/ade20k/downloads

all: ${ADE20K_OUT}/ade20k_image_list.txt

include ../../make/docker.mk

# Get calibration data
${DOWNLOADS}/ADE20K_2016.zip:
	@echo "Fetching ADE20K_2016.zip"
	@mkdir -p ${DOWNLOADS}
	@echo rm -f ${DOWNLOADS}/zip.part??
	python3 ${TOPDIR}/utils/chunkdownload.py https://data.deepai.org/ADE20K_2016.zip 4051732024 ${DOWNLOADS}/zip
	@cat ${DOWNLOADS}/zip.part?? > $@

${ADE20K_OUT}/ADE20K_2016_07_26/.stamp: ${DOWNLOADS}/ADE20K_2016.zip
	@rm -rf ${ADE20K_OUT}/ADE20K_2016_07_26
	@mkdir -p ${ADE20K_OUT}/ADE20K_2016_07_26
	@echo "Extracting ADE20K_2016.zip"
	@(cd ${ADE20K_OUT} && unzip -q ${DOWNLOADS}/ADE20K_2016.zip)
	@touch $@

# Used 41a1e1d6200cebaa5cfd73ce7110998480a2f43b for testing
${DOWNLOADS}/v2.3.0.zip:
	@echo "Fetching tensorflow/models"
	@mkdir -p ${DOWNLOADS}
	@(cd ${DOWNLOADS} && wget https://github.com/tensorflow/models/archive/v2.3.0.zip)

${DOWNLOADS}/models-2.3.0/.stamp: ${DOWNLOADS}/v2.3.0.zip
	@(cd ${DOWNLOADS} && unzip v2.3.0.zip)
	@touch $@

${ADE20K_OUT}/quantdata.stamp: \
		${DOWNLOADS}/models-2.3.0/.stamp \
		${ADE20K_OUT}/ADE20K_2016_07_26/.stamp \
		${BUILDDIR}/mlperf_mobile_docker_1_0.stamp
	@rm -rf ${ADE20K_OUT}/quantdata
	@mkdir -p ${ADE20K_OUT}/quantdata
	@docker run \
		-e PYTHONPATH=/downloads/models-2.3.0/research/slim:/downloads/models-2.3.0/research/deeplab:/downloads/models-2.3.0/research \
		-v $(CURDIR):/ade20k \
		-v ${ADE20K_OUT}:/ade20k_out \
	  	-v ${DOWNLOADS}:/downloads \
	  	-u ${USERID}:${GROUPID} \
	  	mlcommons/mlperf_mobile:1.0 \
		python3 /ade20k/resize_512_ade20k_calibration.py /ade20k_out/ADE20K_2016_07_26 /ade20k_out/quantdata /ade20k/ade20k_calibration_files.txt
	@touch $@

${ADE20K_OUT}/ade20k_image_list.txt: ${ADE20K_OUT}/quantdata.stamp
	@echo "Generating ADE20k quantization images for SNPE"
	@mkdir -p ${ADE20K_OUT}/quantdata_raw
	@docker run \
		-v $(CURDIR)/..:/datasets \
		-v ${ADE20K_OUT}:/ade20k_out \
	  	-u ${USERID}:${GROUPID} \
	  	mlcommons/mlperf_mobile:1.0 \
		/bin/bash -c "python3 /datasets/jpg_to_raw.py /ade20k_out/quantdata"
	@(cd ${ADE20K_OUT}/quantdata_raw && ls > ../tmp_ade20k_image_list.txt)
	@awk '{print "quantdata_raw/"$$0}' ${ADE20K_OUT}/tmp_ade20k_image_list.txt > $@
	@rm ${ADE20K_OUT}/tmp_ade20k_image_list.txt

clean:
	@rm -rf ${ADE20K_OUT}

clean_downloads:
	@rm -rf ${DOWNLOADS}

