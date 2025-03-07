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

.PHONY: htp-quantized-dlc htp-dlc dependencies gen-hta-dlc-info gen-htp-dlc-info clean

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


INTERNAL_DOCKER?=0

ifeq ($(INTERNAL_DOCKER),1)
$(info "Using internal docker")
include load_internal_docker.mk
else
$(info "Using normal docker")
include load_normal_docker.mk
endif

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
mobilenet_v4_apirec mobilenet_v4_batched_apirec mosaic_apirec mobilebert_apirec mobiledet_apirec snusr_apirec))
include ../make/apirec.mk
endif

generate-apirec: mobilenet_v4_apirec \
	mobilenet_v4_batched_apirec \
	mosaic_apirec \
	mobiledet_apirec \
	mobilebert_apirec \
	snusr_apirec

mobilenet_v4_batched: \
    ${DLCBUILDDIR}/mobilenet_v4_htp_batched_4.stamp

mobilenet_v4_batched_O2: \
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

stable_diffusion_qnn: \
	${DLCBUILDDIR}/sd_precompute_data.tar \
	${DLCBUILDDIR}/text_encoder_qnn_context_binary_generator.stamp \
	${DLCBUILDDIR}/vae_decoder_qnn_context_binary_generator.stamp \
	${DLCBUILDDIR}/unet_qnn_context_binary_generator.stamp

text_encoder: \
	${DLCBUILDDIR}/text_encoder_qnn_context_binary_generator.stamp

vae_decoder: \
	${DLCBUILDDIR}/vae_decoder_qnn_context_binary_generator.stamp

unet: \
	${DLCBUILDDIR}/unet_qnn_context_binary_generator.stamp

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
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# MobilenetV4 TPU model conversion ....
	# Float model
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${MOBILENETV4_MODEL_PATH}:/models \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-tensorflow-to-dlc \
			-i /models/ \
			-d inputs "1,384,384,3" --out_node "probs" \
			-o /output/mobilenet_v4_float.dlc

${DLCBUILDDIR}/imagenet/imagenet_image_384_list.txt: ${DLCBUILDDIR}/imagenet/state/quantdata_384.stamp
	cat ${TOPDIR}/datasets/downloads/imagenet/cal_image_list_option_1.txt | sed "s!^!quantdata_384/!" | sed "s!JPEG!raw!" > $@

${DLCBUILDDIR}/mobilenet_v4_quant.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobilenet_v4_float.dlc \
		${DLCBUILDDIR}/imagenet/imagenet_image_384_list.txt
	# Quantization of MobilenetV4 DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/imagenet:/imagenet \
		-w /imagenet \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quant \
			--input_dlc=/output/mobilenet_v4_float.dlc \
			--input_list=imagenet_image_384_list.txt \
			--output_dlc=/output/mobilenet_v4_quant.dlc \
	# Mobilenetedge TPU model conversion completed

${DLCBUILDDIR}/mobilenet_v4_htp_sm8750.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobilenet_v4_quant.stamp \
	# Offline prepare of MobilenetV4 DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/imagenet:/imagenet \
		-w /imagenet \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--htp_dlbc=true \
			--input_dlc=/output/mobilenet_v4_quant.dlc \
			--output_dlc=/output/mobilenet_v4_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm8750 \
                         --memorymapped_buffer_hint=true
	# MobilenetV4 TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_sm7550.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
	# Offline prepare of MobilenetV4 DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/imagenet:/imagenet \
		-w /imagenet \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
			--htp_dlbc=true \
			--input_dlc=/output/mobilenet_v4_htp.dlc \
			--output_dlc=/output/mobilenet_v4_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm7550,sm7635 \
                         --memorymapped_buffer_hint=true
	# MobilenetV4 TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobilenet_v4_htp_sm8750.stamp \
		${DLCBUILDDIR}/mobilenet_v4_htp_sm7550.stamp \
	# Offline prepare of MobilenetV4 DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/imagenet:/imagenet \
		-w /imagenet \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
			--htp_dlbc=true \
			--input_dlc=/output/mobilenet_v4_htp.dlc \
			--output_dlc=/output/mobilenet_v4_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm8650,sm8735,sm8635,sm8350,sm7325,sm8450,sm8550,sm7475,sc8380xp,sc8280x \
                         --memorymapped_buffer_hint=true
	# MobilenetV4 TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_O2_sm8750.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobilenet_v4_quant.stamp \
	# Offline prepare of MobilenetEdgeTPU DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/imagenet:/imagenet \
		-w /imagenet \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--htp_dlbc=true \
			--input_dlc=/output/mobilenet_v4_quant.dlc \
			--output_dlc=/output/mobilenet_v4_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8750 \
                         --memorymapped_buffer_hint=true
	# Mobilenetedge TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_O2_sm7550.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
	# Offline prepare of MobilenetEdgeTPU DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/imagenet:/imagenet \
		-w /imagenet \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
			--htp_dlbc=true \
			--input_dlc=/output/mobilenet_v4_htp_O2.dlc \
			--output_dlc=/output/mobilenet_v4_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm7550,sm7635 \
                         --memorymapped_buffer_hint=true
	# Mobilenetedge TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_O2.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobilenet_v4_htp_O2_sm8750.stamp \
		${DLCBUILDDIR}/mobilenet_v4_htp_O2_sm7550.stamp \
	# Offline prepare of MobilenetEdgeTPU DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/imagenet:/imagenet \
		-w /imagenet \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
			--htp_dlbc=true \
			--input_dlc=/output/mobilenet_v4_htp_O2.dlc \
			--output_dlc=/output/mobilenet_v4_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8650,sm8735,sm8635,sm8350,sm7325,sm8450,sm8550,sm7475,sc8380xp,sc8280x \
                         --memorymapped_buffer_hint=true
	# Mobilenetedge TPU model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_float_batched_4.dlc: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# MobilenetV4 TPU model conversion ....
	# Batched Float model
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${MOBILENETV4_MODEL_PATH}:/models \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-tensorflow-to-dlc \
			-i /models/ \
			-d inputs "4,384,384,3" --out_node "probs" \
			-o /output/mobilenet_v4_float_batched_4.dlc

