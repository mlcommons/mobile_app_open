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

# Based on
# https://github.com/freedomtan/coreml_models_for_mlperf/tree/main/mobiledet
# https://github.com/hollance/coreml-survival-guide/tree/master/MobileNetV2%2BSSDLite


import tensorflow as tf
import coremltools as ct
from coremltools.converters.mil import Builder as mb
import numpy as np

input_height = 320
input_width = 320

input_node = 'Preprocessor/map/TensorArrayStack/TensorArrayGatherV3'
bbox_output_node = 'Squeeze'
class_output_node = 'Postprocessor/convert_scores'

coreml_export_filepath = '../dev-resources/mobiledet/MobileDet.mlpackage'

# Download saved model from
# https://github.com/mlcommons/mobile_open/tree/main/vision/mobiledet/fp32/export_inference_graph/saved_model
saved_model_dir = '../dev-resources/mobiledet/saved_model'
original_model = tf.saved_model.load(saved_model_dir)
pruned_model = original_model.prune(
    input_node + ':0', [bbox_output_node + ':0', class_output_node + ':0'])


def get_anchors(start_tensor, end_tensor):
  """
  Computes the list of anchor boxes by sending a fake image through the graph.
  Outputs an array of size (4, num_anchors) where each element is an anchor box
  given as [ycenter, xcenter, height, width] in normalized coordinates.
  """
  anchors_model = original_model.prune(start_tensor, [end_tensor])
  box_corners = tf.squeeze(
      anchors_model(tf.zeros((1, input_height, input_width, 3), tf.uint8)))

  # The TensorFlow graph gives each anchor box as [ymin, xmin, ymax, xmax].
  # Convert these min/max values to a center coordinate, width and height.
  ymin, xmin, ymax, xmax = np.transpose(box_corners)
  width = xmax - xmin
  height = ymax - ymin
  ycenter = ymin + height / 2.
  xcenter = xmin + width / 2.
  return np.stack([ycenter, xcenter, height, width]).astype(np.float32)


num_classes = 90
num_anchors = 2034

# Read the anchors into a (4, 2034) tensor.
start_tensor = 'image_tensor:0'
anchors_tensor = 'Concatenate/concat:0'
anchors = get_anchors(start_tensor, anchors_tensor)
assert anchors.shape == (4, num_anchors)

tf_input_types = [
    ct.TensorType(
        name=input_node,
        shape=(1, input_height, input_width, 3),
        dtype=np.float32,
    )
]
prog = ct.convert(
    [pruned_model],
    source='tensorflow',
    convert_to='milinternal',
    inputs=tf_input_types,
    compute_precision=ct.precision.FLOAT32,
    minimum_deployment_target=ct.target.iOS17,
)

fn = prog.functions['main']
if fn.outputs[0].shape[-1] == 4:
  boxes_var, scores_var = fn.outputs
else:
  scores_var, boxes_var = fn.outputs
with fn:
  anchors_yx = anchors[:2, :].T.reshape(1, num_anchors, 2)
  anchors_hw = anchors[2:, :].T.reshape(1, num_anchors, 2)

  yx_raw, hw_raw = mb.split(x=boxes_var, num_splits=2, axis=2)
  yx = mb.add(
      x=mb.mul(x=mb.mul(x=yx_raw, y=np.float32(0.1)), y=anchors_hw),
      y=anchors_yx,
  )
  hw = mb.mul(
      x=mb.exp(x=mb.mul(x=hw_raw, y=np.float32(0.2))),
      y=anchors_hw,
  )

  # NonMaximumSuppression (iOS 17+) expects boxes [n, 4, B] and scores [n, K, B].
  y_c, x_c = mb.split(x=yx, num_splits=2, axis=2)
  h_c, w_c = mb.split(x=hw, num_splits=2, axis=2)
  raw_coordinates = mb.concat(values=[x_c, y_c, w_c, h_c], axis=2)
  boxes_t = mb.transpose(x=raw_coordinates, perm=[0, 2, 1])

  scores_t = mb.transpose(x=scores_var, perm=[0, 2, 1])
  score_mask = mb.greater_equal(x=scores_t, y=np.float32(0.3))
  masked_scores = mb.select(
      cond=score_mask, a=scores_t, b=np.float32(0.0)
  )

  coords_t, conf_t, _ = mb.non_maximum_suppression(
      boxes=boxes_t,
      scores=masked_scores,
      iou_threshold=np.float32(0.3),
      max_boxes=10,
      per_class_suppression=True,
  )
  coords = mb.transpose(x=coords_t, perm=[0, 2, 1])
  # conf_t is [n, K, B] (NMS's native class-major layout).
  # reduce_argmax/reduce_max over axis=1 (K) give the same [n, B]
  # result as transposing to [n, B, K] and reducing over axis=2 would,
  # without the extra transpose op.

  # Convert (x_center, y_center, width, height) to (y1, x1, y2, x2).
  t = np.array(
      [
          [0, 1, 0, 1],
          [1, 0, 1, 0],
          [0, -0.5, 0, 0.5],
          [-0.5, 0, 0.5, 0],
      ],
      dtype=np.float32,
  )
  det_boxes = mb.matmul(x=coords, y=t, name='detection_boxes')
  cls_idx = mb.reduce_argmax(x=conf_t, axis=1, keep_dims=False)
  det_classes = mb.sub(
      x=mb.cast(x=cls_idx, dtype='fp32'),
      y=np.float32(1.0),
      name='detection_classes',
  )
  det_scores = mb.reduce_max(
      x=conf_t, axes=[1], keep_dims=False, name='detection_scores'
  )
  det_num = mb.reduce_sum(
      x=mb.sign(x=det_scores), axes=[1], keep_dims=False,
      name='number_of_boxes',
  )
  fn.set_outputs([det_boxes, det_classes, det_scores, det_num])

mil_input_types = [
    ct.TensorType(
        name=input_node.replace('/', '_'),
        shape=(1, input_height, input_width, 3),
        dtype=np.float32,
    )
]
output_types = [
    ct.TensorType(name='detection_boxes', dtype=np.float32),
    ct.TensorType(name='detection_classes', dtype=np.float32),
    ct.TensorType(name='detection_scores', dtype=np.float32),
    ct.TensorType(name='number_of_boxes', dtype=np.float32),
]
fn.set_output_types(output_types)

ssd_model = ct.convert(
    prog,
    convert_to='mlprogram',
    minimum_deployment_target=ct.target.iOS17,
    inputs=mil_input_types,
    outputs=output_types,
)

spec = ssd_model.get_spec()
ct.utils.rename_feature(spec, input_node.replace('/', '_'), 'image')

spec.description.metadata.versionString = (
    'ssdlite_mobiledet_edgetpu_320x320_coco_2020_05_19'
)
spec.description.metadata.shortDescription = 'MobileDet, trained on COCO'
spec.description.metadata.author = (
    'Converted to Core ML by Koan-Sin Tan. Original Authors: Yunyang Xiong, '
    'Hanxiao Liu, Suyog Gupta, Berkin Akin, Gabriel Bender, Yongzhe Wang, '
    'Pieter-Jan Kindermans, Mingxing Tan, Vikas Singh, Bo Chen'
)
spec.description.metadata.license = (
    'https://github.com/tensorflow/models/blob/master/research/object_detection'
)

final_model = ct.models.MLModel(spec, weights_dir=ssd_model.weights_dir)
final_model.save(coreml_export_filepath)

print(spec.description)
print('Done! Exported to', coreml_export_filepath)

