# !/usr/bin/env python3
# coding: utf-8

# Copyright 2022 The MLPerf Authors. All Rights Reserved.
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
from utils import load_frozen_graph, optimize_graph


def main():
  # Download edge_frozen_graph.pb as frozen_graph.pb from
  # https://github.com/mlcommons/mobile_open/blob/main/vision/mobilenet/models_and_code/checkpoints/float/edge_frozen_graph.pb
  frozen_graph_filepath = '../dev-resources/mobilenet_edgetpu/frozen_graph.pb'
  input_name = 'images'
  input_shape = (1, 224, 224, 3)
  output_name = 'Softmax'
  saved_model_export_dir = '../dev-resources/mobilenet_edgetpu/optimized_saved_model'
  coreml_export_filepath = '../dev-resources/mobilenet_edgetpu/MobilenetEdgeTPU.mlpackage'

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

  print(model.get_spec().description)
  model.save(coreml_export_filepath)
  print('Done! Core ML model exported to:', coreml_export_filepath)


if __name__ == "__main__":
  main()
