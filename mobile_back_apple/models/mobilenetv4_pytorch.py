# !/usr/bin/env python3
# coding: utf-8

# Copyright 2024 The MLPerf Authors. All Rights Reserved.
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

# Tested with torch==2.3.1, coremltools==8.0b1, timm==1.0.7, macOS 14.5, XCode 16.0 beta

import os
import timm
import torch
import numpy as np
import coremltools as ct
import coremltools.optimize.coreml as cto
from torchvision.transforms import v2

# The following API is for coremltools==8.0b1
# It will be moved out of "experimental" in later versions of coremltools
from coremltools.optimize.coreml.experimental import OpActivationLinearQuantizerConfig, \
  linear_quantize_activations
from PIL import Image

C = 3
H = 384
W = 384

INPUT_NAME = 'images'
OUTPUT_NAME = 'softmax'
MODEL_NAME = 'hf-hub:timm/mobilenetv4_conv_large.e600_r384_in1k'

MLMODEL_FILE_FP32 = 'mobilenetv4_fp32.mlpackage'
MLMODEL_FILE_W8 = "mobilenetv4_w8.mlpackage"
MLMODEL_FILE_W8A8 = "mobilenetv4_w8a8.mlpackage"

IMAGE_DIR = './imagenet'
LABELS_FILE = 'imagenet_val_full.txt'


def load_labels(labels_file: str) -> list[str]:
  with open(labels_file, 'r') as f:
    lines = f.readlines()
    return lines


def load_dummy_images(count: int = 9) -> list[Image]:
  images = []
  for _ in range(count):
    dummy_image = np.random.randint(0, 256, (H, W, C), dtype=np.uint8)
    images.append(Image.fromarray(dummy_image))
  return images


def load_images_from_folder(folder: str, max_images: int = None) -> list[Image]:
  images = []
  filenames = os.listdir(folder)
  filenames.sort()
  if max_images is not None and len(filenames) > max_images:
    filenames = filenames[:max_images]
  for filename in filenames:
    if filename.lower().endswith((".jpg", ".jpeg", ".png")):
      img_path = os.path.join(folder, filename)
      img = Image.open(img_path).convert('RGB')
      images.append(img)
      print(f'Loaded: {filename}')
  print(f'Loaded {len(images)} images from {folder}')
  return images


def preprocess_images(pil_images: list[Image]) -> list[dict]:
  # mean and std for ImageNet
  mean = [0.485, 0.456, 0.406]
  std = [0.229, 0.224, 0.225]
  transform = v2.Compose([
    v2.ToImage(),
    v2.ToDtype(torch.uint8, scale=True),
    v2.CenterCrop(size=(H, W)),
    v2.ToDtype(torch.float32, scale=True),
    v2.Normalize(mean, std)
  ])
  transformed_images = transform(pil_images)
  data = []
  for image in transformed_images:
    img_np = image.numpy()
    img_np = img_np.reshape(1, C, H, W)
    assert (img_np.shape == (1, C, H, W))
    data.append({INPUT_NAME: img_np})
  return data


def quantize_weights(mlmodel: ct.models.MLModel) -> ct.models.MLModel:
  # quantize weights to 8 bits
  weight_quant_op_config = cto.OpLinearQuantizerConfig(mode="linear_symmetric",
                                                       dtype="int8")
  weight_quant_model_config = cto.OptimizationConfig(weight_quant_op_config)
  mlmodel_quantized = cto.linear_quantize_weights(mlmodel,
                                                  weight_quant_model_config)
  print('Weights quantization finished.')
  return mlmodel_quantized


def quantize_activations(mlmodel: ct.models.MLModel, sample_data: list[dict]) -> ct.models.MLModel:
  # quantize activations to 8 bits
  act_quant_op_config = OpActivationLinearQuantizerConfig(mode="linear_symmetric",
                                                          dtype="int8")
  act_quant_model_config = cto.OptimizationConfig(global_config=act_quant_op_config)
  mlmodel_quantized = linear_quantize_activations(mlmodel,
                                                  act_quant_model_config,
                                                  sample_data=sample_data)
  print('Activations quantization finished.')
  return mlmodel_quantized


def convert_model():
  # Load the pretrained model
  torch_model = timm.create_model(MODEL_NAME, pretrained=True)
  torch_model.eval()

  # Inspect the model
  print("num_classes", torch_model.num_classes)
  print("data_config", timm.data.resolve_model_data_config(torch_model))

  # Trace the model with random data
  example_input = torch.rand(1, C, H, W)
  traced_model = torch.jit.trace(torch_model, example_input)
  _ = traced_model(example_input)

  # Convert the traced model to CoreML
  ml_model = ct.convert(
    traced_model,
    convert_to="mlprogram",
    inputs=[ct.TensorType(name=INPUT_NAME, shape=example_input.shape)],
    outputs=[ct.TensorType(name=OUTPUT_NAME)],
    # minimum_deployment_target=ct.target.iOS18
  )

  ml_model.short_description = MODEL_NAME

  ml_model.save(MLMODEL_FILE_FP32)
  print('Model converted from PyTorch to Core ML.')

  mlmodel_quantized = quantize_weights(ml_model)
  mlmodel_quantized.save(MLMODEL_FILE_W8)

  # pil_images = load_dummy_images(count=9)
  pil_images = load_images_from_folder(IMAGE_DIR, max_images=999)
  sample_data = preprocess_images(pil_images)
  mlmodel_quantized = quantize_activations(mlmodel_quantized, sample_data)
  mlmodel_quantized.save(MLMODEL_FILE_W8A8)


def test_accuracy(mlmodel_file: str):
  expected_labels = load_labels(LABELS_FILE)
  pil_images = load_images_from_folder(IMAGE_DIR)
  mlmodel = ct.models.MLModel(mlmodel_file)
  batch_size = 999
  correct_predictions = 0
  total_predictions = 0
  total_images = len(pil_images)
  for i in range(0, len(pil_images), batch_size):
    batch_images = pil_images[i:i + batch_size]
    image_data = preprocess_images(batch_images)
    predictions = mlmodel.predict(image_data)
    assert (len(predictions) == len(image_data))
    for j in range(len(image_data)):
      total_predictions += 1
      predicted_label = np.argmax(predictions[j][OUTPUT_NAME])
      expected_label = int(expected_labels[i + j])
      if predicted_label == expected_label:
        correct_predictions += 1
    moving_accuracy = correct_predictions / total_predictions
    print(f'Moving Accuracy: {moving_accuracy * 100:.2f}%. Images processed: {total_predictions}/{total_images}.')
  assert (total_predictions == len(pil_images))
  accuracy = correct_predictions / total_predictions
  print(f'Accuracy: {accuracy * 100:.2f}%. Images processed: {total_predictions}/{total_images}.')


def main():
  convert_model()
  test_accuracy(mlmodel_file=MLMODEL_FILE_W8A8)


if __name__ == "__main__":
  main()
