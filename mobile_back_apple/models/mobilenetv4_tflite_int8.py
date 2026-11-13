# !/usr/bin/env python3
# coding: utf-8

# Copyright 2026 The MLPerf Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import coremltools as ct
import numpy as np
from utils import (
  quantize_activations,
  quantize_weights,
  register_keras_optimizer_fallback,
)


def main():
  """
  Download the MobileNetV4-Conv-Large-fp32 from https://github.com/mlcommons/mobile_open/releases
  Tested with tensorflow==2.15.0+ and coremltools==9.0
  """
  register_keras_optimizer_fallback()
  saved_model_dir = '../dev-resources/mobilenet_v4/MobileNetV4-Conv-Large-saved-model'
  input_shape = (1, 384, 384, 3)
  w8_export_fpath = '../dev-resources/mobilenet_v4/MobilenetV4_Large_w8.mlpackage'
  w8a8_export_fpath = '../dev-resources/mobilenet_v4/MobilenetV4_Large_w8a8.mlpackage'

  print('Converting model...')
  model = ct.convert(
    saved_model_dir,
    source='tensorflow',
    convert_to='mlprogram',
    inputs=[ct.TensorType(shape=input_shape)],
  )
  model.short_description = (
    'MobileNetV4-Conv-Large-int8 from https://github.com/mlcommons/mobile_open'
  )

  spec = model.get_spec()
  ct.utils.rename_feature(spec, 'inputs', 'images')
  ct.utils.rename_feature(spec, 'Identity', 'Softmax')
  renamed_model = ct.models.MLModel(spec, weights_dir=model.weights_dir)

  w8_model = quantize_weights(renamed_model)
  w8_model.save(w8_export_fpath)
  print('Done! W8 Core ML model exported to:', w8_export_fpath)

  np.random.seed(42)
  sample_data = [
    {'images': np.random.uniform(-1.0, 1.0, input_shape).astype(np.float32)}
    for _ in range(8)
  ]
  w8a8_model = quantize_activations(w8_model, sample_data)
  w8a8_model.save(w8a8_export_fpath)
  print('Done! W8A8 Core ML model exported to:', w8a8_export_fpath)


if __name__ == '__main__':
  main()
