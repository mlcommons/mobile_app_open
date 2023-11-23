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
import tensorflow as tf
import tensorflow_hub as hub


def main():
  print("Retrieve model from TFHub...")
  keras_layer = hub.KerasLayer('https://tfhub.dev/google/edgetpu/vision/mobilenet-edgetpu-v2/l/1')
  keras_model = tf.keras.Sequential([keras_layer])
  keras_model.build([None, 224, 224, 3])
  input_tensor = tf.ones((4, 224, 224, 3))
  output_tensor = keras_model(input_tensor)
  print("output_tensor.shape:", output_tensor.shape)

  model = ct.convert(
    keras_model,
    convert_to="neuralnetwork",
    inputs=[ct.TensorType(shape=(1, 224, 224, 3))],
  )
  model.short_description = "MobilenetEdgeTPUv2 from https://tfhub.dev/google/edgetpu/vision/mobilenet-edgetpu-v2/l/1"

  spec = model.get_spec()
  for n in (1, 1001):
    spec.description.output[0].type.multiArrayType.shape.append(n)

  ct.utils.rename_feature(spec, "keras_layer_input", "images")
  ct.utils.rename_feature(spec, "Identity", "Softmax")
  print(spec.description)

  export_fpath = '../dev-resources/mobilenet_edgetpu/MobilenetEdgeTPUv2.mlmodel'
  ct.models.MLModel(spec).save(export_fpath)


if __name__ == "__main__":
  main()
