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

import os
import sys
import fnmatch
import tensorflow as tf
import deeplab.input_preprocess
from PIL import Image as Image

tf.enable_eager_execution()

ADE20K_PATH = sys.argv[1]
OUTPUT_PATH = sys.argv[2]
FILELIST = sys.argv[3]

files = open(FILELIST).read().split("\n")[:-1]
for fname in files:
  found = False
  for dirpath, dirs, files in os.walk(ADE20K_PATH):  
    for filename in files: 
      if filename == fname:
        image_jpeg = os.path.join(dirpath,filename)
        image_jpeg_data = tf.io.read_file(image_jpeg)
        image_tensor = tf.io.decode_jpeg(image_jpeg_data)
        o_image, p_image, p_label = deeplab.input_preprocess.preprocess_image_and_label(image_tensor, None, 512, 512, 512, 512, is_training=False)
        target_image_jpeg = os.path.join(OUTPUT_PATH,fname);
        resized_image = Image.fromarray(tf.reshape(tf.cast(p_image, tf.uint8), [512, 512, 3]).numpy())
        resized_image.save(target_image_jpeg)
        found = True 
        break
    if found:
      break
