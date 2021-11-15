# Copyright (c) 2020-2021 The MLPerf Authors. All rights reserved.
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
import tensorflow as tf
import deeplab.input_preprocess
from PIL import Image as Image

tf.enable_eager_execution()

ADE20K_PATH = sys.argv[1]
OUTPUT_PATH = sys.argv[2]


for i in range(1, 2001):
    image_jpeg = ADE20K_PATH + '/images/validation/ADE_val_0000%04d.jpg' % i
    label_png = ADE20K_PATH + '/annotations/validation/ADE_val_0000%04d.png' % i
    print(image_jpeg)
    image_jpeg_data = tf.io.read_file(image_jpeg)
    image_tensor = tf.io.decode_jpeg(image_jpeg_data)
    label_png_data = tf.io.read_file(label_png)
    label_tensor = tf.io.decode_jpeg(label_png_data)
    o_image, p_image, p_label = deeplab.input_preprocess.preprocess_image_and_label(image_tensor, label_tensor, 512, 512, 512, 512, is_training=False)

    target_image_jpeg = OUTPUT_PATH + '/images/ADE_val_0000%04d.jpg' % i
    target_label_raw = OUTPUT_PATH + '/annotations/ADE_val_0000%04d.raw' % i

    resized_image = Image.fromarray(tf.reshape(tf.cast(p_image, tf.uint8), [512, 512, 3]).numpy())
    resized_image.save(target_image_jpeg)
    tf.reshape(tf.cast(p_label, tf.uint8), [512, 512]).numpy().tofile(target_label_raw)
