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
  acceptable_value {
    value: "1"
    name: "Single thread"
  }
  acceptable_value {
    value: "2"
    name: "2 threads"
  }
  acceptable_value {
    value: "4"
    name: "4 threads"
  }
  acceptable_value {
    value: "8"
    name: "8 threads"
  }
  acceptable_value {
    value: "16"
    name: "16 threads"
  }
}

common_setting {
  id: "configuration"
  name: "Configuration"
  value {
    value: "TFLite backend using NNAPI, GPU delegate or CPU delegate."
    name: "Default"
  }
}

common_setting {
  id: "share_results"
  name: "Share results"
  value {
    value: "0"
    name: "false"
  }
  acceptable_value {
    value: "1"
    name: "true"
  }
  acceptable_value {
    value: "0"
    name: "false"
  }
}

common_setting {
  id: "cooldown"
  name: "Cooldown"
  value {
    value: "0"
    name: "false"
  }
  acceptable_value {
    value: "1"
    name: "true"
  }
  acceptable_value {
    value: "0"
    name: "false"
  }
}

benchmark_setting {
  benchmark_id: "IC_tpu_float32"
  accelerator: "coreml"
  accelerator_desc: "CoreML"
  configuration: "TFLite"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_1/tflite/mobilenet_edgetpu_224_1.0_float.tflite"
  md5Checksum: "66bb4eba50987221608f8487ed405794"
}

benchmark_setting {
  benchmark_id: "IC_tpu_float32_offline"
  accelerator: "coreml"
  accelerator_desc: "CoreML"
  configuration: "TFLite"
  batch_size: 2
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_1/tflite/mobilenet_edgetpu_224_1.0_float.tflite"
  md5Checksum: "66bb4eba50987221608f8487ed405794"
}

benchmark_setting {
  benchmark_id: "OD_float32"
  accelerator: "coreml"
  accelerator_desc: "CoreML"
  configuration: "TFLite"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/tflite/mobiledet.tflite"
  md5Checksum: "566ceb72a4c7c8926fe4ac8eededb5bf"
}

benchmark_setting {
  benchmark_id: "LU_float32"
  accelerator: "metal"
  accelerator_desc: "Metal"
  configuration: "TFLite"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilebert_float_384_gpu.tflite"
  md5Checksum: "36a953d07a8c6f2d3e05b22e87cec95b"
}

benchmark_setting {
  benchmark_id: "IS_float32"
  accelerator: "coreml"
  accelerator_desc: "CoreML"
  configuration: "TFLite"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/deeplabv3_mnv2_ade20k_float.tflite"
  md5Checksum: "e3f7a05b9e0f398c0c0502bab1fc202a"
}

benchmark_setting {
  benchmark_id: "IS_float32_mosaic"
  accelerator: "coreml"
  accelerator_desc: "CoreML"
  configuration: "TFLite"
  src: "https://github.com/mlcommons/mobile_open/raw/main/vision/mosaic/models_and_checkpoints/R4/mobile_segmenter_r4_argmax_f32.tflite"
  md5Checksum: "b3a5d3c2e5756431a471ed5211c344a9"
}

)SETTINGS";

#endif
