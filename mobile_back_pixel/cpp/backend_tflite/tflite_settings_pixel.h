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
  benchmark_id: "IC_tpu_uint8"
  accelerator: "nnapi"
  accelerator_desc: "NNAPI"
  configuration: "TFLite"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilenet_edgetpu_224_1.0_uint8.tflite"
}

benchmark_setting {
  benchmark_id: "IC_tpu_uint8_offline"
  accelerator: "nnapi"
  accelerator_desc: "NNAPI"
  configuration: "TFLite"
  batch_size: 64
  src: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilenet_edgetpu_224_1.0_uint8.tflite"
}

benchmark_setting {
  benchmark_id: "OD_uint8"
  accelerator: "nnapi"
  accelerator_desc: "NNAPI"
  configuration: "TFLite"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/tflite/mobiledet_qat.tflite"
}

benchmark_setting {
  benchmark_id: "LU_float32"
  accelerator: "nnapi"
  accelerator_desc: "NNAPI"
  configuration: "TFLite"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilebert_int8_384_nnapi.tflite"
}

benchmark_setting {
  benchmark_id: "IS_uint8"
  accelerator: "nnapi"
  accelerator_desc: "NNAPI"
  configuration: "TFLite"
  src: "https://github.com/mlcommons/mobile_models/raw/Google/v1_0/Google/deeplabv3.tflite"
})SETTINGS";

#endif
