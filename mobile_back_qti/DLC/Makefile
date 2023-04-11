# Copyright (c) 2020-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

.PHONY: htp-quantized-dlc htp-dlc dependencies gen-hta-dlc-info gen-htp-dlc-info clean

DLCBUILDDIR=${BUILDDIR}/DLC
MODEL_BASE_PATH=${DLCBUILDDIR}/mobile
MOBILENETEDGETPU_MODEL_PATH=${MODEL_BASE_PATH}/vision/mobilenet/models_and_code/checkpoints/float
MOBILEBERT_MODEL_PATH=${MODEL_BASE_PATH}/language/bert/models_and_code/checkpoints/quant/
MOBILEMOSAIC_MODEL_PATH=${MODEL_BASE_PATH}/vision/mosaic/models_and_checkpoints/R4/
SNUSR_MODEL_PATH = ${MODEL_BASE_PATH}/vision/edsr/models_and_checkpoints/checkpoints/f32b5/ckpt_qat/
SNUSR_CALIBRATION_PATH=${MODEL_BASE_PATH}/calibration/OpenImages
MLPERF_MODELS_PATH = ${DLCBUILDDIR}/mlperf_models/

${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp:
	# Building mlperf_mobile docker
	docker image build -t mlperf_mobile:1.1 $(dir $(abspath ${TOPDIR}/datasets/docker/Dockerfile))
	mkdir -p ${DLCBUILDDIR}
	touch $@

${DLCBUILDDIR}/mlperf_mosaic_docker.stamp:
	# Building mlperf_mosaic docker
	docker image build -t mlperf_mosaic:1.0 $(dir $(abspath ${TOPDIR}/mobile_back_qti/docker/mlperf_mosaic/Dockerfile))
	mkdir -p ${DLCBUILDDIR}
	touch $@

${DLCBUILDDIR}/mlperf_snusr_docker.stamp:
	# Building snusr docker
	docker image build -t mlperf_snusr:1.0 $(dir $(abspath ${TOPDIR}/mobile_back_qti/docker/mlperf_snusr/Dockerfile))
	mkdir -p ${DLCBUILDDIR}
	touch $@

htp-dlc: ${DLCBUILDDIR}/mobilebert_htp.stamp \
	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp.stamp \
	${DLCBUILDDIR}/ssd_mobiledet_qat_htp.stamp \
	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched.stamp \
	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd7g1.stamp \
	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd8pg1.stamp \
	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd8g2.stamp \
	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd8cxg3.stamp \
	${DLCBUILDDIR}/mobile_mosaic_htp.stamp \
	${DLCBUILDDIR}/snusr_htp.stamp

mobilenet_edgetpu_batched: \
	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched.stamp \
    ${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd7g1.stamp \
    ${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd8g2.stamp \
    ${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd8cxg3.stamp \
   	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd8pg1.stamp

8pg1: \
	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd8pg1.stamp

7g1: \
	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd7g1.stamp

8g2: \
	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd8g2.stamp

sd8cxg3: \
	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd8cxg3.stamp

mosaic: \
	${DLCBUILDDIR}/mobile_mosaic_htp.stamp

mobilenet_edgetpu: \
	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp.stamp

mobiledet: \
	${DLCBUILDDIR}/ssd_mobiledet_qat_htp.stamp

mobilebert: \
	${DLCBUILDDIR}/mobilebert_htp.stamp

snusr : \
	${DLCBUILDDIR}/snusr_htp.stamp

mlperf_models: \
	${DLCBUILDDIR}/mlperf_models.stamp

${BUILDDIR}/datasets.stamp:
	(cd ../datasets && make)
	touch $@

${DLCBUILDDIR}/mobile/.stamp:
	# Downloading mobile
	mkdir -p ${DLCBUILDDIR}
	(cd ${DLCBUILDDIR} && git clone --depth=1 https://github.com/mlcommons/mobile_open.git mobile)
	touch $@

${DLCBUILDDIR}/mlperf_models.stamp:
	# Creating folder to store required models
	(mkdir -p ${MLPERF_MODELS_PATH})
	touch $@

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_float.dlc: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# Mobilenetedge TPU model conversion ....
	# Float model
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${MOBILENETEDGETPU_MODEL_PATH}:/models \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-tensorflow-to-dlc \
			-i /models/frozen_graph_tf1x_transform.pb \
			-d input "1,224,224,3" --out_node "MobilenetEdgeTPU/Predictions/Softmax" \
			-o /output/mobilenet_edgetpu_224_1.0_float.dlc

${DLCBUILDDIR}/imagenet/imagenet_image_list.txt: ${DLCBUILDDIR}/imagenet/state/quantdata.stamp
	cat ${TOPDIR}/datasets/downloads/imagenet/cal_image_list_option_1.txt | sed "s!^!quantdata/!" | sed "s!JPEG!raw!" > $@

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_quant.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_float.dlc \
		${DLCBUILDDIR}/imagenet/imagenet_image_list.txt
	# Quantization of MobilenetEdgeTPU DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/imagenet:/imagenet \
		-w /imagenet \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quant \
			--input_dlc=/output/mobilenet_edgetpu_224_1.0_float.dlc \
			--input_list=imagenet_image_list.txt \
			--output_dlc=/output/mobilenet_edgetpu_224_1.0_quant.dlc \
	# Mobilenetedge TPU model conversion completed

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_quant.stamp \
	# Offline prepare of MobilenetEdgeTPU DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/imagenet:/imagenet \
		-w /imagenet \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc=/output/mobilenet_edgetpu_224_1.0_quant.dlc \
			--output_dlc=/output/mobilenet_edgetpu_224_1.0_htp.dlc \
            --htp_socs=sm8350,sm7325,sm8450,sm8550,sm7475
	# Mobilenetedge TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_float_batched.dlc: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# Mobilenetedge TPU model conversion ....
	# Batched Float model
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${MOBILENETEDGETPU_MODEL_PATH}:/models \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-tensorflow-to-dlc \
			-i /models/frozen_graph_tf1x_transform.pb \
			-d input "3,224,224,3" --out_node "MobilenetEdgeTPU/Predictions/Softmax" \
			-o /output/mobilenet_edgetpu_224_1.0_float_batched.dlc

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_quant_batched.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/imagenet/imagenet_image_list.txt \
		${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_float_batched.dlc
	# Quantization of MobilenetEdgeTPU Batched DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/imagenet:/imagenet-out \
		-w /imagenet-out \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quant \
			--input_dlc=/output/mobilenet_edgetpu_224_1.0_float_batched.dlc \
			--input_list=imagenet_image_list.txt \
			--output_dlc=/output/mobilenet_edgetpu_224_1.0_quant_batched.dlc
	# Mobilenetedge TPU model conversion completed

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_quant_batched.stamp
	# Offline prepare of MobilenetEdgeTPU Batched DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc=/output/mobilenet_edgetpu_224_1.0_quant_batched.dlc \
			--output_dlc=/output/mobilenet_edgetpu_224_1.0_htp_batched.dlc \
			--htp_socs=sm8350,sm7325,sm8450,sm8550,sm7475
	# Mobilenetedge TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_float_batched_sd7g1.dlc: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# Mobilenetedge TPU model conversion ....
	# Batched Float model
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${MOBILENETEDGETPU_MODEL_PATH}:/models \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-tensorflow-to-dlc \
			-i /models/frozen_graph_tf1x_transform.pb \
			-d input "4,224,224,3" --out_node "MobilenetEdgeTPU/Predictions/Softmax" \
			-o /output/mobilenet_edgetpu_224_1.0_float_batched_sd7g1.dlc

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_quant_batched_sd7g1.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/imagenet/imagenet_image_list.txt \
		${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_float_batched_sd7g1.dlc
	# Quantization of MobilenetEdgeTPU Batched DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/imagenet:/imagenet-out \
		-w /imagenet-out \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quant \
			--input_dlc=/output/mobilenet_edgetpu_224_1.0_float_batched_sd7g1.dlc \
			--input_list=imagenet_image_list.txt \
			--output_dlc=/output/mobilenet_edgetpu_224_1.0_quant_batched_sd7g1.dlc
	# Mobilenetedge TPU model conversion completed

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd7g1.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_quant_batched_sd7g1.stamp
	# Offline prepare of MobilenetEdgeTPU Batched DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc=/output/mobilenet_edgetpu_224_1.0_quant_batched_sd7g1.dlc \
			--output_dlc=/output/mobilenet_edgetpu_224_1.0_htp_batched_sd7g1.dlc \
			--htp_socs=sm8350,sm7325,sm8450,sm8550,sm7475
	# Mobilenetedge TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd7g1.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_float_batched_sd8pg1.dlc: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# Mobilenetedge TPU model conversion ....
	# Batched Float model
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${MOBILENETEDGETPU_MODEL_PATH}:/models \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-tensorflow-to-dlc \
			-i /models/frozen_graph_tf1x_transform.pb \
			-d input "4,224,224,3" --out_node "MobilenetEdgeTPU/Predictions/Softmax" \
			-o /output/mobilenet_edgetpu_224_1.0_float_batched_sd8pg1.dlc

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_quant_batched_sd8pg1.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/imagenet/imagenet_image_list.txt \
		${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_float_batched_sd8pg1.dlc
	# Quantization of MobilenetEdgeTPU Batched DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/imagenet:/imagenet-out \
		-w /imagenet-out \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quant \
			--input_dlc=/output/mobilenet_edgetpu_224_1.0_float_batched_sd8pg1.dlc \
			--input_list=imagenet_image_list.txt \
			--output_dlc=/output/mobilenet_edgetpu_224_1.0_quant_batched_sd8pg1.dlc
	# Mobilenetedge TPU model conversion completed

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd8pg1.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_quant_batched_sd8pg1.stamp
	# Offline prepare of MobilenetEdgeTPU Batched DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc=/output/mobilenet_edgetpu_224_1.0_quant_batched_sd8pg1.dlc \
			--output_dlc=/output/mobilenet_edgetpu_224_1.0_htp_batched_sd8pg1.dlc \
			--htp_socs=sm8350,sm7325,sm8450,sm8550,sm7475
	# Mobilenetedge TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd8pg1.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_float_batched_sd8g2.dlc: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# Mobilenetedge TPU model conversion ....
	# Batched Float model
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${MOBILENETEDGETPU_MODEL_PATH}:/models \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-tensorflow-to-dlc \
			-i /models/frozen_graph_tf1x_transform.pb \
			-d input "3,224,224,3" --out_node "MobilenetEdgeTPU/Predictions/Softmax" \
			-o /output/mobilenet_edgetpu_224_1.0_float_batched_sd8g2.dlc

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_quant_batched_sd8g2.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/imagenet/imagenet_image_list.txt \
		${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_float_batched_sd8g2.dlc
	# Quantization of MobilenetEdgeTPU Batched DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/imagenet:/imagenet-out \
		-w /imagenet-out \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quant \
			--input_dlc=/output/mobilenet_edgetpu_224_1.0_float_batched_sd8g2.dlc \
			--input_list=imagenet_image_list.txt \
			--output_dlc=/output/mobilenet_edgetpu_224_1.0_quant_batched_sd8g2.dlc
	# Mobilenetedge TPU model conversion completed

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd8g2.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_quant_batched_sd8g2.stamp
	# Offline prepare of MobilenetEdgeTPU Batched DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc=/output/mobilenet_edgetpu_224_1.0_quant_batched_sd8g2.dlc \
			--output_dlc=/output/mobilenet_edgetpu_224_1.0_htp_batched_sd8g2.dlc \
			--htp_socs=sm8350,sm7325,sm8450,sm8550,sm7475
	# Mobilenetedge TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd8g2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_float_batched_sd8cxg3.dlc: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# Mobilenetedge TPU model conversion ....
	# Batched Float model
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${MOBILENETEDGETPU_MODEL_PATH}:/models \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-tensorflow-to-dlc \
			-i /models/frozen_graph_tf1x_transform.pb \
			-d input "8,224,224,3" --out_node "MobilenetEdgeTPU/Predictions/Softmax" \
			-o /output/mobilenet_edgetpu_224_1.0_float_batched_sd8cxg3.dlc

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_quant_batched_sd8cxg3.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/imagenet/imagenet_image_list.txt \
		${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_float_batched_sd8cxg3.dlc
	# Quantization of MobilenetEdgeTPU Batched DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/imagenet:/imagenet-out \
		-w /imagenet-out \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quant \
			--input_dlc=/output/mobilenet_edgetpu_224_1.0_float_batched_sd8cxg3.dlc \
			--input_list=imagenet_image_list.txt \
			--output_dlc=/output/mobilenet_edgetpu_224_1.0_quant_batched_sd8cxg3.dlc
	# Mobilenetedge TPU model conversion completed

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd8cxg3.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_quant_batched_sd8cxg3.stamp
	# Offline prepare of MobilenetEdgeTPU Batched DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc=/output/mobilenet_edgetpu_224_1.0_quant_batched_sd8cxg3.dlc \
			--output_dlc=/output/mobilenet_edgetpu_224_1.0_htp_batched_sd8cxg3.dlc \
			--htp_socs=sm8350,sm7325,sm8450,sm8550,sm7475
	# Mobilenetedge TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched_sd8cxg3.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/ssd_mobiledet_qat.dlc: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/dlc \
		-v ${DLCBUILDDIR}/mobile/vision/mobiledet/uint8/export_inference_graph:/model \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-tensorflow-to-dlc \
			-i model/frozen_inference_graph.pb \
			-d Preprocessor/map/TensorArrayStack/TensorArrayGatherV3 "1,320,320,3" \
			--out_node "detection_classes" \
			--out_node "detection_boxes" \
			--out_node "detection_scores" \
			-o /dlc/ssd_mobiledet_qat.dlc
		
${DLCBUILDDIR}/coco/coco_image_list.txt: ${DLCBUILDDIR}/coco/state/quantdata.stamp
	cat ${TOPDIR}/datasets/util/coco/coco_calibration_files.txt | sed "s!^!quantdata/!" | sed "s!jpg!raw!" > ${DLCBUILDDIR}/coco/coco_image_list.txt

${DLCBUILDDIR}/ssd_mobiledet_qat_quant.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/coco/coco_image_list.txt \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp \
		${DLCBUILDDIR}/ssd_mobiledet_qat.dlc
	# Quantization of MobileDET SSD DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${MOBILEDETSSDQAT_MODEL_PATH}:/models \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/coco:/coco-out \
		-w /coco-out \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quant \
			--input_dlc=/output/ssd_mobiledet_qat.dlc \
			--input_list=coco_image_list.txt \
			--output_dlc=/output/ssd_mobiledet_qat_quant.dlc
	# SSD MobileDET model quantization completed


${DLCBUILDDIR}/ssd_mobiledet_qat_htp.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/ssd_mobiledet_qat_quant.stamp
	# Offline prepare of MobileDET SSD DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc=/output/ssd_mobiledet_qat_quant.dlc \
			--output_dlc=/output/ssd_mobiledet_qat_htp.dlc \
			--set_output_layers=Postprocessor/BatchMultiClassNonMaxSuppression \
			--htp_socs=sm8350,sm7325,sm8450,sm8550,sm7475
	# SSD MobileDET model offline prepare for HTP completed
	cp	${DLCBUILDDIR}/ssd_mobiledet_qat_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/ade20k/ade20k_image_list.txt: ${DLCBUILDDIR}/ade20k/state/quantdata.stamp
	mkdir -p ${DLCBUILDDIR}/ade20k
	cat ${TOPDIR}/datasets/util/ade20k/ade20k_calibration_files.txt | sed "s!^!/ade20k/quantdata/!" | sed "s!jpg!raw!" > $@

${DLCBUILDDIR}/mobilebert_quant.pb: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# MobileBERT quant model freeze ....
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${MOBILEBERT_MODEL_PATH}:/models \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		python3 /usr/local/lib/python3.6/dist-packages/tensorflow_core/python/tools/freeze_graph.py \
			--input_graph=/models/saved_model.pb --input_checkpoint=/models/checkpoints/quant \
			--output_graph=/output/mobilebert_quant.pb \
			--output_node_names=end_logits,start_logits \
			--input_binary= True \
			--input_saved_model_dir=/models/ saved_model_tags="serve"

${DLCBUILDDIR}/mobilebert_float.dlc: \
		${DLCBUILDDIR}/mobilebert_quant.pb \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DATASETS_OUT}/state/calibration.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# MobileBERT TF to DLC conversion
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCDIR}:/dlc \
		-v ${MOBILEBERT_MODEL_PATH}:/models \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-tensorflow-to-dlc \
			--input_network /output/mobilebert_quant.pb \
			--input_dim bert/embeddings/ExpandDims 1,384,1 \
			--input_dim input_mask 1,384 \
			--input_dim segment_ids 1,384 \
			--input_type bert/embeddings/ExpandDims opaque \
			--input_type input_mask default \
			--input_type segment_ids opaque \
			--out_node transpose \
			-o /output/mobilebert_float.dlc
	# MobileBERT Float DLC conversion completed

${DLCBUILDDIR}/mobilebert_quant.stamp: \
		${DLCBUILDDIR}/mobilebert_float.dlc \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DATASETS}/output/state/squad_calibration.stamp
	# MobileBERT float model quantization ...
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${CALIBRATION_DATA}/squad:/squad-out \
		-v ${DLCBUILDDIR}:/output \
		-w /squad-out \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quant \
			--input_dlc=/output/mobilebert_float.dlc \
			--input_list=input_list.txt \
			--weights_bitwidth 8 \
			--act_bitwidth 8 \
			--override_params \
			--output_dlc=/output/mobilebert_quant.dlc

${DLCBUILDDIR}/mobilebert_htp.stamp: \
		${DLCBUILDDIR}/mobilebert_quant.stamp \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
	# MobileBERT quant model offline prepare ...
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc=/output/mobilebert_quant.dlc \
			--output_dlc=/output/mobilebert_quantized_htp.dlc \
			--set_output_layers=transpose \
			--htp_socs=sm7325,sm8350,sm8450,sm8550,sm7475
	cp	${DLCBUILDDIR}/mobilebert_quantized_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobile_mosaic_float.dlc: \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_mosaic_docker.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# Mobile Mosaic model conversion ....
	# Mobile Mosaic QAT model
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${MOBILEMOSAIC_MODEL_PATH}:/mobile_mosaic \
		-v ${DLCBUILDDIR}:/output \
		-v ${TOPDIR}/DLC:/dlc \
		-u ${USERID}:${GROUPID} \
		mlperf_mosaic:1.0 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-tensorflow-to-dlc \
			-i /mobile_mosaic/mobile_segmenter_r4_frozen_graph.pb \
			-d sub_2 "1,512,512,3" \
			--out_node ArgMax \
			-o /output/mobile_mosaic_float.dlc
	# Generated DLC from mobile Mosaic QAT model

${DLCBUILDDIR}/mobile_mosaic_quant.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/ade20k/ade20k_image_list.txt \
		${DLCBUILDDIR}/mobile_mosaic_float.dlc
	# Quantization of Mobile Mosaic DLC
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/ade20k:/ade20k \
		-w /ade20k \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quant \
			--input_dlc=/output/mobile_mosaic_float.dlc \
			--input_list=ade20k_image_list.txt \
			--output_dlc=/output/mobile_mosaic_quant.dlc \
			--optimizations cle
	#Mobile Mosaic model quantization completed

${DLCBUILDDIR}/mobile_mosaic_htp.stamp: \
        ${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobile_mosaic_quant.stamp \
	# Offline prepare of Mobile Mosaic DLC
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc=/output/mobile_mosaic_quant.dlc \
			--output_dlc=/output/mobile_mosaic_htp.dlc \
			--htp_socs=sm7325,sm8350,sm8450,sm8550,sm7475
	#Mobile Mosaic HTP model offline prepare completed
	cp	${DLCBUILDDIR}/mobile_mosaic_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/snusr_float.dlc: \
		${DLCBUILDDIR}/mlperf_snusr_docker.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# SNUSR model conversion ....
	# Float model
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-v ${SNPE_SDK}:/snpe_sdk \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNUSR_MODEL_PATH}:/models \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_snusr:1.0 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-tensorflow-to-dlc \
			-i /models \
			-d input_1 "1,540,960,3" --out_node "lambda_1" \
			-o /output/snusr_float.dlc
	# SNUSR float DLC completed		
	touch $@

${DLCBUILDDIR}/snusr_quant.stamp: \
        ${DLCBUILDDIR}/mlperf_snusr_docker.stamp \
		${DLCBUILDDIR}/snusr_float.dlc \
		${DLCBUILDDIR}/snusr_calibration_list.txt
	# Offline prepare of SNUSR DLC
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/snusr/:/snusr \
		-w /snusr \
		-v ${TOPDIR}:/dir \
		-u ${USERID}:${GROUPID} \
		mlperf_snusr:1.0 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quant \
			--input_dlc=/output/snusr_float.dlc \
			--input_list=snusr_calibration_list.txt \
			--output_dlc=/output/snusr_quant.dlc \
	#SNUSR offline prepare completed
	touch $@

${DLCBUILDDIR}/snusr_htp.stamp: \
        ${DLCBUILDDIR}/mlperf_snusr_docker.stamp \
		${DLCBUILDDIR}/snusr_quant.stamp \
		${DLCBUILDDIR}/snusr_calibration_list.txt
	#HTP Graph prepare of SNUSR DLC
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_snusr:1.0 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc=/output/snusr_quant.dlc \
			--output_dlc=/output/snusr_htp.dlc \
			--htp_socs=sm7325,sm8350,sm8450,sm8550,sm7475
	#HTP Graph prepare of SNUSR model completed
	cp	${DLCBUILDDIR}/snusr_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

####################################################################################
# CALIBRATION / QUANTIZATION
####################################################################################

# OpenSR calibration data preprocessing
${DLCBUILDDIR}/snusr_calibration_list.txt: \
			${DLCBUILDDIR}/mlperf_snusr_docker.stamp \
			${DATASETS}/output/state/snusr_calibration.stamp 
	# Preparing SNUSR dataset
	mkdir -p ${DATASETS}/output/snusr/_raws
	docker run \
		-v ${TOPDIR}/mobile_back_qti/DLC/util/:/util \
		-v ${DATASETS}/output/snusr/:/snusr \
		-v ${SNUSR_CALIBRATION_PATH}:/calibration \
		-u ${USERID}:${GROUPID} \
		mlperf_snusr:1.0 \
		python3 /util/snusr/rgb8_to_raw.py /snusr/ /calibration/OpenImages_calibration_rgb8.txt
	mkdir -p ${DLCBUILDDIR}/snusr
	mv ${DATASETS}/output/snusr/_raws ${DLCBUILDDIR}/snusr/ 
	touch ${DLCBUILDDIR}/snusr/snusr_calibration_list.txt
	ls ${DLCBUILDDIR}/snusr/_raws | sed "s!^!_raws/!" > ${DLCBUILDDIR}/snusr/snusr_calibration_list.txt
	touch $@

# ADE20K calibration data preprocessing
${DLCBUILDDIR}/ade20k/state/resized.stamp: \
		${DATASETS_OUT}/state/calibration.stamp \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp
	# Resizing ADE20K calibration data
	rm -rf ${DLCBUILDDIR}/ade20k/resized
	mkdir -p ${DLCBUILDDIR}/ade20k/resized
	docker run \
		-e PYTHONPATH=/input/models-2.3.0/research/slim:/input/models-2.3.0/research/deeplab:/input/models-2.3.0/research \
		-v $(CURDIR)/util/ade20k:/util/ \
		-v ${TOPDIR}/datasets/util/ade20k:/util2 \
		-v ${DATASETS_OUT}/ade20k:/input/ \
		-v $(DLCBUILDDIR)/ade20k:/output/ \
	  	-u ${USERID}:${GROUPID} \
	  	mlperf_mobile:1.1 \
		python3 /util/resize_512_ade20k_calibration.py /input/ADEChallengeData2016/ /output/resized /util2/ade20k_calibration_files.txt
	mkdir -p ${DLCBUILDDIR}/ade20k/state
	touch $@

${DLCBUILDDIR}/ade20k/state/quantdata.stamp: \
		${DLCBUILDDIR}/ade20k/state/resized.stamp \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp
	# Generating ADE20K quantization data for SNPE
	rm -rf ${DLCBUILDDIR}/ade20k/resized_raw
	mkdir -p ${DLCBUILDDIR}/ade20k/resized_raw
	docker run \
		-v ${TOPDIR}/datasets/util:/util \
		-v ${DLCBUILDDIR}/ade20k:/ade20k \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/bin/bash -c "python3 /util/common/jpg_to_raw.py /ade20k/resized"
	rm -rf ${DLCBUILDDIR}/ade20k/quantdata
	mv ${DLCBUILDDIR}/ade20k/resized_raw ${DLCBUILDDIR}/ade20k/quantdata
	touch $@

# Imagenet calibration data preprocessing
${DLCBUILDDIR}/imagenet/state/resized.stamp: \
		${DATASETS_OUT}/state/calibration.stamp \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp
	# Scaling Imagenet images to 224x224
	rm -rf ${DLCBUILDDIR}/imagenet/resized
	mkdir -p ${DLCBUILDDIR}/imagenet/resized
	docker run \
		-v ${TOPDIR}/datasets/util:/util \
		-v ${CALIBRATION_DATA}/imagenet:/imagenet \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
	/bin/bash -c "python3 /util/imagenet/Resize224.py /imagenet/images /output/imagenet/resized"
	mkdir -p ${DLCBUILDDIR}/imagenet/state
	touch $@

${DLCBUILDDIR}/imagenet/state/quantdata.stamp: \
		${DLCBUILDDIR}/imagenet/state/resized.stamp \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp
	# Generating Imagenet quantization data for SNPE
	rm -rf ${DLCBUILDDIR}/imagenet/resized_raw
	mkdir -p ${DLCBUILDDIR}/imagenet/resized_raw
	docker run \
		-v ${TOPDIR}/datasets/util:/util \
		-v ${DLCBUILDDIR}/imagenet:/imagenet \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/bin/bash -c "python3 /util/common/jpg_to_raw.py /imagenet/resized"
	rm -rf ${DLCBUILDDIR}/imagenet/quantdata
	mv ${DLCBUILDDIR}/imagenet/resized_raw ${DLCBUILDDIR}/imagenet/quantdata
	touch $@

# Coco calibration data preprocessing
${DLCBUILDDIR}/coco/state/resized.stamp: \
		${DATASETS_OUT}/state/calibration.stamp \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp
	# Scaling Coco images to 320x320
	rm -rf ${DLCBUILDDIR}/coco/resized
	mkdir -p ${DLCBUILDDIR}/coco/resized
	docker run \
		-v ${CALIBRATION_DATA}/coco:/coco \
		-v ${DLCBUILDDIR}/coco/resized:/resized \
		-v ${TOPDIR}/datasets/util:/util \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		python3 /util/coco/upscale_coco.py --inputs /coco/images --outputs /resized --size 320 320
	mkdir -p ${DLCBUILDDIR}/coco/state
	touch $@

# Create the raw files used by SNPE for calibration/quantization
${DLCBUILDDIR}/coco/state/quantdata.stamp: \
		${DLCBUILDDIR}/coco/state/resized.stamp \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp
	# Generating Coco quantization data for SNPE
	rm -rf ${DLCBUILDDIR}/coco/resized_raw
	mkdir -p ${DLCBUILDDIR}/coco/resized_raw
	docker run \
		-v ${TOPDIR}/datasets/util:/util \
		-v ${DLCBUILDDIR}/coco:/coco_out \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/bin/bash -c "python3 /util/common/jpg_to_raw.py /coco_out/resized"
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
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		htp-dlc
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/dlc \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/bin/bash -c '\
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobilenet_edgetpu_224_1.0_htp.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/ssd_mobiledet_qat_htp.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobilenet_edgetpu_224_1.0_htp_batched.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobilenet_edgetpu_224_1.0_htp_batched_sd7g1.dlc && \
            /snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobilenet_edgetpu_224_1.0_htp_batched_sd8pg1.dlc && \
            /snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobilenet_edgetpu_224_1.0_htp_batched_sd8g2.dlc && \
            /snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobilenet_edgetpu_224_1.0_htp_batched_sd8cxg3.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobile_mosaic_htp.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/snusr_htp.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobilebert_quantized_htp.dlc'

####################################################################################
# Clean
####################################################################################
clean:
	rm -rf ${BUILDDIR}/DLC

