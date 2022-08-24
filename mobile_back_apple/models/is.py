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
import tensorflow as tf
import coremltools as ct
from utils import load_frozen_graph, optimize_graph, rename_feature


def main():
  # Download mobile_segmenter_r4_frozen_graph.pb from
  # https://github.com/mlcommons/mobile_open/tree/main/vision/mosaic/models_and_checkpoints/R4
  frozen_graph_filepath = '../dev-resources/mosaic/mobile_segmenter_r4_frozen_graph.pb'
  input_name = 'sub_2'
  input_shape = (1, 512, 512, 3)
  output_name = 'ArgMax'
  output_shape = (1, 512, 512)
  saved_model_export_dir = '../dev-resources/mosaic/optimized_saved_model'
  coreml_export_filepath = '../dev-resources/mosaic/Mosaic.mlpackage'

  graph_def = load_frozen_graph(frozen_graph_filepath)
  shutil.rmtree(saved_model_export_dir, ignore_errors=True)
  optimize_graph(graph_def,
                 input_name,
                 output_name,
                 saved_model_export_dir,
                 dtype='float32')

  # Based on https://github.com/freedomtan/coreml_models_for_mlperf/tree/main/mosaic
  tfmodel = tf.saved_model.load(saved_model_export_dir)
  pruned = tfmodel.prune(f"{input_name}:0", f"{output_name}:0")

  model = ct.convert(
    [pruned],
    source='tensorflow',
    convert_to="mlprogram",
    inputs=[ct.TensorType(shape=input_shape)],
  )

  spec = model.get_spec()
  for n in output_shape:
    spec.description.output[0].type.multiArrayType.shape.append(n)

  # CoreMLExecutor sorts input and output names alphabetically then access it by index,
  # so we prefix the name with number to make sure they are sorted as we want.
  rename_feature(spec, input_name, 'i1_normalized_image')
  rename_feature(spec, output_name, 'o1_semantic_prediction')

  print(spec.description)
  ct.models.MLModel(spec, weights_dir=model.weights_dir).save(coreml_export_filepath)


if __name__ == "__main__":
  main()
