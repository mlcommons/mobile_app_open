# Copyright 2020-2021 The MLPerf Authors. All Rights Reserved.
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


all: flutter

# avaiable backends
WITH_TFLITE?=1
WITH_QTI?=0
WITH_SAMSUNG?=0
WITH_PIXEL?=0
WITH_MEDIATEK?=0

include tools/common.mk
include tools/formatter/format.mk

include mobile_back_tflite/tflite_backend.mk
include mobile_back_samsung/samsung_backend.mk
include mobile_back_qti/make/qti_backend.mk
include mobile_back_pixel/pixel_backend.mk

include flutter/flutter.mk

.PHONY: clean
clean:
	([ -d output/home/mlperf/cache ] && chmod -R +w output/home/mlperf/cache) || true
	rm -rf output
