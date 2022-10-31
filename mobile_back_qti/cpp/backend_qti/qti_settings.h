/* Copyright (c) 2020-2022 Qualcomm Innovation Center, Inc. All rights reserved.

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

benchmark_setting {
  benchmark_id: "IC_tpu_uint8"
  accelerator: "snpe_aip"
  accelerator_desc: "AIP"
  configuration: "SNPE"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/SNPE/mobilenet_edgetpu_224_1.0_hta.dlc"
  md5_checksum: "73def045aac5a44a152a093d58e04c96"
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
  md5_checksum: "73def045aac5a44a152a093d58e04c96"
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
  md5_checksum: "363d568abb8adc53f3f2480edc7b7f35"
}

benchmark_setting {
  benchmark_id: "LU_gpu_float32"
  accelerator: "gpu_f16"
  accelerator_desc: "GPU (FP16)"
  configuration: "TFLite GPU"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilebert_float_384_gpu.tflite"
  md5_checksum: "36a953d07a8c6f2d3e05b22e87cec95b"
}

benchmark_setting {
  benchmark_id: "IS_uint8_mosaic"
  accelerator: "snpe_aip"
  accelerator_desc: "AIP"
  configuration: "SNPE"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/SNPE/mobile_mosaic_hta.dlc"
  md5_checksum: "d6d74288f81e8d121568e6dff6b771e6"
})SETTINGS";

const std::string qti_settings_sdm888 = R"SETTINGS(
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
  accelerator: "snpe_dsp"
  accelerator_desc: "HTP"
  configuration: "SNPE"
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_1/SNPE/mobilenet_edgetpu_224_1.0_htp.dlc"
  md5_checksum: "2317f5bed0da67b9a13f1b5da4cdff92"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/SNPE/mobilenet_edgetpu_224_1.0_htp_batched.dlc"
  md5_checksum: "d239e3a244da27137ca6dae27facff5a"
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
  md5_checksum: "95fbf908912f9af89bf6006890300e9d"
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
  md5_checksum: "ab97172963ec8a92905c6a2c024557ab"
}

benchmark_setting {
  benchmark_id: "IS_uint8_mosaic"
  accelerator: "snpe_dsp"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/SNPE/mobile_mosaic_htp.dlc"
  md5_checksum: "ebae961e6f0b53bd839f485b125f5e46"
})SETTINGS";

const std::string qti_settings_sdm778 = R"SETTINGS(
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
  accelerator: "snpe_dsp"
  accelerator_desc: "HTP"
  configuration: "SNPE"
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_1/SNPE/mobilenet_edgetpu_224_1.0_htp.dlc"
  md5_checksum: "2317f5bed0da67b9a13f1b5da4cdff92"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/SNPE/mobilenet_edgetpu_224_1.0_htp_batched.dlc"
  md5_checksum: "d239e3a244da27137ca6dae27facff5a"
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
  md5_checksum: "95fbf908912f9af89bf6006890300e9d"
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
  md5_checksum: "ab97172963ec8a92905c6a2c024557ab"
}

benchmark_setting {
  benchmark_id: "IS_uint8_mosaic"
  accelerator: "snpe_dsp"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/SNPE/mobile_mosaic_htp.dlc"
  md5_checksum: "ebae961e6f0b53bd839f485b125f5e46"
})SETTINGS";

const std::string qti_settings_sd7g1 = R"SETTINGS(
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
  accelerator: "snpe_dsp"
  accelerator_desc: "HTP"
  configuration: "SNPE"
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_1/SNPE/mobilenet_edgetpu_224_1.0_htp.dlc"
  md5_checksum: "b0fe84f5d21392198b17b906a97f452d"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_1/SNPE/mobilenet_edgetpu_224_1.0_htp_batched_sd7g1.dlc"
  md5_checksum: "9380ed6edcbb9cd05dc41b7188720b88"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_1/SNPE/ssd_mobiledet_qat_htp.dlc"
  md5_checksum: "6ac3c718f1b6d39f6e9a7700709aa3ff"
}

benchmark_setting {
  benchmark_id: "LU_int8"
  accelerator: "snpe_dsp"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_1/SNPE/mobilebert_quantized_htp.dlc"
  md5_checksum: "896609223af86aefa7f218e515bb977e"
}

benchmark_setting {
  benchmark_id: "IS_uint8_mosaic"
  accelerator: "snpe_dsp"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_1/SNPE/mobile_mosaic_htp.dlc"
  md5_checksum: "7a3f44d5583fae7b2c18fab3e6bcee33"
})SETTINGS";

const std::string qti_settings_sd8g1 = R"SETTINGS(
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
  accelerator: "snpe_dsp"
  accelerator_desc: "HTP"
  configuration: "SNPE"
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/SNPE/mobilenet_edgetpu_224_1.0_htp.dlc"
  md5_checksum: "4e8c9ec583557f8dc341cdcc45dba241"
  single_stream_expected_latency_ns: 600000
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
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/SNPE/mobilenet_edgetpu_224_1.0_htp_batched.dlc"
  md5_checksum: "862ea4d7623c62d49153bad1a774217c"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/SNPE/ssd_mobiledet_qat_htp.dlc"
  md5_checksum: "e55ab007c00629f8616a50d0d9becc26"
}

benchmark_setting {
  benchmark_id: "LU_int8"
  accelerator: "snpe_dsp"
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
  custom_setting {
    id: "perf_profile"
    value: "high_performance"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/SNPE/mobilebert_quantized_htp.dlc"
  md5_checksum: "ad724f945b3745e88158cc5d5de1c2a5"
}

benchmark_setting {
  benchmark_id: "IS_uint8_mosaic"
  accelerator: "snpe_dsp"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/SNPE/mobile_mosaic_htp.dlc"
  md5_checksum: "ebae961e6f0b53bd839f485b125f5e46"
})SETTINGS";

const std::string qti_settings_sd8pg1 = R"SETTINGS(
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
  accelerator: "snpe_dsp"
  accelerator_desc: "HTP"
  configuration: "SNPE"
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_1/SNPE/mobilenet_edgetpu_224_1.0_htp.dlc"
  md5_checksum: "b0fe84f5d21392198b17b906a97f452d"
  single_stream_expected_latency_ns: 600000
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
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_1/SNPE/mobilenet_edgetpu_224_1.0_htp_batched_sd8pg1.dlc"
  md5_checksum: "e2fec06f8d3f1bfeb036a7f6ed012aef"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_1/SNPE/ssd_mobiledet_qat_htp.dlc"
  md5_checksum: "6ac3c718f1b6d39f6e9a7700709aa3ff"
}

benchmark_setting {
  benchmark_id: "LU_int8"
  accelerator: "snpe_dsp"
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
  custom_setting {
    id: "perf_profile"
    value: "high_performance"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_1/SNPE/mobilebert_quantized_htp.dlc"
  md5_checksum: "896609223af86aefa7f218e515bb977e"
}

benchmark_setting {
  benchmark_id: "IS_uint8_mosaic"
  accelerator: "snpe_dsp"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_1/SNPE/mobile_mosaic_htp.dlc"
  md5_checksum: "7a3f44d5583fae7b2c18fab3e6bcee33"
})SETTINGS";

#endif
