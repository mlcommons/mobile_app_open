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
import coremltools.optimize.coreml as cto


def main():
  # Download saved_model from
  # https://storage.googleapis.com/cloud-tpu-checkpoints/mobilebert/mobilebert_squad_savedmodels.tar.gz
  saved_model_source_dir = '../dev-resources/mobilebert/float_saved_model'
  input_names = ['input_ids', 'input_mask', 'segment_ids']
  output_names = ['start_logits', 'end_logits']
  coreml_export_filepath = '../dev-resources/mobilebert/MobileBERT'

  tfmodel = tf.saved_model.load(saved_model_source_dir, tags={'serve'})
  pruned = tfmodel.prune([e + ':0' for e in input_names],
                         [e + ':0' for e in output_names])
  mlmodel = ct.convert(
    [pruned],
    source='tensorflow',
  )
  op_config = cto.OpLinearQuantizerConfig()
  config = cto.OptimizationConfig(op_config)
  mlmodel = cto.linear_quantize_weights(mlmodel, config)

  mlmodel.save(coreml_export_filepath)


if __name__ == "__main__":
  main()
