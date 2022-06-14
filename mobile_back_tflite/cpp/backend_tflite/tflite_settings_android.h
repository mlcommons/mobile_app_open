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
#ifndef TFLITE_SETTINGS_H
#define TFLITE_SETTINGS_H

#include <string>

const std::string tflite_settings = R"SETTINGS(
common_setting {
  id: "num_threads"
  name: "Number of threads"
  value {
    value: "4"
    name: "4 threads"
  }
}

benchmark_setting {
  benchmark_id: "IC_tpu_uint8"
  accelerator: "nnapi"
  accelerator_desc: "NNAPI"
  configuration: "TFLite"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilenet_edgetpu_224_1.0_uint8.tflite"
  md5Checksum: "008dfcb1c1962fedbeef1b998d4c84f2"
}

benchmark_setting {
  benchmark_id: "IC_tpu_uint8_offline"
  accelerator: "nnapi"
  accelerator_desc: "NNAPI"
  configuration: "TFLite"
  batch_size: 2
  src: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilenet_edgetpu_224_1.0_uint8.tflite"
  md5Checksum: "008dfcb1c1962fedbeef1b998d4c84f2"
}

benchmark_setting {
  benchmark_id: "OD_uint8"
  accelerator: "nnapi"
  accelerator_desc: "NNAPI"
  configuration: "TFLite"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/tflite/mobiledet_qat.tflite"
  md5Checksum: "6c7af49d97a2b2488222d94936d2dc18"
}

benchmark_setting {
  benchmark_id: "LU_float32"
  accelerator: "gpu_f16"
  accelerator_desc: "GPU (FP16)"
  configuration: "TFLite"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilebert_float_384_gpu.tflite"
  md5Checksum: "36a953d07a8c6f2d3e05b22e87cec95b"
}

benchmark_setting {
  benchmark_id: "IS_uint8"
  accelerator: "nnapi"
  accelerator_desc: "NNAPI"
  configuration: "TFLite"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/deeplabv3_mnv2_ade20k_uint8.tflite"
  md5Checksum: "1b0a50e380612884f82c157e69c66d22"
}

benchmark_setting {
  benchmark_id: "IS_uint8_mosaic"
  accelerator: "nnapi"
  accelerator_desc: "NNAPI"
  configuration: "TFLite"
  src: "https://github.com/mlcommons/mobile_open/raw/main/vision/mosaic/models_and_checkpoints/R4/mobile_segmenter_r4_quant_argmax_uint8.tflite"
  md5Checksum: "b7a7620b8b818d64305b51ab796bfb1d"
}

)SETTINGS";

#endif
