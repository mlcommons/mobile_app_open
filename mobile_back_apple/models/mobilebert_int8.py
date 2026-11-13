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
import tensorflow as tf
from utils import quantize_activations, quantize_weights


def main():
  # Download saved_model from
  # https://github.com/mlcommons/mobile_open/tree/main/language/bert/models_and_code/checkpoints/float
  saved_model_source_dir = '../dev-resources/mobilebert/float_saved_model'
  input_names = ['input_ids', 'input_mask', 'segment_ids']
  output_names = ['start_logits', 'end_logits']
  w8_export_filepath = '../dev-resources/mobilebert/MobileBERT_w8.mlpackage'
  w8a8_export_filepath = '../dev-resources/mobilebert/MobileBERT_w8a8.mlpackage'

  tfmodel = tf.saved_model.load(saved_model_source_dir, tags={'serve'})
  pruned = tfmodel.prune([e + ':0' for e in input_names],
                         [e + ':0' for e in output_names])
  model = ct.convert(
    [pruned],
    source='tensorflow',
    convert_to='mlprogram',
    inputs=[
      ct.TensorType(name=name, shape=(1, 384), dtype=np.int32)
      for name in input_names
    ],
    outputs=[
      ct.TensorType(name=name, dtype=np.float32) for name in output_names
    ],
  )

  w8_model = quantize_weights(model)
  w8_model.save(w8_export_filepath)
  print('Done! W8 Core ML model exported to:', w8_export_filepath)

  np.random.seed(42)
  sample_data = [
    {
      'input_ids': np.random.randint(1, 1000, (1, 384), dtype=np.int32),
      'input_mask': np.ones((1, 384), dtype=np.int32),
      'segment_ids': np.zeros((1, 384), dtype=np.int32),
    }
    for _ in range(4)
  ]
  w8a8_model = quantize_activations(w8_model, sample_data)
  w8a8_model.save(w8a8_export_filepath)
  print('Done! W8A8 Core ML model exported to:', w8a8_export_filepath)


if __name__ == '__main__':
  main()
