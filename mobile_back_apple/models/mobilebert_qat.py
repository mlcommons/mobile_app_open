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

import argparse

import coremltools as ct
from coremltools.converters.mil import Builder as mb
from coremltools.converters.mil.frontend.tensorflow.tf_op_registry import _TF_OPS_REGISTRY
import numpy as np
import tensorflow as tf
from utils import quantize_weights

_DEFAULT_EXPORTS = {
  'sym_all': '../dev-resources/mobilebert/mobilebert_qat_w8a8_sym_all.mlpackage',
  'sym_no_attn_ln': '../dev-resources/mobilebert/mobilebert_qat_w8a8_sym_no_attn_ln.mlpackage',
  'w8': '../dev-resources/mobilebert/mobilebert_qat_w8.mlpackage',
}


def _make_fake_quant_handler(mode: str):
  def handler(context, node):
    # Weight fake-quant nodes are folded into constants by coremltools before
    # reaching this handler, preserving the QAT-trained int8 weight grid before
    # quantize_weights() compresses weights to int8.
    x = context[node.inputs[0]]
    name = node.name

    if mode == 'w8':
      context.add(name, mb.identity(x=x, name=name))
      return

    if mode == 'sym_no_attn_ln' and (
      'attention/self/MobileBertActFakeQuantWithMinMaxVars' in name
      or 'FakeLayerNorm' in name
    ):
      context.add(name, mb.identity(x=x, name=name))
      return

    mn = float(context[node.inputs[1]].val)
    mx = float(context[node.inputs[2]].val)
    scale = np.float32(max(max(abs(mn), abs(mx)) / 127.0, 1e-7))
    q = mb.quantize(input=x, scale=scale, output_dtype='int8')
    dq = mb.dequantize(input=q, scale=scale, name=name)
    context.add(name, dq)

  return handler


def main():
  # Download checkpoints/quant from
  # https://github.com/mlcommons/mobile_open/tree/main/language/bert/models_and_code/checkpoints/quant
  parser = argparse.ArgumentParser(
    description='Convert TF QAT MobileBERT SavedModel to Core ML INT8 W8A8 or W8 .mlpackage.'
  )
  parser.add_argument(
    'saved_model_source_dir',
    nargs='?',
    default='../dev-resources/mobilebert/quant_saved_model',
    help='Path to TF QAT SavedModel directory (checkpoints/quant)',
  )
  parser.add_argument(
    'export_filepath',
    nargs='?',
    default=None,
    help='Path to output .mlpackage directory',
  )
  parser.add_argument(
    '--mode',
    choices=['sym_all', 'sym_no_attn_ln', 'w8'],
    default='sym_all',
    help='Activation quantization mode: sym_all (default W8A8), sym_no_attn_ln (higher-F1 W8A8), or w8 (FP16 activations).',
  )
  args = parser.parse_args()

  export_filepath = args.export_filepath or _DEFAULT_EXPORTS[args.mode]
  input_names = ['input_ids', 'input_mask', 'segment_ids']
  output_names = ['start_logits', 'end_logits']

  _TF_OPS_REGISTRY['FakeQuantWithMinMaxVars'] = _make_fake_quant_handler(args.mode)

  tfmodel = tf.saved_model.load(args.saved_model_source_dir, tags={'serve'})
  pruned = tfmodel.prune(
    [e + ':0' for e in input_names],
    [e + ':0' for e in output_names],
  )
  convert_kwargs = {
    'source': 'tensorflow',
    'convert_to': 'mlprogram',
    'inputs': [
      ct.TensorType(name=name, shape=(1, 384), dtype=np.int32)
      for name in input_names
    ],
    'outputs': [ct.TensorType(name=name, dtype=np.float32) for name in output_names],
  }
  if args.mode != 'w8':
    convert_kwargs['minimum_deployment_target'] = ct.target.iOS17

  model = ct.convert([pruned], **convert_kwargs)
  model = quantize_weights(model)
  model.save(export_filepath)
  print('Done! Core ML model exported to:', export_filepath)


if __name__ == '__main__':
  main()
