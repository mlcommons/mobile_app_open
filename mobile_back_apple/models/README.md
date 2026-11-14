# models

This folder contains scripts to generate Core ML models for the Core ML backend.

Generated models are stored in <https://github.com/mlcommons/mobile_models>.

## Setup

Create a Python virtual environment on macOS and install the dependencies:

```shell
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

## Conversion scripts

| Script | Model | Task | Output Format |
| :--- | :--- | :--- | :--- |
| `mobilenet_edgetpu.py` | MobileNetEdgeTPU (FP16) | Image Classification (v1) | `.mlpackage` |
| `mobilenet_edgetpu_int8.py` | MobileNetEdgeTPU (INT8 W8 / W8A8) | Image Classification (v1) | `.mlpackage` |
| `mobilenetv4_tflite.py` | MobileNetV4-Conv-Large (TF FP16) | Image Classification (v2) | `.mlpackage` |
| `mobilenetv4_tflite_int8.py` | MobileNetV4-Conv-Large (TF INT8 W8 / W8A8) | Image Classification (v2) | `.mlpackage` |
| `mobilenetv4_pytorch.py` | MobileNetV4-Conv-Large (PyTorch FP16 / W8 / W8A8) | Image Classification (v2) | `.mlpackage` |
| `mobiledet.py` | MobileDet SSD (FP16) | Object Detection | `.mlpackage` |
| `mobiledet_int8.py` | MobileDet SSD (INT8 W8 / W8A8) | Object Detection | `.mlpackage` |
| `mosaic.py` | MOSAIC (FP16) | Image Segmentation | `.mlpackage` |
| `mosaic_int8.py` | MOSAIC (INT8 W8 / W8A8) | Image Segmentation | `.mlpackage` |
| `mobilebert.py` | MobileBERT (FP16) | Natural Language Processing | `.mlpackage` |
| `mobilebert_int8.py` | MobileBERT (INT8 W8 / W8A8) | Natural Language Processing | `.mlpackage` |
| `edsr.py` | EDSR (`edsr_f32b5` FP16) | Super Resolution | `.mlpackage` |
| `edsr_int8.py` | EDSR (`edsr_f32b5` INT8 W8 / W8A8) | Super Resolution | `.mlpackage` |
| `tflite_to_coreml.py` | Any quantized INT8/UINT8 `.tflite` | Direct TFLite W8A8 -> Core ML | `.mlpackage` |

## Helpful resources

* API Reference: <https://apple.github.io/coremltools/index.html>
* Core ML Specification: <https://apple.github.io/coremltools/mlmodel/index.html>
* Supported
  layers: <https://github.com/apple/coremltools/blob/main/mlmodel/format/NeuralNetwork.proto> (
  also see other `*.proto` files in the same directory)
