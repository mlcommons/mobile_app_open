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
  # Download frozen_graph.pb from
  # https://storage.cloud.google.com/mobilenet_edgetpu/checkpoints/mobilenet_edgetpu_224_1.0.tgz
  frozen_graph_filepath = '../dev-resources/mobilenet_edgetpu/frozen_graph.pb'
  input_name = 'images'
  input_shape = (1, 224, 224, 3)
  output_name = 'Softmax'
  output_shape = (1, 1001)
  saved_model_export_dir = '../dev-resources/mobilenet_edgetpu/optimized_saved_model'
  coreml_export_filepath = '../dev-resources/mobilenet_edgetpu/MobilenetEdgeTPU.mlmodel'

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
    inputs=[ct.TensorType(shape=input_shape)],
  )

  spec = model.get_spec()
  for n in output_shape:
    spec.description.output[0].type.multiArrayType.shape.append(n)

  print(spec.description)
  ct.models.MLModel(spec).save(coreml_export_filepath)


if __name__ == "__main__":
  main()
