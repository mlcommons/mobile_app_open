# Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

this_mkfile:=$(abspath $(lastword $(MAKEFILE_LIST)))
AIMETBUILDDIR:=$(abspath $(shell dirname ${this_mkfile}))
TOPDIR:=${AIMETBUILDDIR}/../../../../..

.PHONY: clean aimet_calibration

${AIMETBUILDDIR}/nvidia_docker_runtime.stamp: \
	### Executing nvidia docker container
	curl -fsSL https\://nvidia.github.io/libnvidia-container/gpgkey | sudo gpg --dearmor -o /usr/share/keyrings/nvidia-container-toolkit-keyring.gpg \
    && curl -s -L https\://nvidia.github.io/libnvidia-container/stable/deb/nvidia-container-toolkit.list | sed 's#deb https://#deb [signed-by=/usr/share/keyrings/nvidia-container-toolkit-keyring.gpg] https://#g' | sudo tee /etc/apt/sources.list.d/nvidia-container-toolkit.list
	sed -i -e '/experimental/ s/^#//g' /etc/apt/sources.list.d/nvidia-container-toolkit.list
	sudo apt-get update
	sudo apt-get install -y nvidia-container-toolkit
	sudo nvidia-ctk runtime configure --runtime=docker
	sudo systemctl restart docker
	touch $@

aimet_calibration: \
	clean \
	${AIMETBUILDDIR}/aimet_calibration.stamp \
	${AIMETBUILDDIR}/input_vectors_generation.stamp \
	copy_files_to_ouptut \
	copy_configs_to_ouptut

${AIMETBUILDDIR}/aimet_torch_gpu_docker.stamp: \
	## Building aimet-docker
	docker image build -t aimet_torch_gpu_docker .
	touch $@

${AIMETBUILDDIR}/aimet_calibration.stamp: \
		${AIMETBUILDDIR}/nvidia_docker_runtime.stamp \
		${AIMETBUILDDIR}/aimet_torch_gpu_docker.stamp
	# Executing docker instance
	docker run \
		-v /etc/localtime:/etc/localtime:ro \
        -v /etc/timezone:/etc/timezone:ro \
        -v ${AIMETBUILDDIR}:/tmp \
        -w /tmp/example1 \
		--network=host \
		--ulimit core=-1 \
		--ipc=host \
		--shm-size=8G \
		--cap-add=SYS_PTRACE \
		--security-opt seccomp=unconfined \
		aimet_torch_gpu_docker \
		/bin/bash -c "cp -rv ../aimet.py . && python3 aimet.py"

${AIMETBUILDDIR}/input_vectors_generation.stamp: \
		${AIMETBUILDDIR}/aimet_torch_gpu_docker.stamp
	# Creates input vectors to be passed during conversion
	docker run \
		-v /etc/localtime:/etc/localtime:ro \
        -v /etc/timezone:/etc/timezone:ro \
        -v ${AIMETBUILDDIR}:/tmp \
        -w /tmp \
		--network=host \
		--ulimit core=-1 \
		--ipc=host \
		--shm-size=8G \
		--cap-add=SYS_PTRACE \
		--security-opt seccomp=unconfined \
		aimet_torch_gpu_docker \
		/bin/bash -c "python3 example2/generate_inputs.py --pickle_path example1/_exports_/fp32.npy --working_dir ."

copy_files_to_ouptut:
	#Copies generated artifacts to output/DLC/stable_diffusion
	chmod -R 777 ${TOPDIR}/output/DLC/
	mkdir -p ${TOPDIR}/output/DLC/stable_diffusion
	cp -rv ${AIMETBUILDDIR}/example1/_exports_/* ${TOPDIR}/output/DLC/stable_diffusion/.
	cp -rv ${AIMETBUILDDIR}/stable_diffusion_models ${TOPDIR}/output/DLC/stable_diffusion/.

copy_configs_to_ouptut:
	#Copies configs to output
	mkdir -p ${TOPDIR}/output/DLC/stable_diffusion/text_encoder
	mkdir -p ${TOPDIR}/output/DLC/stable_diffusion/vae_decoder
	mkdir -p ${TOPDIR}/output/DLC/stable_diffusion/unet
	mv ${TOPDIR}/output/DLC/stable_diffusion/text_encoder_onnx ${TOPDIR}/output/DLC/stable_diffusion/text_encoder/.
	mv ${TOPDIR}/output/DLC/stable_diffusion/vae_decoder_onnx ${TOPDIR}/output/DLC/stable_diffusion/vae_decoder/.
	mv ${TOPDIR}/output/DLC/stable_diffusion/unet_onnx_batch_1 ${TOPDIR}/output/DLC/stable_diffusion/unet/.
	cp -rv ${AIMETBUILDDIR}/graph_config_text_encoder.json ${TOPDIR}/output/DLC/stable_diffusion/text_encoder/graph_config.json
	cp -rv ${AIMETBUILDDIR}/mcp_config.json ${TOPDIR}/output/DLC/stable_diffusion/text_encoder/.
	cp -rv ${AIMETBUILDDIR}/graph_config_vae.json ${TOPDIR}/output/DLC/stable_diffusion/vae_decoder/graph_config.json
	cp -rv ${AIMETBUILDDIR}/mcp_config.json ${TOPDIR}/output/DLC/stable_diffusion/vae_decoder/.
	cp -rv ${AIMETBUILDDIR}/graph_config_unet.json ${TOPDIR}/output/DLC/stable_diffusion/unet/graph_config.json
	cp -rv ${AIMETBUILDDIR}/mcp_config.json ${TOPDIR}/output/DLC/stable_diffusion/unet/.
	mkdir -p ${TOPDIR}/output/DLC/stable_diffusion/text_encoder/stable_diffusion_models
	cp -rv ${AIMETBUILDDIR}/stable_diffusion_models/text_encoder_onnx ${TOPDIR}/output/DLC/stable_diffusion/text_encoder/stable_diffusion_models/.
	mkdir -p ${TOPDIR}/output/DLC/stable_diffusion/vae_decoder/stable_diffusion_models
	cp -rv ${AIMETBUILDDIR}/stable_diffusion_models/vae_decoder_onnx ${TOPDIR}/output/DLC/stable_diffusion/vae_decoder/stable_diffusion_models/.
	mkdir -p ${TOPDIR}/output/DLC/stable_diffusion/unet/stable_diffusion_models
	cp -rv ${AIMETBUILDDIR}/stable_diffusion_models/unet_onnx ${TOPDIR}/output/DLC/stable_diffusion/unet/stable_diffusion_models/.
	chmod -R 777 ${TOPDIR}/output/DLC/stable_diffusion

clean:
	# Removes all generated outputs (except example1/_exports_) from AIMET and DLC/stable_diffusion
	rm -rf ${AIMETBUILDDIR}/*.stamp
	rm -rf ${AIMETBUILDDIR}/example1/*.png
	rm -rf ${AIMETBUILDDIR}/stable_diffusion_models
	rm -rf ${TOPDIR}/output/DLC/stable_diffusion/*

deep_clean:
	# Caution:: Removes all generated outputs (including example1/_exports_) from AIMET and DLC/stable_diffusion
	rm -rf ${AIMETBUILDDIR}/*.stamp
	rm -rf ${AIMETBUILDDIR}/example1/_exports_
	rm -rf ${AIMETBUILDDIR}/example1/*.png
	rm -rf ${AIMETBUILDDIR}/stable_diffusion_models
	rm -rf ${TOPDIR}/output/DLC/stable_diffusion/*