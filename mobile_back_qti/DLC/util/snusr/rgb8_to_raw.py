#!/bin/bash
# Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

import os 
import sys
import numpy as np
from os import listdir
from os.path import isfile, join, isdir
from io import BytesIO
from PIL import Image

model_name = sys.argv[1]
calibration_path = sys.argv[2]
DATA_PATH = model_name
RAW_DATA_PATH = model_name + "_raws"

calibration_list = []
with open(calibration_path, 'r') as g:
	calibration_list = [line.rstrip() for line in g.readlines()]
image_list = [f for f in calibration_list if isfile(join(DATA_PATH, f))]
for image in image_list:
    src = DATA_PATH + '/' + image
    dst = RAW_DATA_PATH + '/' + image
    with open(src, 'rb') as f:
        path_str = np.fromfile(f,np.uint8).astype(np.float32)
    npimage = np.asarray(path_str).astype(np.float32)
    img_ndarray = np.array(npimage)
    f_name = dst[:-4] + '.raw'
    npimage.astype(np.float32).tofile(f_name)
