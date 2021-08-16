/* Copyright (c) 2020-2021 Qualcomm Innovation Center, Inc. All rights reserved.

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
#ifndef SNPE_SETTINGS_H
#define SNPE_SETTINGS_H

#include <string>

const std::string empty_settings = "";

const std::string qti_settings_356 = R"SETTINGS(
common_setting {
  id: "num_threads"
  name: "Number of threads"
  value {
    value: "4"
    name: "4 threads"
  }
}

common_setting {
  id: "configuration"
  name: "Configuration"
  value {
    value: "QTI backend using SNPE, NNAPI and TFLite GPU Delegate"
    name: "QTI"
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
  benchmark_id: "LU_gpu_float32"
  setting {
    id: "accelerator"
    name: "accelerator"
    value {
      value: "gpu_f16"
      name: "GPU(Float 16)"
    }
  }
  setting {
    id: "configuration"
    name: "Configuration"
    value {
      value: "QTI backend using SNPE, NNAPI and TFLite GPU Delegate"
      name: "TFLite"
    }
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilebert_float_384_gpu.tflite"
})SETTINGS";

const std::string qti_settings_415 = R"SETTINGS(
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
  name: "configuration"
  value {
    value: "QTI backend using SNPE, NNAPI and TFLite GPU Delegate"
    name: "QTI"
  }
}

common_setting {
  id: "share_results"
  name: "Share results"
  value {
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
}

benchmark_setting {
  benchmark_id: "OD_uint8"
  setting {
    id: "accelerator"
    name: "accelerator"
    value {
      value: "NNAPI"
      name: "NNAPI"
    }
  }
  setting {
    id: "configuration"
    name: "Configuration"
    value {
      value: "QTI backend using SNPE, NNAPI and TFLite GPU Delegate"
      name: "TFLite"
    }
  }
  setting {
    id: "snpeOutputLayers"
    name: "snpeOutputLayerName"
    value {
        value: "Postprocessor/BatchMultiClassNonMaxSuppression,add"
        name: "snpeOutputLayerName"
    }
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/ssd_mobilenet_v2_300_uint8.tflite"
}

benchmark_setting {
  benchmark_id: "LU_gpu_float32"
  setting {
    id: "accelerator"
    name: "accelerator"
    value {
      value: "gpu_f16"
      name: "GPU(Float 16)"
    }
  }
  setting {
    id: "configuration"
    name: "Configuration"
    value {
      value: "QTI backend using SNPE, NNAPI and TFLite GPU Delegate"
      name: "TFLite"
    }
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilebert_float_384_gpu.tflite"
})SETTINGS";

#endif
