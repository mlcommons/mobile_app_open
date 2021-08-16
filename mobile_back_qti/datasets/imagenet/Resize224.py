#!/bin/bash
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

from PIL import Image
import os
for f in os.listdir(os.sys.argv[1]):
  img = Image.open(os.sys.argv[1]+"/"+f)
  img256 = img.resize((256, 256), Image.ANTIALIAS)

  # Seems like the required algo is to resize to 256x256
  # Then center crop a 224x244 image
  left = (256 - 224)/2
  top = (256 - 224)/2
  right = (256 + 224)/2
  bottom = (256 + 224)/2

  # Crop 224x224 image from the center
  img224 = img256.crop((left, top, right, bottom))
  img224.save(os.sys.argv[2]+"/"+f)
