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

# This file must be called from top level Makefile

include ../make/builddir.mk

DATASETS:=${TOPDIR}/datasets
DATASETS_OUT:=${DATASETS}/output
CALIBRATION_DATA:=${DATASETS_OUT}/calibration_set
MLPERF_DATASETS:=${DATASETS_OUT}/mlperf_datasets

all: ${DATASETS_OUT}/state/calibration.stamp hta-dlc htp-dlc

# Download and build datasets and calibration files
${DATASETS_OUT}/state/calibration.stamp:
	(cd ${TOPDIR}/datasets && make)

.PHONY: hta-dlc htp-dlc dependencies gen-hta-dlc-info gen-htp-dlc-info clean

DLCBUILDDIR=${BUILDDIR}/DLC
MODEL_BASE_PATH=${DLCBUILDDIR}/mobile
MOBILENETEDGETPU_MODEL_PATH=${MODEL_BASE_PATH}/vision/mobilenet/models_and_code/checkpoints/float
DEEPLABV3QAT_MODEL_PATH=${MODEL_BASE_PATH}/vision/deeplab/models_and_code/checkpoints/quantize_aware_training
MOBILEBERT_MODEL_PATH=${MODEL_BASE_PATH}/language/bert/models_and_code/checkpoints/quant/

${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp:
	echo "Building mlperf_mobile docker"
	docker image build -t mlperf_mobile:1.1 $(dir $(abspath ${TOPDIR}/android/docker/mlperf_mobile/Dockerfile))
	mkdir -p ${DLCBUILDDIR}
	touch $@


hta-dlc: ${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_hta.stamp \
	${DLCBUILDDIR}/ssd_mobiledet_qat_hta.stamp \
	${DLCBUILDDIR}/deeplabv3_hta.stamp

htp-dlc: ${DLCBUILDDIR}/mobilebert_htp.stamp \
	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp.stamp \
	${DLCBUILDDIR}/ssd_mobiledet_qat_htp.stamp \
	${DLCBUILDDIR}/deeplabv3_htp.stamp \
	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched.stamp

mobilenet_edgetpu: \
	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp.stamp \
	${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_hta.stamp

mobiledet: \
	${DLCBUILDDIR}/ssd_mobiledet_qat_hta.stamp \
	${DLCBUILDDIR}/ssd_mobiledet_qat_htp.stamp

deeplabv3: \
	${DLCBUILDDIR}/deeplabv3_htp.stamp \
	${DLCBUILDDIR}/deeplabv3_hta.stamp

mobilebert: \
	${DLCBUILDDIR}/mobilebert_htp.stamp

${DLCBUILDDIR}/mobile/.stamp:
	echo "Downloading mobile"
	mkdir -p ${DLCBUILDDIR}
	(cd ${DLCBUILDDIR} && git clone --depth=1 https://github.com/mlcommons/mobile_open.git mobile)
	touch $@

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_float.dlc: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobile/.stamp
	echo "Mobilenetedge TPU model conversion ...."
	echo "Float model"
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

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_hta.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/imagenet/imagenet_image_list.txt \
		${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_float.dlc
	echo "Quantization of MobilenetEdgeTPU DLC for HTA"
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/imagenet:/imagenet-out \
		-w /imagenet-out \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quantize \
			--input_dlc=/output/mobilenet_edgetpu_224_1.0_float.dlc \
			--input_list=imagenet_image_list.txt \
			--output_dlc=/output/mobilenet_edgetpu_224_1.0_hta.dlc \
			--enable_hta
	echo "Mobilenetedge TPU model conversion completed"
	touch $@

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_float.dlc \
		${DLCBUILDDIR}/imagenet/imagenet_image_list.txt
	echo "Quantization of MobilenetEdgeTPU DLC for HTP"
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/imagenet:/imagenet \
		-w /imagenet \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quantize \
			--input_dlc=/output/mobilenet_edgetpu_224_1.0_float.dlc \
			--input_list=imagenet_image_list.txt \
			--output_dlc=/output/mobilenet_edgetpu_224_1.0_htp.dlc \
			--enable_htp \
			--htp_socs sm8350,sm7325
	echo "Mobilenetedge TPU model conversion completed"
	touch $@

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_float_batched.dlc: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobile/.stamp
	echo "Mobilenetedge TPU model conversion ...."
	echo "Batched Float model"
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
			-o /output/mobilenet_edgetpu_224_1.0_float_batched.dlc

${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_htp_batched.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/imagenet/imagenet_image_list.txt \
		${DLCBUILDDIR}/mobilenet_edgetpu_224_1.0_float_batched.dlc
	echo "Quantization of MobilenetEdgeTPU Batched DLC for HTP"
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/imagenet:/imagenet-out \
		-w /imagenet-out \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quantize \
			--input_dlc=/output/mobilenet_edgetpu_224_1.0_float_batched.dlc \
			--input_list=imagenet_image_list.txt \
			--output_dlc=/output/mobilenet_edgetpu_224_1.0_htp_batched.dlc \
			--enable_htp \
			--htp_socs sm8350,sm7325
	echo "Mobilenetedge TPU model conversion completed"
	touch $@
	
${DLCBUILDDIR}/ssd_mobiledet_qat.dlc: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobile/.stamp
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

${DLCBUILDDIR}/ssd_mobiledet_qat_hta.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/coco/coco_image_list.txt \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/ssd_mobiledet_qat.dlc
	echo "Quantization of MobileDET SSD DLC for HTA"
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/coco:/coco-out \
		-w /coco-out \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quantize \
			--input_dlc=/output/ssd_mobiledet_qat.dlc \
			--input_list=coco_image_list.txt \
			--output_dlc=/output/ssd_mobiledet_qat_hta.dlc \
			--enable_hta --hta_partitions sm8250
	echo "SSD MobileDET model conversion for HTA completed"
	touch $@

${DLCBUILDDIR}/ssd_mobiledet_qat_htp.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/coco/coco_image_list.txt \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/ssd_mobiledet_qat.dlc
	echo "Quantization of MobileDET SSD DLC for HTP"
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
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quantize \
			--input_dlc=/output/ssd_mobiledet_qat.dlc \
			--input_list=coco_image_list.txt \
			--output_dlc=/output/ssd_mobiledet_qat_htp.dlc \
			--enable_htp \
			--htp_socs sm8350,sm7325
	echo "SSD MobileDET model conversion for HTP completed"
	touch $@


${DLCBUILDDIR}/deeplabv3_quantized.dlc: \
		${DLCDIR}/argmax-quant.json \
		${DEEPLABV3QAT_MODEL_PATH}/freeze.pb \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobile/.stamp
	echo "Deeplab v3 model conversion ...."
	echo "Deeplabv3 QAT model"
	cat ${DLCDIR}/argmax-quant.json
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DEEPLABV3QAT_MODEL_PATH}:/deeplabv3 \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCDIR}:/dlc \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-tensorflow-to-dlc \
			-i /deeplabv3/freeze.pb \
			-d ImageTensor "1,512,512,3" \
			--out_node ArgMax \
			--quantization_overrides /dlc/argmax-quant.json \
			-o /output/deeplabv3_quantized.dlc
	echo "Generated DLC from Deeplabv3 QAT model"


${DLCBUILDDIR}/ade20k/ade20k_image_list.txt: ${DLCBUILDDIR}/ade20k/state/quantdata.stamp
	mkdir -p ${DLCBUILDDIR}/ade20k
	cat ${TOPDIR}/datasets/util/ade20k/ade20k_calibration_files.txt | sed "s!^!/ade20k/quantdata/!" | sed "s!jpg!raw!" > $@

${DLCBUILDDIR}/deeplabv3_hta.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/ade20k/ade20k_image_list.txt \
		${DLCBUILDDIR}/deeplabv3_quantized.dlc
	echo "Quantization of Deeplabv3 DLC"
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/ade20k:/ade20k \
		-w /ade20k \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quantize \
			--input_dlc=/output/deeplabv3_quantized.dlc \
			--input_list=ade20k_image_list.txt \
			--output_dlc=/output/deeplabv3_hta.dlc \
			--enable_hta
	echo "Deeplab v3 model conversion completed"
	# Can't use deeplabv3_hta.dlc as make target since this file is present on failure
	touch $@

${DLCBUILDDIR}/deeplabv3_htp.stamp: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/ade20k/ade20k_image_list.txt \
		${DLCBUILDDIR}/deeplabv3_quantized.dlc
	echo "Quantization of Deeplabv3 DLC"
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/ade20k:/ade20k \
		-w /ade20k \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quantize \
			--input_dlc=/output/deeplabv3_quantized.dlc \
			--input_list=ade20k_image_list.txt \
			--output_dlc=/output/deeplabv3_htp.dlc \
			--override_params --enable_htp \
			--htp_socs sm8350,sm7325
	echo "Deeplab v3 model conversion completed"
	# Can't use deeplabv3_htp.dlc as make target since this file is present on failure
	touch $@

${DLCBUILDDIR}/mobilebert_quant.pb: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DLCBUILDDIR}/mobile/.stamp
	echo "MobileBERT quant model freeze ...."
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
		${DLCBUILDDIR}/mobile/.stamp
	echo "MobileBERT TF to DLC conversion"
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
	echo "MobileBERT Float DLC conversion completed"


${DLCBUILDDIR}/mobilebert_htp.stamp: \
		${DLCBUILDDIR}/mobilebert_float.dlc \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		${DATASETS}/output/state/squad_calibration.stamp
	echo "MobileBERT float model quantization ..."
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
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quantize \
			--input_dlc=/output/mobilebert_float.dlc \
			--input_list=input_list.txt \
			--weights_bitwidth 8 \
			--act_bitwidth 8 \
			--enable_htp \
			--htp_socs sm7325,sm8350 \
			--use_encoding_optimizations \
			--output_dlc=/output/mobilebert_quantized_htp.dlc
	touch $@

####################################################################################
# CALIBRATION / QUANTIZATION
####################################################################################

# ADE20K calibration data preprocessing
${DLCBUILDDIR}/ade20k/state/resized.stamp: \
		${DATASETS_OUT}/state/calibration.stamp \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp
	echo "Resizing ADE20K calibration data"
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
	echo "Generating ADE20K quantization data for SNPE"
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
	echo "Scaling Imagenet images to 224x224"
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
	echo "Generating Imagenet quantization data for SNPE"
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
	echo "Scaling Coco images to 320x320"
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
	echo "Generating Coco quantization data for SNPE"
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

####################################################################################
# DLC Info
####################################################################################
gen-hta-dlc-info: \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp \
		hta-dlc
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/dlc \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/bin/bash -c '\
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobilenet_edgetpu_224_1.0_hta.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/ssd_mobiledet_qat_htp.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/deeplabv3_hta.dlc'

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
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/deeplabv3_htp.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobilenet_edgetpu_224_1.0_htp_batched.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobilebert_quantized_htp.dlc'

####################################################################################
# Clean
####################################################################################
clean:
	rm -rf ${BUILDDIR}/DLC

