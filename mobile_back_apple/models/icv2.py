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


def main():
  """
  Download the MobileNetV4-Conv-Large-fp32 from https://github.com/mlcommons/mobile_open/releases
  Tested with tensorflow==2.15.0 and coremltools==7.1
  """
  saved_model_dir = '../dev-resources/mobilenet_v4/MobileNetV4-Conv-Large-saved-model'
  export_fpath = '../dev-resources/mobilenet_v4/MobilenetV4_Large.mlmodel'
  print("Converting model...")
  model = ct.convert(
    saved_model_dir,
    source="tensorflow",
    convert_to="neuralnetwork",
    inputs=[ct.TensorType(shape=(1, 384, 384, 3))],
  )
  model.short_description = "MobileNetV4-Conv-Large-fp32 from https://github.com/mlcommons/mobile_open"

  spec = model.get_spec()
  for n in (1, 1001):
    spec.description.output[0].type.multiArrayType.shape.append(n)

  ct.utils.rename_feature(spec, "inputs", "images")
  ct.utils.rename_feature(spec, "Identity", "Softmax")
  print(spec.description)

  ct.models.MLModel(spec).save(export_fpath)
  print("Done! Core ML model exported to:", export_fpath)


if __name__ == "__main__":
  main()
