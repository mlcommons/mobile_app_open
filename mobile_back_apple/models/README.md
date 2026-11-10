# models

This folder contains scripts to generate Core ML models for the Core ML backend.

Generated models are stored in <https://github.com/mlcommons/mobile_models>.

The scripts need Python 3.11 on macOS:

```shell
python3.11 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

## Helpful resources

* API Reference: <https://apple.github.io/coremltools/index.html>
* Core ML Specification: <https://apple.github.io/coremltools/mlmodel/index.html>
* Supported
  layers: <https://github.com/apple/coremltools/blob/main/mlmodel/format/NeuralNetwork.proto> (
  also see other `*.proto` files in the same directory)
