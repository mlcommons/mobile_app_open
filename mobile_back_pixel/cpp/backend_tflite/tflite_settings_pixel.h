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
#include <string>

#ifndef TFLITE_SETTINGS_H
#define TFLITE_SETTINGS_H

const std::string tflite_settings = R"SETTINGS(
common_setting {
  id: "num_threads"
  name: "Number of threads"
  value {
    value: "2"
    name: "2 threads"
  }
}

benchmark_setting {
  benchmark_id: "image_classification"
  accelerator: "tpu"
  accelerator_desc: "Google Edge TPU"
  framework: "TFLite NNAPI"
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilenet_edgetpu_224_1.0_uint8.tflite"
  model_checksum: "008dfcb1c1962fedbeef1b998d4c84f2"
}

benchmark_setting {
  benchmark_id: "image_classification_offline"
  accelerator: "tpu"
  accelerator_desc: "Google Edge TPU"
  framework: "TFLite NNAPI"
  batch_size: 64
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilenet_edgetpu_224_1.0_uint8.tflite"
  model_checksum: "008dfcb1c1962fedbeef1b998d4c84f2"
}

benchmark_setting {
  benchmark_id: "object_detection"
  accelerator: "tpu"
  accelerator_desc: "Google Edge TPU"
  framework: "TFLite NNAPI"
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/tflite/mobiledet_qat.tflite"
  model_checksum: "6c7af49d97a2b2488222d94936d2dc18"
}

benchmark_setting {
  benchmark_id: "natural_language_processing"
  accelerator: "tpu"
  accelerator_desc: "Google Edge TPU"
  framework: "TFLite NNAPI"
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilebert_int8_384_nnapi.tflite"
  model_checksum: "3944a2dee04a5f8a5fd016ac34c4d390"
}

benchmark_setting {
  benchmark_id: "image_segmentation_v1"
  accelerator: "tpu"
  accelerator_desc: "Google Edge TPU"
  framework: "TFLite NNAPI"
  model_path: "https://github.com/mlcommons/mobile_models/raw/Google/v1_0/Google/deeplabv3.tflite"
  model_checksum: "7da3ac5017f6527eed5c85295f7695f4"
}

benchmark_setting {
  benchmark_id: "image_segmentation_v2"
  accelerator: "tpu"
  accelerator_desc: "Google Edge TPU"
  framework: "TFLite NNAPI"
  model_path: "https://github.com/mlcommons/mobile_open/raw/main/vision/mosaic/models_and_checkpoints/R4/mobile_segmenter_r4_quant_argmax_uint8.tflite"
  model_checksum: "b7a7620b8b818d64305b51ab796bfb1d"
}

benchmark_setting {
  benchmark_id: "super_resolution"
  accelerator: "tpu"
  accelerator_desc: "Google Edge TPU"
  framework: "TFLite NNAPI"
  model_path: "https://github.com/mlcommons/mobile_models/raw/anh/super-resolution/v3_0/tflite/edsr_f32b5_fp32.tflite"
  model_checksum: "672240427c1f3dc33baf2facacd9631f"
}

)SETTINGS";

#endif
