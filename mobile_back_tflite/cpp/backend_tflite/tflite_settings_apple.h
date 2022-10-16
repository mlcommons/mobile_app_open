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
#ifndef TFLITE_SETTINGS_APPLE_H
#define TFLITE_SETTINGS_APPLE_H

#include <string>

static std::string tflite_settings_apple;

const std::string tflite_settings_apple_main = R"SETTINGS(
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
  accelerator: "coreml"
  accelerator_desc: "CoreML"
  framework: "TFLite"
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v1_1/tflite/mobilenet_edgetpu_224_1.0_float.tflite"
  model_checksum: "66bb4eba50987221608f8487ed405794"
}

benchmark_setting {
  benchmark_id: "object_detection"
  accelerator: "coreml"
  accelerator_desc: "CoreML"
  framework: "TFLite"
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/tflite/mobiledet.tflite"
  model_checksum: "566ceb72a4c7c8926fe4ac8eededb5bf"
}

benchmark_setting {
  benchmark_id: "natural_language_processing"
  accelerator: "metal"
  accelerator_desc: "Metal"
  framework: "TFLite"
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilebert_float_384_gpu.tflite"
  model_checksum: "36a953d07a8c6f2d3e05b22e87cec95b"
}

benchmark_setting {
  benchmark_id: "image_segmentation_v2"
  accelerator: "coreml"
  accelerator_desc: "CoreML"
  framework: "TFLite"
  model_path: "https://github.com/mlcommons/mobile_open/raw/main/vision/mosaic/models_and_checkpoints/R4/mobile_segmenter_r4_argmax_f32.tflite"
  model_checksum: "b3a5d3c2e5756431a471ed5211c344a9"
}

)SETTINGS";

const std::string tflite_settings_apple_iphoneX = R"SETTINGS(

benchmark_setting {
  benchmark_id: "image_classification_offline"
  accelerator: "coreml"
  accelerator_desc: "CoreML"
  framework: "TFLite"
  batch_size: 32
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v1_1/tflite/mobilenet_edgetpu_224_1.0_float.tflite"
  model_checksum: "66bb4eba50987221608f8487ed405794"
}

common_setting {
  id: "shards_num"
  name: "Number of threads for inference"
  value {
    value: "8"
    name: "8"
  }
}

)SETTINGS";

const std::string tflite_settings_apple_iphone11 = R"SETTINGS(

benchmark_setting {
  benchmark_id: "image_classification_offline"
  accelerator: "coreml"
  accelerator_desc: "CoreML"
  framework: "TFLite"
  batch_size: 64
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v1_1/tflite/mobilenet_edgetpu_224_1.0_float.tflite"
  model_checksum: "66bb4eba50987221608f8487ed405794"
}

common_setting {
  id: "shards_num"
  name: "Number of threads for inference"
  value {
    value: "8"
    name: "8"
  }
}

)SETTINGS";

const std::string tflite_settings_apple_iphone12 = R"SETTINGS(

benchmark_setting {
  benchmark_id: "image_classification_offline"
  accelerator: "coreml"
  accelerator_desc: "CoreML"
  framework: "TFLite"
  batch_size: 8
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v1_1/tflite/mobilenet_edgetpu_224_1.0_float.tflite"
  model_checksum: "66bb4eba50987221608f8487ed405794"
}

common_setting {
  id: "shards_num"
  name: "Number of threads for inference"
  value {
    value: "4"
    name: "4"
  }
}

)SETTINGS";

#endif