${DLCBUILDDIR}/mobilenet_v4_quant_batched_4.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/imagenet/imagenet_image_384_list.txt \
		${DLCBUILDDIR}/mobilenet_v4_float_batched_4.dlc
	# Quantization of MobilenetV4 Batched DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-v ${DLCBUILDDIR}/imagenet:/imagenet-out \
		-w /imagenet-out \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quant \
			--input_dlc=/output/mobilenet_v4_float_batched_4.dlc \
			--input_list=imagenet_image_384_list.txt \
			--output_dlc=/output/mobilenet_v4_quant_batched_4.dlc
	# MobilenetV4 model conversion completed

${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_sm8750.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobilenet_v4_quant_batched_4.stamp
	# Offline prepare of MobilenetV4 Batched DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--input_dlc=/output/mobilenet_v4_quant_batched_4.dlc \
			--output_dlc=/output/mobilenet_v4_htp_batched_4.dlc \
			--optimization_level 3 \
			--htp_socs=sm8750 \
                         --memorymapped_buffer_hint=true
	# MobilenetV4 model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_batched_4.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_sm7550.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
	# Offline prepare of MobilenetV4 Batched DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
			--input_dlc=/output/mobilenet_v4_htp_batched_4.dlc \
			--output_dlc=/output/mobilenet_v4_htp_batched_4.dlc \
			--optimization_level 3 \
			--htp_socs=sm7550,sm7635 \
                         --memorymapped_buffer_hint=true
	# MobilenetV4 model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_batched_4.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_batched_4.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_sm8750.stamp \
		${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_sm7550.stamp \
	# Offline prepare of MobilenetV4 Batched DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
			--input_dlc=/output/mobilenet_v4_htp_batched_4.dlc \
			--output_dlc=/output/mobilenet_v4_htp_batched_4.dlc \
			--optimization_level 3 \
			--htp_socs=sm8650,sm8735,sm8635,sm8350,sm7325,sm8450,sm8550,sm7475,sc8380xp,sc8280x \
                         --memorymapped_buffer_hint=true
	# MobilenetV4 model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_batched_4.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2_sm8750.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobilenet_v4_quant_batched_4.stamp
	# Offline prepare of MobilenetV4 Batched DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--input_dlc=/output/mobilenet_v4_quant_batched_4.dlc \
			--output_dlc=/output/mobilenet_v4_htp_batched_4_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8750 \
                         --memorymapped_buffer_hint=true
	# MobilenetV4 model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2_sm7550.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
	# Offline prepare of MobilenetV4 Batched DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
			--input_dlc=/output/mobilenet_v4_htp_batched_4_O2.dlc \
			--output_dlc=/output/mobilenet_v4_htp_batched_4_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm7550,sm7635 \
                         --memorymapped_buffer_hint=true
	# MobilenetV4 model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2_sm8750.stamp \
		${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2_sm7550.stamp \
	# Offline prepare of MobilenetV4 Batched DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
			--input_dlc=/output/mobilenet_v4_htp_batched_4_O2.dlc \
			--output_dlc=/output/mobilenet_v4_htp_batched_4_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8650,sm8735,sm8635,sm8350,sm7325,sm8450,sm8550,sm7475,sc8380xp,sc8280x \
                         --memorymapped_buffer_hint=true
	# MobilenetV4 model conversion completed
	cp	${DLCBUILDDIR}/mobilenet_v4_htp_batched_4_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/ssd_mobiledet_qat.dlc: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/dlc \
		-v ${MOBILEDETSSDQAT_MODEL_PATH}:/model \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
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
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
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
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quant \
			--input_dlc=/output/ssd_mobiledet_qat.dlc \
			--input_list=coco_image_list.txt \
			--output_dlc=/output/ssd_mobiledet_qat_quant.dlc
	# SSD MobileDET model quantization completed

${DLCBUILDDIR}/ssd_mobiledet_qat_htp_sm8750.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/ssd_mobiledet_qat_quant.stamp
	# Offline prepare of MobileDET SSD DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
            --htp_dlbc=true \
			--input_dlc=/output/ssd_mobiledet_qat_quant.dlc \
			--output_dlc=/output/ssd_mobiledet_qat_htp.dlc \
			--set_output_tensors="Postprocessor/BatchMultiClassNonMaxSuppression_classes,Postprocessor/BatchMultiClassNonMaxSuppression_num_detections,detection_scores:0,detection_boxes:0"\
			--optimization_level 3 \
			--htp_socs=sm8750 \
                         --memorymapped_buffer_hint=true
	# SSD MobileDET model offline prepare for HTP completed
	cp	${DLCBUILDDIR}/ssd_mobiledet_qat_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/ssd_mobiledet_qat_htp_sm7550.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
	# Offline prepare of MobileDET SSD DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
            --htp_dlbc=true \
			--input_dlc=/output/ssd_mobiledet_qat_htp.dlc \
			--output_dlc=/output/ssd_mobiledet_qat_htp.dlc \
			--set_output_tensors="Postprocessor/BatchMultiClassNonMaxSuppression_classes,Postprocessor/BatchMultiClassNonMaxSuppression_num_detections,detection_scores:0,detection_boxes:0"\
			--optimization_level 3 \
			--htp_socs=sm7550,sm7635 \
                         --memorymapped_buffer_hint=true
	# SSD MobileDET model offline prepare for HTP completed
	cp	${DLCBUILDDIR}/ssd_mobiledet_qat_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/ssd_mobiledet_qat_htp.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/ssd_mobiledet_qat_htp_sm8750.stamp \
		${DLCBUILDDIR}/ssd_mobiledet_qat_htp_sm7550.stamp \
	# Offline prepare of MobileDET SSD DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
            --htp_dlbc=true \
			--input_dlc=/output/ssd_mobiledet_qat_htp.dlc \
			--output_dlc=/output/ssd_mobiledet_qat_htp.dlc \
			--set_output_tensors="Postprocessor/BatchMultiClassNonMaxSuppression_classes,Postprocessor/BatchMultiClassNonMaxSuppression_num_detections,detection_scores:0,detection_boxes:0"\
			--optimization_level 3 \
			--htp_socs=sm8650,sm8735,sm8635,sm8350,sm7325,sm8450,sm8550,sm7475,sc8380xp,sc8280x \
                         --memorymapped_buffer_hint=true
	# SSD MobileDET model offline prepare for HTP completed
	cp	${DLCBUILDDIR}/ssd_mobiledet_qat_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2_sm8750.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/ssd_mobiledet_qat_quant.stamp
	# Offline prepare of MobileDET SSD DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--input_dlc=/output/ssd_mobiledet_qat_quant.dlc \
			--output_dlc=/output/ssd_mobiledet_qat_htp_O2.dlc \
			--set_output_tensors="Postprocessor/BatchMultiClassNonMaxSuppression_classes,Postprocessor/BatchMultiClassNonMaxSuppression_num_detections,detection_scores:0,detection_boxes:0"\
			--optimization_level 2 \
			--htp_socs=sm8750 \
                         --memorymapped_buffer_hint=true
	# SSD MobileDET model offline prepare for HTP completed
	cp	${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2_sm7550.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
	# Offline prepare of MobileDET SSD DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
			--input_dlc=/output/ssd_mobiledet_qat_htp_O2.dlc \
			--output_dlc=/output/ssd_mobiledet_qat_htp_O2.dlc \
			--set_output_tensors="Postprocessor/BatchMultiClassNonMaxSuppression_classes,Postprocessor/BatchMultiClassNonMaxSuppression_num_detections,detection_scores:0,detection_boxes:0"\
			--optimization_level 2 \
			--htp_socs=sm7550,sm7635 \
                         --memorymapped_buffer_hint=true
	# SSD MobileDET model offline prepare for HTP completed
	cp	${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2_sm8750.stamp \
		${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2_sm7550.stamp \
	# Offline prepare of MobileDET SSD DLC for HTP
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
			--input_dlc=/output/ssd_mobiledet_qat_htp_O2.dlc \
			--output_dlc=/output/ssd_mobiledet_qat_htp_O2.dlc \
			--set_output_tensors="Postprocessor/BatchMultiClassNonMaxSuppression_classes,Postprocessor/BatchMultiClassNonMaxSuppression_num_detections,detection_scores:0,detection_boxes:0"\
			--optimization_level 2 \
			--htp_socs=sm8650,sm8735,sm8635,sm8350,sm7325,sm8450,sm8550,sm7475,sc8380xp,sc8280x \
                         --memorymapped_buffer_hint=true
	# SSD MobileDET model offline prepare for HTP completed
	cp	${DLCBUILDDIR}/ssd_mobiledet_qat_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/ade20k/ade20k_image_list.txt: ${DLCBUILDDIR}/ade20k/state/quantdata.stamp
	mkdir -p ${DLCBUILDDIR}/ade20k
	cat ${TOPDIR}/datasets/util/ade20k/ade20k_calibration_files.txt | sed "s!^!/ade20k/quantdata/!" | sed "s!jpg!raw!" > $@

${DLCBUILDDIR}/mobilebert_quant.pb: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
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
		mlperf_dlc_prepare \
		python3 /usr/local/lib/python3.10/dist-packages/tensorflow/python/tools/freeze_graph.py \
			--input_graph=/models/saved_model.pb --input_checkpoint=/models/checkpoints/quant \
			--output_graph=/output/mobilebert_quant.pb \
			--output_node_names=end_logits,start_logits \
			--input_binary= True \
			--input_saved_model_dir=/models/ saved_model_tags="serve"

${DLCBUILDDIR}/mobilebert_float.dlc: \
		${DLCBUILDDIR}/mobilebert_quant.pb \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
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
		mlperf_dlc_prepare \
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
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
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
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quant \
			--input_dlc=/output/mobilebert_float.dlc \
			--input_list=input_list.txt \
			--weights_bitwidth 8 \
			--act_bitwidth 8 \
			--override_params \
			--output_dlc=/output/mobilebert_quant.dlc

${DLCBUILDDIR}/mobilebert_htp_sm8750.stamp: \
		${DLCBUILDDIR}/mobilebert_quant.stamp \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
	# MobileBERT quant model offline prepare ...
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
            --htp_dlbc=true \
			--input_dlc=/output/mobilebert_quant.dlc \
			--output_dlc=/output/mobilebert_quantized_htp.dlc \
			--set_output_tensors="transpose:0" \
			--optimization_level 3 \
			--htp_socs=sm8750 \
			--memorymapped_buffer_hint=true
	cp	${DLCBUILDDIR}/mobilebert_quantized_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilebert_htp_sm7550.stamp: \
		${DLCBUILDDIR}/mobilebert_quant.stamp \
	# MobileBERT quant model offline prepare ...
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
            --htp_dlbc=true \
			--input_dlc=/output/mobilebert_quantized_htp.dlc \
			--output_dlc=/output/mobilebert_quantized_htp.dlc \
			--set_output_tensors="transpose:0" \
			--optimization_level 3 \
			--htp_socs=sm7550,sm7635 \
			--memorymapped_buffer_hint=true
	cp	${DLCBUILDDIR}/mobilebert_quantized_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilebert_htp.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobilebert_htp_sm8750.stamp \
		${DLCBUILDDIR}/mobilebert_htp_sm7550.stamp \
	# MobileBERT quant model offline prepare ...
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
            --htp_dlbc=true \
			--input_dlc=/output/mobilebert_quantized_htp.dlc \
			--output_dlc=/output/mobilebert_quantized_htp.dlc \
			--set_output_tensors="transpose:0" \
			--optimization_level 3 \
			--htp_socs=sm8650,sm8735,sm8635,sm7325,sm8350,sm8450,sm8550,sm7475,sc8380xp,sc8280x \
			--memorymapped_buffer_hint=true
	cp	${DLCBUILDDIR}/mobilebert_quantized_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilebert_htp_O2_sm8750.stamp: \
		${DLCBUILDDIR}/mobilebert_quant.stamp \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
	# MobileBERT quant model offline prepare ...
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
            --htp_dlbc=true \
			--input_dlc=/output/mobilebert_quant.dlc \
			--output_dlc=/output/mobilebert_quantized_htp_O2.dlc \
			--set_output_tensors="transpose:0" \
			--optimization_level 2 \
			--htp_socs=sm8750 \
			--memorymapped_buffer_hint=true
	cp	${DLCBUILDDIR}/mobilebert_quantized_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilebert_htp_O2_sm7550.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
	# MobileBERT quant model offline prepare ...
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
            --htp_dlbc=true \
			--input_dlc=/output/mobilebert_quantized_htp_O2.dlc \
			--output_dlc=/output/mobilebert_quantized_htp_O2.dlc \
			--set_output_tensors="transpose:0" \
			--optimization_level 2 \
			--htp_socs=sm7550,sm7635 \
			--memorymapped_buffer_hint=true
	cp	${DLCBUILDDIR}/mobilebert_quantized_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobilebert_htp_O2.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobilebert_htp_O2_sm8750.stamp \
		${DLCBUILDDIR}/mobilebert_htp_O2_sm7550.stamp \
	# MobileBERT quant model offline prepare ...
	mkdir -p ${DLCBUILDDIR}
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
            --htp_dlbc=true \
			--input_dlc=/output/mobilebert_quantized_htp_O2.dlc \
			--output_dlc=/output/mobilebert_quantized_htp_O2.dlc \
			--set_output_tensors="transpose:0" \
			--optimization_level 2 \
			--htp_socs=sm8650,sm8735,sm8635,sm7325,sm8350,sm8450,sm8550,sm7475,sc8380xp,sc8280x \
			--memorymapped_buffer_hint=true
	cp	${DLCBUILDDIR}/mobilebert_quantized_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobile_mosaic_float.dlc: \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
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
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-tensorflow-to-dlc \
			-i /mobile_mosaic/mobile_segmenter_r4_frozen_graph.pb \
			-d sub_2 "1,512,512,3" \
			--out_node ArgMax \
			-o /output/mobile_mosaic_float.dlc
	# Generated DLC from mobile Mosaic QAT model
	touch $@

${DLCBUILDDIR}/mobile_mosaic_quant.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
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
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quant \
			--input_dlc=/output/mobile_mosaic_float.dlc \
			--input_list=ade20k_image_list.txt \
			--output_dlc=/output/mobile_mosaic_quant.dlc \
			--optimizations cle
	#Mobile Mosaic model quantization completed
	touch $@

${DLCBUILDDIR}/mobile_mosaic_htp_sm8750.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobile_mosaic_quant.stamp \
	# Offline prepare of Mobile Mosaic DLC
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--htp_dlbc=true \
			--input_dlc=/output/mobile_mosaic_quant.dlc \
			--output_dlc=/output/mobile_mosaic_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm8750 \
			--memorymapped_buffer_hint=true
	#Mobile Mosaic HTP model offline prepare completed
	cp	${DLCBUILDDIR}/mobile_mosaic_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobile_mosaic_htp_sm7550.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
	# Offline prepare of Mobile Mosaic DLC
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
            --htp_dlbc=true \
			--input_dlc=/output/mobile_mosaic_htp.dlc \
			--output_dlc=/output/mobile_mosaic_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm7550,sm7635 \
			--memorymapped_buffer_hint=true
	#Mobile Mosaic HTP model offline prepare completed
	cp	${DLCBUILDDIR}/mobile_mosaic_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobile_mosaic_htp.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobile_mosaic_htp_sm8750.stamp \
		${DLCBUILDDIR}/mobile_mosaic_htp_sm7550.stamp \
	# Offline prepare of Mobile Mosaic DLC
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
            --htp_dlbc=true \
			--input_dlc=/output/mobile_mosaic_htp.dlc \
			--output_dlc=/output/mobile_mosaic_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm8650,sm8735,sm8635,sm7325,sm8350,sm8450,sm8550,sm7475,sc8380xp,sc8280x \
			--memorymapped_buffer_hint=true
	#Mobile Mosaic HTP model offline prepare completed
	cp	${DLCBUILDDIR}/mobile_mosaic_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobile_mosaic_htp_O2_sm8750.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobile_mosaic_quant.stamp \
	# Offline prepare of Mobile Mosaic DLC
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--input_dlc=/output/mobile_mosaic_quant.dlc \
			--output_dlc=/output/mobile_mosaic_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8750 \
			--memorymapped_buffer_hint=true
	#Mobile Mosaic HTP model offline prepare completed
	cp	${DLCBUILDDIR}/mobile_mosaic_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobile_mosaic_htp_O2_sm7550.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
	# Offline prepare of Mobile Mosaic DLC
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
			--input_dlc=/output/mobile_mosaic_htp_O2.dlc \
			--output_dlc=/output/mobile_mosaic_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm7550,sm7635 \
			--memorymapped_buffer_hint=true
	#Mobile Mosaic HTP model offline prepare completed
	cp	${DLCBUILDDIR}/mobile_mosaic_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/mobile_mosaic_htp_O2.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobile_mosaic_htp_O2_sm8750.stamp \
		${DLCBUILDDIR}/mobile_mosaic_htp_O2_sm7550.stamp \
	# Offline prepare of Mobile Mosaic DLC
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
			--input_dlc=/output/mobile_mosaic_htp_O2.dlc \
			--output_dlc=/output/mobile_mosaic_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8650,sm8735,sm8635,sm7325,sm8350,sm8450,sm8550,sm7475,sc8380xp,sc8280x \
			--memorymapped_buffer_hint=true
	#Mobile Mosaic HTP model offline prepare completed
	cp	${DLCBUILDDIR}/mobile_mosaic_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/snusr_float.dlc: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
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
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-tensorflow-to-dlc \
			-i /models \
			-d input_1 "1,540,960,3" --out_node "lambda_1" \
			-o /output/snusr_float.dlc
	# SNUSR float DLC completed
	touch $@

${DLCBUILDDIR}/snusr_quant.stamp: \
        ${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
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
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-quant \
			--input_dlc=/output/snusr_float.dlc \
			--input_list=snusr_calibration_list.txt \
			--output_dlc=/output/snusr_quant.dlc \
	#SNUSR offline prepare completed
	touch $@

${DLCBUILDDIR}/snusr_htp_sm8750.stamp: \
        ${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/snusr_quant.stamp \
		${DLCBUILDDIR}/snusr_calibration_list.txt
	#HTP Graph prepare of SNUSR DLC
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
            --htp_dlbc=true \
			--input_dlc=/output/snusr_quant.dlc \
			--output_dlc=/output/snusr_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm8750 \
			--memorymapped_buffer_hint=true
	#HTP Graph prepare of SNUSR model completed
	cp	${DLCBUILDDIR}/snusr_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/snusr_htp_sm7550.stamp: \
        ${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
	#HTP Graph prepare of SNUSR DLC
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
            --htp_dlbc=true \
			--input_dlc=/output/snusr_htp.dlc \
			--output_dlc=/output/snusr_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm7550,sm7635 \
			--memorymapped_buffer_hint=true
	#HTP Graph prepare of SNUSR model completed
	cp	${DLCBUILDDIR}/snusr_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/snusr_htp.stamp: \
        ${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
        ${DLCBUILDDIR}/snusr_htp_sm8750.stamp \
        ${DLCBUILDDIR}/snusr_htp_sm7550.stamp \
	#HTP Graph prepare of SNUSR DLC
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
            --htp_dlbc=true \
			--input_dlc=/output/snusr_htp.dlc \
			--output_dlc=/output/snusr_htp.dlc \
			--optimization_level 3 \
			--htp_socs=sm8650,sm8735,sm8635,sm7325,sm8350,sm8450,sm8550,sm7475,sc8380xp,sc8280x \
			--memorymapped_buffer_hint=true
	#HTP Graph prepare of SNUSR model completed
	cp	${DLCBUILDDIR}/snusr_htp.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/snusr_htp_O2_sm8750.stamp: \
        ${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/snusr_quant.stamp \
		${DLCBUILDDIR}/snusr_calibration_list.txt
	#HTP Graph prepare of SNUSR DLC
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=6 \
			--input_dlc=/output/snusr_quant.dlc \
			--output_dlc=/output/snusr_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8750 \
			--memorymapped_buffer_hint=true
	#HTP Graph prepare of SNUSR model completed
	cp	${DLCBUILDDIR}/snusr_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/snusr_htp_O2_sm7550.stamp: \
        ${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
	#HTP Graph prepare of SNUSR DLC
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=2 \
			--input_dlc=/output/snusr_quant.dlc \
			--output_dlc=/output/snusr_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm7550,sm7635 \
			--memorymapped_buffer_hint=true
	#HTP Graph prepare of SNUSR model completed
	cp	${DLCBUILDDIR}/snusr_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/snusr_htp_O2.stamp: \
        ${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
        ${DLCBUILDDIR}/snusr_htp_O2_sm8750.stamp \
        ${DLCBUILDDIR}/snusr_htp_O2_sm7550.stamp \
	#HTP Graph prepare of SNUSR DLC
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--num_hvx_threads=4 \
			--input_dlc=/output/snusr_htp_O2.dlc \
			--output_dlc=/output/snusr_htp_O2.dlc \
			--optimization_level 2 \
			--htp_socs=sm8650,sm8735,sm8635,sm7325,sm8350,sm8450,sm8550,sm7475,sc8380xp,sc8280x \
			--memorymapped_buffer_hint=true
	#HTP Graph prepare of SNUSR model completed
	cp	${DLCBUILDDIR}/snusr_htp_O2.dlc ${MLPERF_MODELS_PATH}
	touch $@

${DLCBUILDDIR}/sd_precompute_data.tar: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
	# Preparing sd_precompute_data.tar
	docker run \
		-v ${SNPE_SDK}:/qnn_sdk \
		-v ${TOPDIR}/mobile_back_qti/DLC/util/:/util \
        -e PYTHONPATH=/qnn_sdk/lib/python \
		-e LD_LIBRARY_PATH=/usr/local/clang-9.0.0/lib:qnn_sdk/lib/x86_64-linux-clang:${LD_LIBRARY_PATH} \
		-v ${DLCBUILDDIR}:/output \
        -v ${DLCBUILDDIR}/stable_diffusion/:/stable_diffusion \
        -w /stable_diffusion \
		mlperf_dlc_prepare \
		python3 /util/StableDiffusion/flatten.py \
			--random_latent_init /stable_diffusion/random_latent_init/random_init_1.pkl \
            --time_step_embedding /stable_diffusion/time_step_embeddings/unet_time_step_embeddings_20.pkl \
            --time_step_list /stable_diffusion/scheduler/scheduler_time_steps_20.pkl \
            --unconditional_text_emb /stable_diffusion/unconditional_text_emb.pkl \
            --dumpdir /stable_diffusion/.
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion
	cp ${DLCBUILDDIR}/stable_diffusion/sd_precompute_data.tar ${MLPERF_MODELS_PATH}/stable_diffusion/.
	cp ${DLCBUILDDIR}/stable_diffusion/scheduler/lambdas.bin ${MLPERF_MODELS_PATH}/stable_diffusion/.
	cp ${DLCBUILDDIR}/stable_diffusion/scheduler/betas.bin ${MLPERF_MODELS_PATH}/stable_diffusion/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion

${DLCBUILDDIR}/text_encoder_qnn.cpp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# TEXT-ENCODER conversion and quantization
    # cpp & bin files
	docker run \
		-v ${SNPE_SDK}:/qnn_sdk \
		-v ${TEXTENCODER_MODEL_PATH}:/models \
        -e PYTHONPATH=/qnn_sdk/lib/python \
		-e LD_LIBRARY_PATH=/usr/local/clang-9.0.0/lib:qnn_sdk/lib/x86_64-linux-clang:${LD_LIBRARY_PATH} \
		-v ${DLCBUILDDIR}:/output \
        -v ${DLCBUILDDIR}/stable_diffusion/:/stable_diffusion \
        -w /stable_diffusion/text_encoder \
        -v ${TOPDIR}:/dir \
		mlperf_dlc_prepare \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-onnx-converter \
			--input_network text_encoder_onnx/text_encoder.onnx \
			--input_list stable_diffusion_models/text_encoder_onnx/text_encoder_input_list.txt \
			--act_bitwidth 16 \
			--bias_bitwidth 32 \
			--quantization_overrides text_encoder_onnx/text_encoder.encodings \
			--output_path /output/text_encoder.cpp

${DLCBUILDDIR}/text_encoder_qnn_model_generator.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/text_encoder_qnn.cpp
	# TEXT-ENCODER lib generation started
	docker run \
    	-v ${SNPE_SDK}:/qnn_sdk \
        -e PYTHONPATH=/qnn_sdk/lib/python \
    	-e LD_LIBRARY_PATH=/usr/local/clang-9.0.0/lib:qnn_sdk/lib/x86_64-linux-clang:${LD_LIBRARY_PATH} \
    	-v ${DLCBUILDDIR}:/output \
    	-u ${USERID}:${GROUPID} \
        mlperf_dlc_prepare \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-model-lib-generator \
			-c /output/text_encoder.cpp \
			-b /output/text_encoder.bin \
			-o /output/model_libs \
			-t x86_64-linux-clang
	# Text-encoder lib generation completed

${DLCBUILDDIR}/text_encoder_qnn_context_binary_generator.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/text_encoder_qnn_model_generator.stamp
	# TEXT-ENCODER context-binary generation started
	docker run \
    	-v ${SNPE_SDK}:/qnn_sdk \
    	-v ${TEXTENCODER_MODEL_PATH}:/models \
        -e PYTHONPATH=/qnn_sdk/lib/python \
    	-e LD_LIBRARY_PATH=/usr/local/clang-9.0.0/lib:qnn_sdk/lib/x86_64-linux-clang:${LD_LIBRARY_PATH} \
    	-v ${DLCBUILDDIR}:/output \
    	-v ${DLCBUILDDIR}/stable_diffusion/:/stable_diffusion \
    	-w /stable_diffusion/text_encoder \
    	-u ${USERID}:${GROUPID} \
        mlperf_dlc_prepare \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libtext_encoder.so \
			--binary_file /output/text_encoder.serialized \
			--config_file mcp_config.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion
	cp ${DLCBUILDDIR}/text_encoder.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion
	chmod -R 777 ${DLCBUILDDIR}/stable_diffusion
	# TEXT-ENCODER context binary generation completed

${DLCBUILDDIR}/vae_decoder_qnn.cpp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# VAE-DECODER conversion and quantization
    # cpp & bin files
	docker run \
		-v ${SNPE_SDK}:/qnn_sdk \
		-v ${VAEDECODER_MODEL_PATH}:/models \
        -e PYTHONPATH=/qnn_sdk/lib/python \
		-e LD_LIBRARY_PATH=/usr/local/clang-9.0.0/lib:qnn_sdk/lib/x86_64-linux-clang:${LD_LIBRARY_PATH} \
		-v ${DLCBUILDDIR}:/output \
        -v ${DLCBUILDDIR}/stable_diffusion/:/stable_diffusion \
        -w /stable_diffusion/vae_decoder \
        -v ${TOPDIR}:/dir \
		mlperf_dlc_prepare \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-onnx-converter \
			--input_network vae_decoder_onnx/vae_decoder.onnx \
			--input_list stable_diffusion_models/vae_decoder_onnx/vae_decoder_input_list.txt  \
			--act_bitwidth 16 \
			--bias_bitwidth 32 \
			--quantization_overrides vae_decoder_onnx/vae_decoder.encodings \
			--output_path /output/vae_decoder.cpp

${DLCBUILDDIR}/vae_decoder_qnn_model_generator.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/vae_decoder_qnn.cpp
	# VAE-DECODER lib generation started
	docker run \
    	-v ${SNPE_SDK}:/qnn_sdk \
        -e PYTHONPATH=/qnn_sdk/lib/python \
    	-e LD_LIBRARY_PATH=/usr/local/clang-9.0.0/lib:qnn_sdk/lib/x86_64-linux-clang:${LD_LIBRARY_PATH} \
    	-v ${DLCBUILDDIR}:/output \
    	-u ${USERID}:${GROUPID} \
        mlperf_dlc_prepare \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-model-lib-generator \
			-c /output/vae_decoder.cpp \
			-b /output/vae_decoder.bin \
			-o /output/model_libs \
			-t x86_64-linux-clang
	# vae-decoder lib generation completed

${DLCBUILDDIR}/vae_decoder_qnn_context_binary_generator.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/vae_decoder_qnn_model_generator.stamp
	# VAE-DECODER lib generation started
	docker run \
    	-v ${SNPE_SDK}:/qnn_sdk \
    	-v ${VAEDECODER_MODEL_PATH}:/models \
        -e PYTHONPATH=/qnn_sdk/lib/python \
    	-e LD_LIBRARY_PATH=/usr/local/clang-9.0.0/lib:qnn_sdk/lib/x86_64-linux-clang:${LD_LIBRARY_PATH} \
    	-v ${DLCBUILDDIR}:/output \
    	-u ${USERID}:${GROUPID} \
        mlperf_dlc_prepare \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libvae_decoder.so \
			--binary_file /output/vae_decoder.serialized \
			--config_file /models/mcp_config.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion
	cp ${DLCBUILDDIR}/vae_decoder.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion
	chmod -R 777 ${DLCBUILDDIR}/stable_diffusion
	# VAE context binary generation completed

${DLCBUILDDIR}/unet_qnn.cpp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/mlperf_models.stamp
	# UNET conversion and quantization
    # cpp & bin files
	docker run \
		-v ${SNPE_SDK}:/qnn_sdk \
		-v ${UNET_MODEL_PATH}:/models \
        -e PYTHONPATH=/qnn_sdk/lib/python \
		-e LD_LIBRARY_PATH=/usr/local/clang-9.0.0/lib:qnn_sdk/lib/x86_64-linux-clang:${LD_LIBRARY_PATH} \
		-v ${DLCBUILDDIR}:/output \
        -v ${DLCBUILDDIR}/stable_diffusion/:/stable_diffusion \
        -w /stable_diffusion/unet \
        -v ${TOPDIR}:/dir \
		mlperf_dlc_prepare \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-onnx-converter \
			--input_network unet_onnx_batch_1/unet.onnx \
			-l input_3 NONTRIVIAL \
			--input_list stable_diffusion_models/unet_onnx/unet_input_list.txt \
			--act_bitwidth 16 \
			--bias_bitwidth 32 \
			--quantization_overrides unet_onnx_batch_1/unet.encodings \
			--output_path /output/unet.cpp

${DLCBUILDDIR}/unet_qnn_model_generator.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/unet_qnn.cpp
	# UNET lib generation started
	docker run \
    	-v ${SNPE_SDK}:/qnn_sdk \
        -e PYTHONPATH=/qnn_sdk/lib/python \
    	-e LD_LIBRARY_PATH=/usr/local/clang-9.0.0/lib:qnn_sdk/lib/x86_64-linux-clang:${LD_LIBRARY_PATH} \
    	-v ${DLCBUILDDIR}:/output \
    	-u ${USERID}:${GROUPID} \
        mlperf_dlc_prepare \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-model-lib-generator \
			-c /output/unet.cpp \
			-b /output/unet.bin \
			-o /output/model_libs \
			-t x86_64-linux-clang \
			-t aarch64-android
	# UNET lib generation completed

${DLCBUILDDIR}/unet_qnn_context_binary_generator.stamp: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/mobile/.stamp \
		${DLCBUILDDIR}/unet_qnn_model_generator.stamp
	# UNET context binary generation started
	docker run \
    	-v ${SNPE_SDK}:/qnn_sdk \
    	-v ${UNET_MODEL_PATH}:/models \
        -e PYTHONPATH=/qnn_sdk/lib/python \
    	-e LD_LIBRARY_PATH=/usr/local/clang-9.0.0/lib:qnn_sdk/lib/x86_64-linux-clang:${LD_LIBRARY_PATH} \
    	-v ${DLCBUILDDIR}:/output \
    	-u ${USERID}:${GROUPID} \
        mlperf_dlc_prepare \
		/qnn_sdk/bin/x86_64-linux-clang/qnn-context-binary-generator \
			--backend /qnn_sdk/lib/x86_64-linux-clang/libQnnHtp.so \
			--model /output/model_libs/x86_64-linux-clang/libunet.so \
			--binary_file /output/unet.serialized \
			--config_file /models/mcp_config.json
	mkdir -p ${MLPERF_MODELS_PATH}/stable_diffusion
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion
	cp ${DLCBUILDDIR}/unet.serialized.bin ${MLPERF_MODELS_PATH}/stable_diffusion/.
	chmod -R 777 ${MLPERF_MODELS_PATH}/stable_diffusion
	chmod -R 777 ${DLCBUILDDIR}/stable_diffusion
	# UNET context binary generation completed

####################################################################################
# CALIBRATION / QUANTIZATION
####################################################################################

# OpenSR calibration data preprocessing
${DLCBUILDDIR}/snusr_calibration_list.txt: \
			${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
			${DATASETS}/output/state/snusr_calibration.stamp
	# Preparing SNUSR dataset
	mkdir -p ${DATASETS}/output/snusr/_raws
	docker run \
		-v ${TOPDIR}/mobile_back_qti/DLC/util/:/util \
		-v ${DATASETS}/output/snusr/:/snusr \
		-v ${SNUSR_CALIBRATION_PATH}:/calibration \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		python3 /util/snusr/rgb8_to_raw.py /snusr/ /calibration/OpenImages_calibration_rgb8.txt
	rm -rf ${DLCBUILDDIR}/snusr/_raws
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

# Imagenet 384x384 calibration data preprocessing
${DLCBUILDDIR}/imagenet/state/resized_384.stamp: \
		${DATASETS_OUT}/state/calibration.stamp \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp
	# Scaling Imagenet images to 384x384
	rm -rf ${DLCBUILDDIR}/imagenet/resized_384
	mkdir -p ${DLCBUILDDIR}/imagenet/resized_384
	docker run \
		-v ${TOPDIR}/datasets/util:/util \
		-v ${CALIBRATION_DATA}/imagenet:/imagenet \
		-v ${DLCBUILDDIR}:/output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
	/bin/bash -c "python3 /util/imagenet/Resize384.py /imagenet/images /output/imagenet/resized_384"
	mkdir -p ${DLCBUILDDIR}/imagenet/state
	touch $@

${DLCBUILDDIR}/imagenet/state/quantdata_384.stamp: \
		${DLCBUILDDIR}/imagenet/state/resized_384.stamp \
		${DLCBUILDDIR}/mlperf_mobile_docker_1_1.stamp
	# Generating Imagenet quantization data for SNPE
	rm -rf ${DLCBUILDDIR}/imagenet/resized_384_raw
	mkdir -p ${DLCBUILDDIR}/imagenet/resized_384_raw
	docker run \
		-v ${TOPDIR}/datasets/util:/util \
		-v ${DLCBUILDDIR}/imagenet:/imagenet \
		-u ${USERID}:${GROUPID} \
		mlperf_mobile:1.1 \
		/bin/bash -c "python3 /util/common/jpg_to_raw.py /imagenet/resized_384"
	rm -rf ${DLCBUILDDIR}/imagenet/quantdata_384
	mv ${DLCBUILDDIR}/imagenet/resized_384_raw ${DLCBUILDDIR}/imagenet/quantdata_384
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
		mlperf_dlc_prepare \
		/bin/bash -c '\
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobilenet_v4_htp.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/ssd_mobiledet_qat_htp.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobilenet_v4_htp_batched_4.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobile_mosaic_htp.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/snusr_htp.dlc && \
			/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-info -i /dlc/mobilebert_quantized_htp.dlc'

####################################################################################
# Clean
####################################################################################
clean:
	rm -rf ${BUILDDIR}/DLC

