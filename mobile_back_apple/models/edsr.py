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
from utils import register_keras_optimizer_fallback


def main():
  """
  Download the EDSR f32b5 SavedModel from
  https://github.com/mlcommons/mobile_open/tree/main/vision/edsr/models_and_checkpoints/checkpoints/f32b5/ckpt
  """
  register_keras_optimizer_fallback()

  saved_model_dir = '../dev-resources/edsr/f32b5/ckpt'
  input_shape = (1, 540, 960, 3)
  coreml_export_filepath = '../dev-resources/edsr/edsr_f32b5_fp32.mlpackage'

  print('Converting model...')
  model = ct.convert(
    saved_model_dir,
    source='tensorflow',
    convert_to='mlprogram',
    inputs=[ct.TensorType(shape=input_shape)],
  )
  model.short_description = (
    'EDSR f32b5 fp32 from https://github.com/mlcommons/mobile_open'
  )

  print(model.get_spec().description)
  model.save(coreml_export_filepath)
  print('Done! Core ML model exported to:', coreml_export_filepath)


if __name__ == '__main__':
  main()
