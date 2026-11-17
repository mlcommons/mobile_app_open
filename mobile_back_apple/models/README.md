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

## Converting the 5 Core ML Benchmarks (FP16 & Quantized W8A8)

Run all commands from `mobile_back_apple/models/` with the virtual environment activated:

### 1. Image Classification v2 (`image_classification_v2`)
- **FP16** (`MobilenetV4_Large_NCHW.mlpackage`, transpose-free NCHW `(1, 3, 384, 384)`):
  ```shell
  python3 mobilenetv4_tflite.py
  ```
  Source: `../dev-resources/mobilenet_v4/MobileNetV4-Conv-Large-saved-model` -> `../dev-resources/mobilenet_v4/MobilenetV4_Large_NCHW.mlpackage`.
  *(Requires `custom_setting { id: "data-format" value: "NCHW" }` in `coreml_settings.pbtxt`.)*
- **Quantized W8A8** (`MobilenetV4_Large_NCHW_w8a8.mlpackage`, transpose-free NCHW `(1, 3, 384, 384)`):
  ```shell
  python3 mobilenetv4_tflite_int8.py
  ```
  Source: `../dev-resources/mobilenet_v4/MobileNetV4-Conv-Large-saved-model` -> `../dev-resources/mobilenet_v4/MobilenetV4_Large_NCHW_w8a8.mlpackage`.
  *(Requires `custom_setting { id: "data-format" value: "NCHW" }` in `coreml_settings.pbtxt`.)*

### 2. Object Detection (`object_detection`)
- **FP16** (`MobileDet.mlpackage`):
  ```shell
  python3 mobiledet.py
  ```
  Source: `../dev-resources/mobiledet/saved_model` -> `../dev-resources/mobiledet/MobileDet.mlpackage`.
- **Quantized W8A8** (`MobileDet_w8a8.mlpackage`):
  ```shell
  python3 mobiledet_int8.py
  ```
  Source: `../dev-resources/mobiledet/saved_model` -> `../dev-resources/mobiledet/MobileDet_w8a8.mlpackage`.

### 3. Image Segmentation v2 (`image_segmentation_v2`)
- **FP16** (`Mosaic.mlpackage`):
  ```shell
  python3 mosaic.py
  ```
  Source: `../dev-resources/mosaic/mobile_segmenter_r4_frozen_graph.pb` -> `../dev-resources/mosaic/Mosaic.mlpackage`.
- **Quantized W8A8** (`mosaic_tflite_asym.mlpackage`):
  ```shell
  python3 tflite_to_coreml.py <mosaic_int8.tflite> <mosaic_tflite_asym.mlpackage>
  ```
  *(Alternatively, `python3 mosaic_int8.py` quantizes from the frozen graph via `coremltools.optimize`.)*

### 4. Natural Language Processing (`natural_language_processing`)
- **FP16** (`MobileBERT.mlpackage`):
  ```shell
  python3 mobilebert.py
  ```
  Source: `../dev-resources/mobilebert/float_saved_model` -> `../dev-resources/mobilebert/MobileBERT.mlpackage`.
- **Quantized W8A8** (`mobilebert_nnapi_sym_lin.mlpackage`):
  ```shell
  python3 tflite_to_coreml.py --symmetric <mobilebert_int8_384_nnapi.tflite> <mobilebert_nnapi_sym_lin.mlpackage>
  ```

### 5. Super Resolution (`super_resolution`)
- **FP16** (`edsr_f32b5_fp32.mlpackage`):
  ```shell
  python3 edsr.py
  ```
  Source: `../dev-resources/edsr/f32b5/ckpt` -> `../dev-resources/edsr/edsr_f32b5_fp32.mlpackage`.
- **Quantized W8A8** (`edsr_f32b5_tflite_sym.mlpackage`):
  ```shell
  python3 tflite_to_coreml.py --symmetric <edsr_f32b5_int8.tflite> <edsr_f32b5_tflite_sym.mlpackage>
  ```
  *(Alternatively, `python3 edsr_int8.py` quantizes from the SavedModel via `coremltools.optimize`.)*

### Packaging for `coreml_settings.pbtxt`
Zip each `.mlpackage` directory and compute its MD5 checksum:
```shell
zip -r <Model>.mlpackage.zip <Model>.mlpackage
md5 <Model>.mlpackage.zip
```

## Helpful resources

* API Reference: <https://apple.github.io/coremltools/index.html>
* Core ML Specification: <https://apple.github.io/coremltools/mlmodel/index.html>
* Supported
  layers: <https://github.com/apple/coremltools/blob/main/mlmodel/format/NeuralNetwork.proto> (
  also see other `*.proto` files in the same directory)
