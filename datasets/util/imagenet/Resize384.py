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

from PIL import Image
import os
for f in os.listdir(os.sys.argv[1]):
    img = Image.open(os.sys.argv[1]+"/"+f)
    img438 = img.resize((438, 438), Image.Resampling.BILINEAR)
    # Resize to 438x438
    # Then center crop a 384x384 image
    left = (438 - 384)/2
    top = (438 - 384)/2
    right = (438 + 384)/2
    bottom = (438 + 384)/2
    # Crop 384x384 image from the center
    img384 = img438.crop((left, top, right, bottom))
    img384.save(os.sys.argv[2]+"/"+f)