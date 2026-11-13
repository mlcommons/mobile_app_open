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
  Download the EDSR f32b5 SavedModel from
  https://github.com/mlcommons/mobile_open/tree/main/vision/edsr/models_and_checkpoints/checkpoints/f32b5/ckpt
  """
  register_keras_optimizer_fallback()

  saved_model_dir = '../dev-resources/edsr/f32b5/ckpt'
  input_shape = (1, 540, 960, 3)
  w8_export_filepath = '../dev-resources/edsr/edsr_f32b5_w8.mlpackage'
  w8a8_export_filepath = '../dev-resources/edsr/edsr_f32b5_w8a8.mlpackage'

  print('Converting model...')
  model = ct.convert(
    saved_model_dir,
    source='tensorflow',
    convert_to='mlprogram',
    inputs=[ct.TensorType(shape=input_shape)],
  )
  model.short_description = (
    'EDSR f32b5 int8 from https://github.com/mlcommons/mobile_open'
  )

  w8_model = quantize_weights(model)
  w8_model.save(w8_export_filepath)
  print('Done! W8 Core ML model exported to:', w8_export_filepath)

  input_name = model.get_spec().description.input[0].name
  np.random.seed(42)
  sample_data = [
    {input_name: np.random.uniform(0.0, 255.0, input_shape).astype(np.float32)}
    for _ in range(4)
  ]
  w8a8_model = quantize_activations(w8_model, sample_data)
  w8a8_model.save(w8a8_export_filepath)
  print('Done! W8A8 Core ML model exported to:', w8a8_export_filepath)


if __name__ == '__main__':
  main()
