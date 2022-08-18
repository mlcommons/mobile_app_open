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

import tensorflow as tf
import coremltools as ct
from coremltools.models.neural_network import quantization_utils
import numpy as np

def main():
  # Download saved_model from
  # https://storage.googleapis.com/cloud-tpu-checkpoints/mobilebert/mobilebert_squad_savedmodels.tar.gz
  saved_model_source_dir = '../dev-resources/mobilebert/float_saved_model'
  input_names = ['input_ids', 'input_mask', 'segment_ids']
  output_names = ['start_logits', 'end_logits']
  output_shapes = [(1, 384), (1, 384)]
  coreml_export_filepath = '../dev-resources/mobilebert/MobileBERT.mlpackage'

  tfmodel = tf.saved_model.load(saved_model_source_dir, tags={'serve'})
  pruned = tfmodel.prune([e + ':0' for e in input_names],
                         [e + ':0' for e in output_names])
  model = ct.convert(
    [pruned],
    source='tensorflow',
    convert_to="mlprogram",
  )

  spec = model.get_spec()
  for i, shape in enumerate(output_shapes):
    for n in shape:
      spec.description.output[i].type.multiArrayType.shape.append(n)

  dtype_int32 = ct.proto.FeatureTypes_pb2.ArrayFeatureType.INT32
  for i, _ in enumerate(input_names):
    spec.description.input[i].type.multiArrayType.dataType = dtype_int32

  print(spec.description)

  ct.models.MLModel(spec, weights_dir=model.weights_dir).save(coreml_export_filepath)


if __name__ == "__main__":
  main()
