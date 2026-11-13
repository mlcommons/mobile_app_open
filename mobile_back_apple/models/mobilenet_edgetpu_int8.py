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

import shutil
import coremltools as ct
import numpy as np
from utils import (
  load_frozen_graph,
  optimize_graph,
  quantize_activations,
  quantize_weights,
)


def main():
  # Download edge_frozen_graph.pb as frozen_graph.pb from
  # https://github.com/mlcommons/mobile_open/blob/main/vision/mobilenet/models_and_code/checkpoints/float/edge_frozen_graph.pb
  frozen_graph_filepath = '../dev-resources/mobilenet_edgetpu/frozen_graph.pb'
  input_name = 'images'
  input_shape = (1, 224, 224, 3)
  output_name = 'Softmax'
  saved_model_export_dir = '../dev-resources/mobilenet_edgetpu/optimized_saved_model'
  w8_export_filepath = '../dev-resources/mobilenet_edgetpu/MobilenetEdgeTPU_w8.mlpackage'
  w8a8_export_filepath = '../dev-resources/mobilenet_edgetpu/MobilenetEdgeTPU_w8a8.mlpackage'

  graph_def = load_frozen_graph(frozen_graph_filepath)
  shutil.rmtree(saved_model_export_dir, ignore_errors=True)
  optimize_graph(graph_def,
                 input_name,
                 output_name,
                 saved_model_export_dir,
                 dtype='float32')

  model = ct.convert(
    saved_model_export_dir,
    source='tensorflow',
    convert_to='mlprogram',
    inputs=[ct.TensorType(shape=input_shape)],
  )

  w8_model = quantize_weights(model)
  w8_model.save(w8_export_filepath)
  print('Done! W8 Core ML model exported to:', w8_export_filepath)

  np.random.seed(42)
  sample_data = [
    {input_name: np.random.uniform(-1.0, 1.0, input_shape).astype(np.float32)}
    for _ in range(8)
  ]
  w8a8_model = quantize_activations(w8_model, sample_data)
  w8a8_model.save(w8a8_export_filepath)
  print('Done! W8A8 Core ML model exported to:', w8a8_export_filepath)


if __name__ == '__main__':
  main()
