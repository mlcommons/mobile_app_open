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


def print_versions():
  import tensorflow as tf
  import coremltools as ct
  print('tensorflow version:', tf.__version__)
  print('coremltools version:', ct.__version__)


def register_keras_optimizer_fallback():
  """Registers a fallback revived type for legacy Keras 2 'optimizer' nodes
  when loading SavedModels on TensorFlow 2.16+ (Keras 3) without tf_keras."""
  from tensorflow.python.saved_model import revived_types
  from tensorflow.python.trackable import autotrackable

  if 'optimizer' not in revived_types.registered_identifiers():
    class _DummyOptimizer(autotrackable.AutoTrackable):
      def add_slot(self, var, slot_name):
        return tf.Variable(tf.zeros_like(var), trainable=False)

    revived_types.register_revived_type(
      'optimizer',
      lambda proto: proto.identifier == 'optimizer',
      [
        revived_types.VersionedTypeRegistration(
          object_factory=lambda proto: _DummyOptimizer(),
          version=1,
          min_producer_version=1,
          min_consumer_version=1,
          bad_consumers=[],
        )
      ],
    )


def load_frozen_graph(path: str) -> tf.compat.v1.GraphDef():
  print(f"Loading frozen graph from {path}")
  with tf.io.gfile.GFile(path, "rb") as f:
    graph_def = tf.compat.v1.GraphDef()
    graph_def.ParseFromString(f.read())
    return graph_def


def optimize_graph(graph_def: tf.compat.v1.GraphDef(),
                   input_node_name: str,
                   output_node_name: str,
                   export_dir: str,
                   dtype: str = 'float32'):
  from tensorflow.python.tools import strip_unused_lib
  from tensorflow.python.saved_model import signature_constants, tag_constants
  from tensorflow.python.framework import dtypes

  type_enum_dict = {
    'float32': dtypes.float32.as_datatype_enum,
    'float16': dtypes.float16.as_datatype_enum,
    'uint8': dtypes.uint8.as_datatype_enum,
  }
  gdef = strip_unused_lib.strip_unused(
    input_graph_def=graph_def,
    input_node_names=[input_node_name],
    output_node_names=[output_node_name],
    placeholder_type_enum=type_enum_dict[dtype])

  with tf.compat.v1.Session(graph=tf.Graph()) as sess:
    tf.import_graph_def(gdef, name="")
    g = tf.compat.v1.get_default_graph()
    input_name = g.get_tensor_by_name(input_node_name + ':0')
    output_name = g.get_tensor_by_name(output_node_name + ':0')

    signature_def = tf.compat.v1.saved_model.signature_def_utils.predict_signature_def
    key = signature_constants.DEFAULT_SERVING_SIGNATURE_DEF_KEY
    sigs = {key: signature_def({"input": input_name}, {"output": output_name})}

    builder = tf.compat.v1.saved_model.builder.SavedModelBuilder(export_dir)
    builder.add_meta_graph_and_variables(sess,
                                         [tag_constants.SERVING],
                                         signature_def_map=sigs)
    builder.save()
    print('Optimized graph saved at:', export_dir)


def quantize_weights(mlmodel, dtype: str = 'int8', mode: str = 'linear_symmetric'):
  """Quantizes ML Program weights to 8-bit integers (W8)."""
  import coremltools.optimize.coreml as cto

  op_config = cto.OpLinearQuantizerConfig(mode=mode, dtype=dtype)
  config = cto.OptimizationConfig(global_config=op_config)
  quantized_model = cto.linear_quantize_weights(mlmodel, config)
  print('Weights quantization (W8) finished.')
  return quantized_model


def quantize_activations(mlmodel,
                         sample_data: list,
                         dtype: str = 'int8',
                         mode: str = 'linear_symmetric'):
  """Quantizes ML Program activations to 8-bit integers (A8, requires iOS 17+)."""
  import coremltools.optimize.coreml as cto
  try:
    from coremltools.optimize.coreml import (
      OpActivationLinearQuantizerConfig,
      linear_quantize_activations,
    )
  except ImportError:
    from coremltools.optimize.coreml.experimental import (
      OpActivationLinearQuantizerConfig,
      linear_quantize_activations,
    )

  op_config = OpActivationLinearQuantizerConfig(mode=mode, dtype=dtype)
  config = cto.OptimizationConfig(global_config=op_config)
  quantized_model = linear_quantize_activations(
    mlmodel, config, sample_data=sample_data
  )
  print('Activations quantization (A8) finished.')
  return quantized_model


def _patch_reduce_argmax_uint16(prog):
  """Replaces trailing reduce_argmax with uint16 output_dtype for iOS 18+ ANE."""
  import coremltools as ct
  from coremltools.converters.mil import Builder as mb

  fn = prog.functions['main']
  fn.opset_version = ct.target.iOS18
  out_var = fn.outputs[0]
  last_op = out_var.op
  if last_op is not None and last_op.op_type == 'reduce_argmax':
    with fn:
      new_out = mb.reduce_argmax(
        x=last_op.x,
        axis=last_op.axis,
        keep_dims=last_op.keep_dims,
        output_dtype='uint16',
        name=out_var.name,
      )
      fn.set_outputs([new_out])
    fn.remove_ops([last_op])


def convert_mosaic_ios18(pruned_fn, input_shape: tuple):
  """Converts pruned MOSAIC TF graph to iOS 18 ML Program with uint16 argmax for ANE."""
  import coremltools as ct
  import numpy as np

  prog = ct.convert(
    [pruned_fn],
    source='tensorflow',
    convert_to='milinternal',
    inputs=[ct.TensorType(shape=input_shape, dtype=np.float32)],
    minimum_deployment_target=ct.target.iOS18,
  )
  _patch_reduce_argmax_uint16(prog)

  return ct.convert(
    prog,
    convert_to='mlprogram',
    minimum_deployment_target=ct.target.iOS18,
  )


def upgrade_mosaic_ios18(mlmodel):
  """Upgrades a quantized MOSAIC MLModel to iOS 18 with uint16 argmax for ANE."""
  import coremltools as ct
  from coremltools.converters.mil.frontend.milproto.load import load as milproto_load

  spec = mlmodel.get_spec()
  prog = milproto_load(
    spec, spec.specificationVersion, file_weights_dir=mlmodel.weights_dir
  )
  _patch_reduce_argmax_uint16(prog)

  return ct.convert(
    prog,
    convert_to='mlprogram',
    minimum_deployment_target=ct.target.iOS18,
  )


