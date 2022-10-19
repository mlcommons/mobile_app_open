/* Copyright 2021 The MLPerf Authors. All Rights Reserved.

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
#ifndef TFLITE_SETTINGS_MTK_H
#define TFLITE_SETTINGS_MTK_H

#include <string>

const std::string tflite_settings_mtk = R"SETTINGS(
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
  accelerator: "neuron"
  accelerator_desc: "MediaTek NN accelerator via the Neuron Delegate"
  configuration: "TFLite"
  batch_size: 1
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilenet_edgetpu_224_1.0_uint8.tflite"
  model_checksum: "008dfcb1c1962fedbeef1b998d4c84f2"
}

benchmark_setting {
  benchmark_id: "image_classification_offline"
  accelerator: "neuron"
  accelerator_desc: "MediaTek NN accelerator via the Neuron Delegate"
  framework: "TFLite"
  batch_size: 256
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilenet_edgetpu_224_1.0_uint8.tflite"
  model_checksum: "008dfcb1c1962fedbeef1b998d4c84f2"
}

benchmark_setting {
  benchmark_id: "object_detection"
  accelerator: "neuron"
  accelerator_desc: "MediaTek NN accelerator + CPU via the Neuron Delegate"
  framework: "TFLite"
  batch_size: 1
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/tflite/mobiledet_qat.tflite"
  model_checksum: "6c7af49d97a2b2488222d94936d2dc18"
}

benchmark_setting {
  benchmark_id: "natural_language_processing"
  accelerator: "neuron"
  accelerator_desc: "MediaTek NN accelerator + VPU via the Neuron Delegate"
  framework: "TFLite"
  batch_size: 1
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilebert_int8_384_20200602.tflite"
  model_checksum: "3a636c066ca2916e1858266857e96c72"
}

benchmark_setting {
  benchmark_id: "image_segmentation_v1"
  accelerator: "neuron"
  accelerator_desc: "MediaTek NN accelerator via the Neuron Delegate"
  framework: "TFLite"
  batch_size: 1
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/deeplabv3_mnv2_ade20k_uint8.tflite"
  model_checksum: "1b0a50e380612884f82c157e69c66d22"
}

benchmark_setting {
  benchmark_id: "image_segmentation_v2"
  accelerator: "neuron"
  accelerator_desc: "Neuron"
  framework: "TFLite"
  model_path: "https://github.com/mlcommons/mobile_open/raw/main/vision/mosaic/models_and_checkpoints/R4/mobile_segmenter_r4_quant_argmax_uint8.tflite"
  model_checksum: "b7a7620b8b818d64305b51ab796bfb1d"
}

)SETTINGS";

#endif
