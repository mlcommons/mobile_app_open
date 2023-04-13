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
  framework: "TFLite"
  delegate_choice: {
    priority: 2
    delegate_name: "NNAPI"
    accelerator_name: "npu"
    accelerator_desc: "NPU"
    model_path: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilenet_edgetpu_224_1.0_uint8.tflite"
    model_checksum: "008dfcb1c1962fedbeef1b998d4c84f2"
    custom_setting: {
      id: "sample id"
      value: "sample value"
    }
  }
  delegate_choice: {
    priority: 1
    delegate_name: "GPU"
    accelerator_name: "gpu"
    accelerator_desc: "GPU"
    model_path: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilenet_edgetpu_224_1.0_uint8.tflite"
    model_checksum: "008dfcb1c1962fedbeef1b998d4c84f2"
  }
  delegate_selected: "NNAPI"
}

benchmark_setting {
  benchmark_id: "image_classification_offline"
  framework: "TFLite"
  delegate_choice: {
    delegate_name: "NNAPI"
    accelerator_name: "npu"
    accelerator_desc: "NPU"
    model_path: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilenet_edgetpu_224_1.0_uint8.tflite"
    model_checksum: "008dfcb1c1962fedbeef1b998d4c84f2"
    batch_size: 2
  }
  delegate_selected: "NNAPI"
}

benchmark_setting {
  benchmark_id: "object_detection"
  framework: "TFLite"
  delegate_choice: {
    delegate_name: "NNAPI"
    accelerator_name: "npu"
    accelerator_desc: "NPU"
    model_path: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/tflite/mobiledet_qat.tflite"
    model_checksum: "6c7af49d97a2b2488222d94936d2dc18"
  }
  delegate_selected: "NNAPI"
}

benchmark_setting {
  benchmark_id: "natural_language_processing"
  framework: "TFLite"
  delegate_choice: {
    delegate_name: "GPU"
    accelerator_name: "gpu"
    accelerator_desc: "GPU"
    model_path: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilebert_float_384_gpu.tflite"
    model_checksum: "36a953d07a8c6f2d3e05b22e87cec95b"
  }
  delegate_selected: "GPU"
}

benchmark_setting {
  benchmark_id: "image_segmentation_v2"
  framework: "TFLite"
  delegate_choice: {
    delegate_name: "NNAPI"
    accelerator_name: "npu"
    accelerator_desc: "NPU"
    model_path: "https://github.com/mlcommons/mobile_open/raw/main/vision/mosaic/models_and_checkpoints/R4/mobile_segmenter_r4_quant_argmax_uint8.tflite"
    model_checksum: "b7a7620b8b818d64305b51ab796bfb1d"
  }
  delegate_selected: "NNAPI"
}

benchmark_setting {
  benchmark_id: "super_resolution"
  framework: "TFLite"
  delegate_choice: {
    delegate_name: "NNAPI"
    accelerator_name: "npu"
    accelerator_desc: "NPU"
    model_path: "https://github.com/mlcommons/mobile_models/raw/main/v3_0/tflite/edsr_f32b5_full_qint8.tflite"
    model_checksum: "18ce6df0e4603f4b4ee5d04193708d9c"
  }
  delegate_selected: "NNAPI"
}

)SETTINGS";

#endif
