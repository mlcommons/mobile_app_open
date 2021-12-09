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

.PHONY: libqtibackend
libqtibackend: android/builder-image
	# Building libqtibackend
	mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	mkdir -p output/mobile_back_qti
	docker run \
		${ANDROID_COMMON_DOCKER_FLAGS} \
		--config android_arm64 //mobile_back_qti:qtibackend
	cp output/`readlink bazel-bin`/mobile_back_qti/cpp/backend_qti/libqtibackend.so output/mobile_back_qti/libtqtibackend.so

# You need libasan5 installed to run allocator_test (sudo apt install libasan5)
qti_allocator_test: output/mobile_back_qti/test/allocator_test
output/mobile_back_qti/test/allocator_test: docker_image
	# Building QTI allocator_test
	mkdir -p output/home/mlperf/cache && chmod 777 output/home/mlperf/cache
	mkdir -p output/mobile_back_qti/test
	docker run \
		${ANDROID_NATIVE_DOCKER_FLAGS} --experimental_repo_remote_exec \
		--config=asan \
		//mobile_back_qti/cpp/backend_qti:allocator_test
	cp output/`readlink bazel-out`/k8-opt/bin/mobile_back_qti/cpp/backend_qti/allocator_test $@
	chmod 777 $@

