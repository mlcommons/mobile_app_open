# !/usr/bin/env python3
# coding: utf-8

# Copyright 2023 The MLPerf Authors. All Rights Reserved.
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
from coremltools.converters.mil import Builder as mb
import tensorflow as tf
from utils import register_keras_optimizer_fallback


def convert_mobilenetv4_nchw(saved_model_dir: str) -> ct.models.MLModel:
  """Convert MobileNetV4 SavedModel to a transpose-free NCHW Core ML model."""
  register_keras_optimizer_fallback()
  tf_model = tf.saved_model.load(saved_model_dir)
  infer = tf_model.signatures["serving_default"]

  @tf.function(
    input_signature=[
      tf.TensorSpec(shape=[1, 3, 384, 384], dtype=tf.float32, name="images")
    ]
  )
  def nchw_fn(images):
    nhwc = tf.transpose(images, [0, 2, 3, 1])
    out = infer(inputs=nhwc)
    return list(out.values())[0]

  prog = ct.convert(
    [nchw_fn.get_concrete_function()],
    source="tensorflow",
    convert_to="milinternal",
    inputs=[ct.TensorType(name="images", shape=(1, 3, 384, 384))],
  )

  fn = prog.functions["main"]
  with fn:
    for op in list(fn.operations):
      if (
        op.op_type == "transpose"
        and tuple(op.inputs["x"].shape) == (1, 1280, 1, 1)
        and tuple(op.outputs[0].shape) == (1, 1, 1, 1280)
      ):
        new_var = mb.reshape(
          x=op.inputs["x"], shape=(1, 1, 1, 1280), before_op=op
        )
        fn.replace_uses_of_var_after_op(
          anchor_op=op, old_var=op.outputs[0], new_var=new_var
        )
        fn.remove_ops([op])

  model = ct.convert(prog, convert_to="mlprogram")
  spec = model.get_spec()
  out_name = spec.description.output[0].name
  if out_name != "Softmax":
    ct.utils.rename_feature(spec, out_name, "Softmax")
  return ct.models.MLModel(spec, weights_dir=model.weights_dir)


def main():
  """
  Download the MobileNetV4-Conv-Large-fp32 from https://github.com/mlcommons/mobile_open/releases
  Tested with tensorflow==2.15.0+ and coremltools==9.0
  """
  saved_model_dir = "../dev-resources/mobilenet_v4/MobileNetV4-Conv-Large-saved-model"
  export_fpath = "../dev-resources/mobilenet_v4/MobilenetV4_Large_NCHW.mlpackage"
  print("Converting model...")
  model = convert_mobilenetv4_nchw(saved_model_dir)
  model.short_description = "MobileNetV4-Conv-Large-fp32 (NCHW) from https://github.com/mlcommons/mobile_open"
  print(model.get_spec().description)
  model.save(export_fpath)
  print("Done! Core ML model exported to:", export_fpath)


if __name__ == "__main__":
  main()
