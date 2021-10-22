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

SQUAD_OUT=${BUILDDIR}/SQUAD/SQUAD_out
DOWNLOADS=${BUILDDIR}/SQUAD/downloads

all: ${SQUAD_OUT}/.stamp

include ../../make/docker.mk

${DOWNLOADS}/mobilebert.tar.gz:
	@mkdir -p ${DOWNLOADS}
	@echo "Fetching SQUAD vocab file"
	@curl -o $@ -L https://storage.googleapis.com/cloud-tpu-checkpoints/mobilebert/uncased_L-24_H-128_B-512_A-4_F-4_OPT.tar.gz
	@cd ${DOWNLOADS} && tar -xvf $@
	@chmod 777 -R ${DOWNLOADS}/mobilebert
	@cd ${DOWNLOADS}/mobilebert/
	@echo "Fetching SQUAD dataset dev-v1.1.json"
	@curl -o ${DOWNLOADS}/mobilebert/dev-v1.1.json -L https://rajpurkar.github.io/SQuAD-explorer/dataset/dev-v1.1.json


${SQUAD_OUT}/quantdata.stamp: \
		${DOWNLOADS}/mobilebert.tar.gz \
		${BUILDDIR}/mlperf_mobile_docker_1_0.stamp
	@echo "Generating squad quantization data for SNPE"
	@rm -rf ${SQUAD_OUT}/sqaud_raw
	@mkdir -p ${SQUAD_OUT}/squad_raw
	@touch ${SQUAD_OUT}/squad_raw/input_list.txt
	@chmod 777 ${SQUAD_OUT}/squad_raw/input_list.txt
	@docker run \
		-v $(CURDIR)/squad_tools/:/squad_tools \
		-v ${SQUAD_OUT}:/squad_out \
		-v ${DOWNLOADS}/mobilebert/:/mobilebert \
	  	-u ${USERID}:${GROUPID} \
		mlcommons/mlperf_mobile:1.0 \
		/bin/bash -c "python3 /squad_tools/convert.py --test_file /mobilebert/dev-v1.1.json --vocab_file /mobilebert/vocab.txt --output_dir /squad_out/squad_raw --input_list_dir /squad_out/squad_raw --num_samples 400"
	@touch $@

${SQUAD_OUT}/.stamp: ${SQUAD_OUT}/quantdata.stamp
	touch $@

clean:
	@rm -rf ${SQUAD_OUT}

clean_downloads:
	@rm -rf ${DOWNLOADS}
