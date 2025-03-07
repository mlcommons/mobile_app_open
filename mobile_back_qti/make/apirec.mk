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

####################################################################################
# APIREC GENERATION
####################################################################################

${DLCBUILDDIR}/QnnHtpDebug.conf:
	# Replace the libHtpPrepare.so in your SNPE with libQnnHtp.so of QNN debug build with similar tag as snpe build
	# Rename the copied libQnnHtp.so to libHtpPrepare.so
	# Manually copy QnnHtpDebug.conf from your <qnn_src>/build folder to the folder where DLC is present
	# Detailed instructions are present in this link https://confluence.qualcomm.com/confluence/display/~yuhuchua/Generate+apirec+from+SNPE+.dlc+model
	false

${DLCBUILDDIR}/generate_mobilenet_edgetpu_batched_4_apirec: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/QnnHtpDebug.conf
	#Generating Apirec for input DLC in /output path
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-e PATH=/snpe_sdk/bin/x86_64-linux-clang:$PATH \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-w /output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc /output/mobilenet_edgetpu_224_1.0_htp_batched_4.dlc \
			--overwrite_cache_records || true
	# Mobilenet_edgetpu_batched_4 apirec generation completed
	mv	${DLCBUILDDIR}/QnnGraph_htpcore.apirec ${DLCBUILDDIR}/mobilenet_edgetpu_batched_4.apirec

${DLCBUILDDIR}/generate_mobilenet_edgetpu_batched_3_apirec: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/QnnHtpDebug.conf
	#Generating Apirec for input DLC in /output path
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-e PATH=/snpe_sdk/bin/x86_64-linux-clang:$PATH \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-w /output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc /output/mobilenet_edgetpu_224_1.0_htp_batched_3.dlc \
			--overwrite_cache_records || true
	# Mobilenet_edgetpu_batched_3 apirec generation completed
	mv	${DLCBUILDDIR}/QnnGraph_htpcore.apirec ${DLCBUILDDIR}/mobilenet_edgetpu_batched_3.apirec

${DLCBUILDDIR}/generate_mobilenet_edgetpu_batched_8_apirec: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/QnnHtpDebug.conf
	#Generating Apirec for input DLC in /output path
	# Apirec generation completed
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-e PATH=/snpe_sdk/bin/x86_64-linux-clang:$PATH \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-w /output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc /output/mobilenet_edgetpu_224_1.0_htp_batched_8.dlc \
			--overwrite_cache_records || true
	# Mobilenet_edgetpu_batched_8 apirec generation completed
	mv	${DLCBUILDDIR}/QnnGraph_htpcore.apirec ${DLCBUILDDIR}/mobilenet_edgetpu_batched_8.apirec

${DLCBUILDDIR}/generate_mobilenet_v4_batched_4_apirec: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/QnnHtpDebug.conf
	#Generating Apirec for input DLC in /output path
	# Apirec generation completed
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-e PATH=/snpe_sdk/bin/x86_64-linux-clang:$PATH \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-w /output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc /output/mobilenet_v4_htp_batched_4.dlc \
			--overwrite_cache_records || true
	# MobilenetV4_batched_4 apirec generation completed
	mv	${DLCBUILDDIR}/QnnGraph_htpcore.apirec ${DLCBUILDDIR}/mobilenet_v4_batched_4.apirec

${DLCBUILDDIR}/generate_mobilenet_edgetpu_apirec: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/QnnHtpDebug.conf
	#Generating Apirec for input DLC in /output path
	# Apirec generation completed
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-e PATH=/snpe_sdk/bin/x86_64-linux-clang:$PATH \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-w /output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc /output/mobilenet_edgetpu_224_1.0_htp.dlc \
			--overwrite_cache_records || true
	# Mobilenet_edgetpu apirec generation completed
	mv	${DLCBUILDDIR}/QnnGraph_htpcore.apirec ${DLCBUILDDIR}/mobilenet_edgetpu.apirec

${DLCBUILDDIR}/generate_mobilenet_v4_apirec: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/QnnHtpDebug.conf
	#Generating Apirec for input DLC in /output path
	# Apirec generation completed
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-e PATH=/snpe_sdk/bin/x86_64-linux-clang:$PATH \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-w /output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc /output/mobilenet_v4_htp.dlc \
			--overwrite_cache_records || true
	# MobilenetV4 apirec generation completed
	mv	${DLCBUILDDIR}/QnnGraph_htpcore.apirec ${DLCBUILDDIR}/mobilenet_v4.apirec

${DLCBUILDDIR}/generate_mosaic_apirec: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/QnnHtpDebug.conf
	#Generating Apirec for input DLC in /output path
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-e PATH=/snpe_sdk/bin/x86_64-linux-clang:$PATH \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-w /output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc /output/mobile_mosaic_htp.dlc \
			--overwrite_cache_records || true
	# Mosaic apirec generation completed
	mv	${DLCBUILDDIR}/QnnGraph_htpcore.apirec ${DLCBUILDDIR}/mosaic.apirec

${DLCBUILDDIR}/generate_mobiledet_apirec: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/QnnHtpDebug.conf
	#Generating Apirec for input DLC in /output path
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-e PATH=/snpe_sdk/bin/x86_64-linux-clang:$PATH \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-w /output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc /output/ssd_mobiledet_qat_htp.dlc \
			--overwrite_cache_records || true
	# Mobiledet apirec generation completed
	mv	${DLCBUILDDIR}/QnnGraph_htpcore.apirec ${DLCBUILDDIR}/mobiledet.apirec

${DLCBUILDDIR}/generate_mobilebert_apirec: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/QnnHtpDebug.conf
	#Generating Apirec for input DLC in /output path
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-e PATH=/snpe_sdk/bin/x86_64-linux-clang:$PATH \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-w /output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc /output/mobilebert_quantized_htp.dlc\
			--overwrite_cache_records || true
	# Mobilebert apirec generation completed
	mv ${DLCBUILDDIR}/QnnGraph_htpcore.apirec ${DLCBUILDDIR}/mobilebert.apirec

${DLCBUILDDIR}/generate_snusr_apirec: \
		${DLCBUILDDIR}/mlperf_dlc_prepare_docker.stamp \
		${DLCBUILDDIR}/QnnHtpDebug.conf
	#Generating Apirec for input DLC in /output path
	docker run \
		-e PYTHONPATH=/snpe_sdk/lib/python \
		-e LD_LIBRARY_PATH=/snpe_sdk/lib/x86_64-linux-clang \
		-e PATH=/snpe_sdk/bin/x86_64-linux-clang:$PATH \
		-v ${SNPE_SDK}:/snpe_sdk \
		-v ${DLCBUILDDIR}:/output \
		-w /output \
		-u ${USERID}:${GROUPID} \
		mlperf_dlc_prepare \
		/snpe_sdk/bin/x86_64-linux-clang/snpe-dlc-graph-prepare \
			--input_dlc /output/snusr_htp.dlc\
			--overwrite_cache_records || true
	# Snusr apirec generation completed
	mv	${DLCBUILDDIR}/QnnGraph_htpcore.apirec ${DLCBUILDDIR}/snusr.apirec

