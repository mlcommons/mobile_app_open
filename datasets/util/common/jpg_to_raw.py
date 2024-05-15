#!/bin/bash
# Copyright (c) 2020-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
DATA_PATH = model_name
RAW_DATA_PATH = model_name + "_raw"

image_list = [f for f in listdir(DATA_PATH) if isfile(join(DATA_PATH, f))]
for image in image_list:
    src = DATA_PATH + '/' + image
    dst = RAW_DATA_PATH + '/' + image
    with open(src, 'rb') as f:
        jpeg_str = f.read()
    original_im = Image.open(BytesIO(jpeg_str))
    converted_image = original_im.convert('RGB')
    npimage = np.asarray(converted_image).astype(np.float32)
    npimage = npimage * 0.00784313771874
    npimage = npimage - 1.0
    img_ndarray = np.array(npimage)
    tmp = dst.split(".")
    tmp[-1] = "raw"
    f_name = ".".join(tmp)
    npimage.astype(np.float32).tofile(f_name)
