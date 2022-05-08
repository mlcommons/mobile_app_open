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
#ifndef QTI_SETTINGS_H
#define QTI_SETTINGS_H

#include <string>

const std::string empty_settings = "";

const std::string qti_settings_sdm865 = R"SETTINGS(
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
    value: "1"
    name: "true"
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
  accelerator: "snpe_aip"
  accelerator_desc: "AIP"
  configuration: "SNPE"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/SNPE/mobilenet_edgetpu_224_1.0_hta.dlc"
  md5Checksum: ""
}

benchmark_setting {
  benchmark_id: "IC_tpu_uint8_offline"
  accelerator: "psnpe_aip"
  accelerator_desc: "AIP"
  configuration: "SNPE"
  batch_size: 3072
  custom_setting {
    id: "scenario"
    value: "Offline"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/SNPE/mobilenet_edgetpu_224_1.0_hta.dlc"
  md5Checksum: ""
}

benchmark_setting {
  benchmark_id: "OD_uint8"
  accelerator: "snpe_aip"
  accelerator_desc: "AIP"
  configuration: "SNPE"
  custom_setting {
    id: "snpe_output_layers"
    value: "Postprocessor/BatchMultiClassNonMaxSuppression"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/SNPE/ssd_mobiledet_qat_hta.dlc"
  md5Checksum: ""
}

benchmark_setting {
  benchmark_id: "LU_gpu_float32"
  accelerator: "gpu_f16"
  accelerator_desc: "GPU (FP16)"
  configuration: "TFLite GPU"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilebert_float_384_gpu.tflite"
  md5Checksum: ""
}

benchmark_setting {
  benchmark_id: "IS_uint8"
  accelerator: "snpe_aip"
  accelerator_desc: "AIP"
  configuration: "SNPE"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/SNPE/deeplabv3_hta.dlc"
  md5Checksum: ""
})SETTINGS";

const std::string qti_settings_sdm888 = R"SETTINGS(
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
    value: "1"
    name: "true"
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
  accelerator: "snpe_dsp"
  accelerator_desc: "HTP"
  configuration: "SNPE"
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_1/SNPE/mobilenet_edgetpu_224_1.0_htp.dlc"
  md5Checksum: ""
}

benchmark_setting {
  benchmark_id: "IC_tpu_uint8_offline"
  accelerator: "psnpe_dsp"
  accelerator_desc: "HTP"
  configuration: "SNPE"
  batch_size: 12288
  custom_setting {
    id: "scenario"
    value: "Offline"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_1/SNPE/mobilenet_edgetpu_224_1.0_htp_batched.dlc"
  md5Checksum: ""
}

benchmark_setting {
  benchmark_id: "OD_uint8"
  accelerator: "snpe_dsp"
  accelerator_desc: "HTP"
  configuration: "SNPE"
  custom_setting {
    id: "snpe_output_layers"
    value: "Postprocessor/BatchMultiClassNonMaxSuppression"
  }
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_1/SNPE/ssd_mobiledet_qat_htp.dlc"
  md5Checksum: ""
}

benchmark_setting {
  benchmark_id: "LU_int8"
  accelerator: "psnpe_dsp"
  accelerator_desc: "HTP"
  configuration: "SNPE"
  custom_setting {
    id: "snpe_output_layers"
    value: "transpose"
  }
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  custom_setting {
    id: "input_buffer_type"
    value: "float_32"
  }
  custom_setting {
    id: "use_ion_buffer"
    value: "false"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_1/SNPE/mobilebert_quantized_htp.dlc"
  md5Checksum: ""
}

benchmark_setting {
  benchmark_id: "IS_uint8"
  accelerator: "psnpe_dsp"
  accelerator_desc: "HTP"
  configuration: "SNPE"
  custom_setting {
    id: "input_buffer_type"
    value: "uint_8"
  }
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  custom_setting {
    id: "output_buffer_type"
    value: "uint_8"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_1/SNPE/deeplabv3_htp.dlc"
  md5Checksum: ""
})SETTINGS";

const std::string qti_settings_sdm778 = R"SETTINGS(
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
    value: "1"
    name: "true"
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
  accelerator: "snpe_dsp"
  accelerator_desc: "HTP"
  configuration: "SNPE"
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_1/SNPE/mobilenet_edgetpu_224_1.0_htp.dlc"
  md5Checksum: ""
}

benchmark_setting {
  benchmark_id: "IC_tpu_uint8_offline"
  accelerator: "psnpe_dsp"
  accelerator_desc: "HTP"
  configuration: "SNPE"
  batch_size: 12288
  custom_setting {
    id: "scenario"
    value: "Offline"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_1/SNPE/mobilenet_edgetpu_224_1.0_htp_batched.dlc"
  md5Checksum: ""
}

benchmark_setting {
  benchmark_id: "OD_uint8"
  accelerator: "snpe_dsp"
  accelerator_desc: "HTP"
  configuration: "SNPE"
  custom_setting {
    id: "snpe_output_layers"
    value: "Postprocessor/BatchMultiClassNonMaxSuppression"
  }
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_1/SNPE/ssd_mobiledet_qat_htp.dlc"
  md5Checksum: ""
}

benchmark_setting {
  benchmark_id: "LU_int8"
  accelerator: "psnpe_dsp"
  accelerator_desc: "HTP"
  configuration: "SNPE"
  custom_setting {
    id: "snpe_output_layers"
    value: "transpose"
  }
  custom_setting {
    id: "input_buffer_type"
    value: "float_32"
  }
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_1/SNPE/mobilebert_quantized_htp.dlc"
  md5Checksum: ""
}

benchmark_setting {
  benchmark_id: "IS_uint8"
  accelerator: "psnpe_dsp"
  accelerator_desc: "HTP"
  configuration: "SNPE"
  custom_setting {
    id: "input_buffer_type"
    value: "uint_8"
  }
  custom_setting {
    id: "output_buffer_type"
    value: "uint_8"
  }
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_1/SNPE/deeplabv3_htp.dlc"
  md5Checksum: ""
})SETTINGS";
#endif
