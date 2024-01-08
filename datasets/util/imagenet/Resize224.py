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

def center_crop(img):
  frac=0.875
  left = img.size[0]*((1-frac)/2)
  upper = img.size[1]*((1-frac)/2)
  right = img.size[0]-((1-frac)/2)*img.size[0]
  bottom = img.size[1]-((1-frac)/2)*img.size[1]
  img = img.crop((left, upper, right, bottom))
  return img

for f in os.listdir(os.sys.argv[1]):
  print(os.sys.argv[1]+"/"+f)
  img = Image.open(os.sys.argv[1]+"/"+f)
  # center crop
  img = center_crop(img)
  img = img.resize((384, 384), resample= Image.Resampling.BILINEAR)
  img.save(os.sys.argv[2]+"/"+f)