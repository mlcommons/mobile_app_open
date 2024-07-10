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
MODEL_NAME = "hf-hub:timm/mobilenetv4_conv_large.e600_r384_in1k"

def load_dummy_images(count=9):
  # TODO: Replace this with actual loading of images
  images = []
  for _ in range(count):
    dummy_image = np.random.randint(0, 256, (H, W, C), dtype=np.uint8)
    images.append(Image.fromarray(dummy_image))
  return images


def load_images_from_folder(folder, max_images=99):
  images = []
  filenames = os.listdir(folder)
  if len(filenames) > max_images:
    filenames = np.random.choice(filenames, max_images, replace=False)
  for filename in filenames:
    if filename.lower().endswith((".jpg", ".jpeg", ".png")):
      img_path = os.path.join(folder, filename)
      img = Image.open(img_path).convert('RGB')
      img_array = np.array(img)
      images.append(img_array)
  return images


def load_sample_data():
  # sample_images = load_dummy_images(count=9)
  folder_path = './imagenet'
  sample_images = load_images_from_folder(folder_path, max_images=999)
  print(f'Loaded {len(sample_images)} images from {folder_path}')
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
  sample_data = []
  for image in sample_images:
    img_normalized = transform(image)
    img_np = np.array(img_normalized)
    img_np = img_np.reshape(1, C, H, W)
    assert (img_np.shape == (1, C, H, W))
    sample_data.append({'images': img_np})
  return sample_data


def main():
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
    inputs=[ct.TensorType(name="images", shape=example_input.shape)],
    outputs=[ct.TensorType(name="softmax")],
    # minimum_deployment_target=ct.target.iOS18
  )

  ml_model.short_description = MODEL_NAME

  ml_model.save("mobilenetv4_fp32.mlpackage")
  print('Model converted from PyTorch to Core ML.')

  mlmodel_quantized = quantize_weights(ml_model)
  mlmodel_quantized.save("mobilenetv4_w8.mlpackage")

  sample_data = load_sample_data()
  mlmodel_quantized = quantize_activations(mlmodel_quantized, sample_data)
  mlmodel_quantized.save("mobilenetv4_w8a8.mlpackage")


def quantize_weights(mlmodel):
  # quantize weights to 8 bits
  weight_quant_op_config = cto.OpLinearQuantizerConfig(mode="linear_symmetric",
                                                       dtype="int8")
  weight_quant_model_config = cto.OptimizationConfig(weight_quant_op_config)
  mlmodel_quantized = cto.linear_quantize_weights(mlmodel,
                                                  weight_quant_model_config)
  print('Weights quantization finished.')
  return mlmodel_quantized


def quantize_activations(mlmodel, sample_data):
  # quantize activations to 8 bits
  act_quant_op_config = OpActivationLinearQuantizerConfig(mode="linear_symmetric",
                                                          dtype="int8")
  act_quant_model_config = cto.OptimizationConfig(global_config=act_quant_op_config)
  mlmodel_quantized = linear_quantize_activations(mlmodel,
                                                  act_quant_model_config,
                                                  sample_data=sample_data)
  print('Activations quantization finished.')
  return mlmodel_quantized


if __name__ == "__main__":
  main()
