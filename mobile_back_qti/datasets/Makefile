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

include ../make/builddir.mk

all: ${BUILDDIR}/datasets.stamp

dependencies:
	@[ -f "${BUILDDIR}/imagenet/downloads/LSVRC2012_img_val.tar" ] || (echo "Download imagenet validation data (LSVRC2012_img_val.tar) and move it to ${BUILDDIR}/imagenet/downloads/LSVRC2012_img_val.tar" && false)

${BUILDDIR}/datasets.stamp: dependencies ${BUILDDIR}/ade20k/ade20k_out/.stamp \
		${BUILDDIR}/coco/coco-out/.stamp \
		${BUILDDIR}/imagenet/imagenet-out/.stamp \
		${BUILDDIR}/SQUAD/SQUAD_out/.stamp
	@touch $@

# ADE20K
${BUILDDIR}/ade20k/ade20k_out/.stamp:
	@(cd ade20k && make)

# Coco
${BUILDDIR}/coco/coco-out/.stamp:
	@(cd coco && make)

# Imagenet
${BUILDDIR}/imagenet/imagenet-out/.stamp:
	@(cd imagenet && make)

# SQUAD
${BUILDDIR}/SQUAD/SQUAD_out/.stamp:
	@(cd squad && make)

clean:
	@(cd ade20k && make clean)
	@(cd coco && make clean)
	@(cd imagenet && make clean)
	@(cd squad && make clean)
	@rm ${BUILDDIR}/datasets.stamp

clean_downloads:
	@(cd ade20k && make clean_downloads)
	@(cd coco && make clean_downloads)
	@(cd imagenet && make clean_downloads)
	@(cd squad && make clean_downloads)
