#!/usr/bin/env python3
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

import sys

import coremltools as ct
import numpy as np
import tensorflow as tf
from coremltools.converters.mil import Builder as mb
from coremltools.converters.mil.frontend.tensorflow.tf_op_registry import _TF_OPS_REGISTRY
from utils import quantize_weights


def _fake_quant_activation_passthrough(context, node):
  # The QAT checkpoint carries 8-bit activation fake-quant nodes. Keeping them
  # makes coremltools emit a mul/round/clip chain per node (6k+ ops, ~40% slower
  # and wrong in fp16). Drop them; weight fake-quant is folded into constants by
  # coremltools before it reaches this handler, so the QAT-trained int8 weight
  # grid is preserved and then compressed to int8 by quantize_weights().
  x = context[node.inputs[0]]
  context.add(node.name, mb.identity(x=x, name=node.name))


def main():
  # Download from
  # https://github.com/mlcommons/mobile_open/tree/main/language/bert/models_and_code/checkpoints/quant
  saved_model_source_dir = sys.argv[1] if len(sys.argv) > 1 else '../dev-resources/mobilebert/quant_saved_model'
  export_filepath = sys.argv[2] if len(sys.argv) > 2 else '../dev-resources/mobilebert/MobileBERT_qat_w8.mlpackage'
  input_names = ['input_ids', 'input_mask', 'segment_ids']
  output_names = ['start_logits', 'end_logits']

  _TF_OPS_REGISTRY['FakeQuantWithMinMaxVars'] = _fake_quant_activation_passthrough

  tfmodel = tf.saved_model.load(saved_model_source_dir, tags={'serve'})
  pruned = tfmodel.prune([e + ':0' for e in input_names],
                         [e + ':0' for e in output_names])
  model = ct.convert(
    [pruned],
    source='tensorflow',
    convert_to='mlprogram',
    inputs=[ct.TensorType(name=name, shape=(1, 384), dtype=np.int32) for name in input_names],
    outputs=[ct.TensorType(name=name, dtype=np.float32) for name in output_names],
  )
  model = quantize_weights(model)
  model.save(export_filepath)
  print('Done! Core ML model exported to:', export_filepath)


if __name__ == '__main__':
  main()
