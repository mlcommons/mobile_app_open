/* Copyright 2020-2021 The MLPerf Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#ifndef TFLITE_SETTINGS_ANDROID_H
#define TFLITE_SETTINGS_ANDROID_H

#include <string>

const std::string tflite_settings_android = R"SETTINGS(
common_setting {
  id: "num_threads"
  name: "Number of threads"
  value {
    value: "4"
    name: "4 threads"
  }
}

benchmark_setting {
  benchmark_id: "image_classification"
  accelerator: "npu"
  accelerator_desc: "NPU"
  framework: "TFLite"
  delegate_selected: "NNAPI"
  delegate_choice: "CPU"
  delegate_choice: "GPU"
  delegate_choice: "NNAPI"
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilenet_edgetpu_224_1.0_uint8.tflite"
  model_checksum: "008dfcb1c1962fedbeef1b998d4c84f2"
}

benchmark_setting {
  benchmark_id: "image_classification_offline"
  accelerator: "npu"
  accelerator_desc: "NPU"
  framework: "TFLite"
  delegate_selected: "NNAPI"
  delegate_choice: "CPU"
  delegate_choice: "GPU"
  delegate_choice: "NNAPI"
  batch_size: 2
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilenet_edgetpu_224_1.0_uint8.tflite"
  model_checksum: "008dfcb1c1962fedbeef1b998d4c84f2"
}

benchmark_setting {
  benchmark_id: "object_detection"
  accelerator: "npu"
  accelerator_desc: "NPU"
  framework: "TFLite"
  delegate_selected: "NNAPI"
  delegate_choice: "CPU"
  delegate_choice: "GPU"
  delegate_choice: "NNAPI"
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/tflite/mobiledet_qat.tflite"
  model_checksum: "6c7af49d97a2b2488222d94936d2dc18"
}

benchmark_setting {
  benchmark_id: "natural_language_processing"
  accelerator: "gpu"
  accelerator_desc: "GPU"
  framework: "TFLite"
  delegate_selected: "GPU"
  delegate_choice: "CPU"
  delegate_choice: "GPU"
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilebert_float_384_gpu.tflite"
  model_checksum: "36a953d07a8c6f2d3e05b22e87cec95b"
}

benchmark_setting {
  benchmark_id: "image_segmentation_v2"
  accelerator: "npu"
  accelerator_desc: "NPU"
  framework: "TFLite"
  delegate_selected: "NNAPI"
  delegate_choice: "CPU"
  delegate_choice: "GPU"
  delegate_choice: "NNAPI"
  model_path: "https://github.com/mlcommons/mobile_open/raw/main/vision/mosaic/models_and_checkpoints/R4/mobile_segmenter_r4_quant_argmax_uint8.tflite"
  model_checksum: "b7a7620b8b818d64305b51ab796bfb1d"
}

benchmark_setting {
  benchmark_id: "super_resolution"
  accelerator: "npu"
  accelerator_desc: "NPU"
  framework: "TFLite"
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v3_0/tflite/edsr_f32b5_fp32.tflite"
  model_checksum: "672240427c1f3dc33baf2facacd9631f"
}

)SETTINGS";

#endif
