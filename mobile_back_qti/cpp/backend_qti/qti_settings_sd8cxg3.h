/* Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.

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

#ifndef QTI_SETTINGS_SD8cxG3_H
#define QTI_SETTINGS_SD8cxG3_H

const std::string qti_settings_sd8cxg3 = R"SETTINGS(
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
  accelerator: "snpe_dsp"
  accelerator_desc: "HTP"
  framework: "SNPE"
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v3_0/SNPE/mobilenet_edgetpu_224_1.0_htp.dlc"
  model_checksum: "2e7c4d33b480b5566bdf05e1204b6152"
  single_stream_expected_latency_ns: 600000
}

benchmark_setting {
  benchmark_id: "image_classification_offline"
  accelerator: "psnpe_dsp"
  accelerator_desc: "HTP"
  framework: "SNPE"
  batch_size: 12288
  custom_setting {
    id: "scenario"
    value: "Offline"
  }
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v3_0/SNPE/mobilenet_edgetpu_224_1.0_htp_batched_sd8cxg3.dlc"
  model_checksum: "4a73e95f0b48384daf94fc53d4a2aec3"
}

benchmark_setting {
  benchmark_id: "object_detection"
  accelerator: "snpe_dsp"
  accelerator_desc: "HTP"
  framework: "SNPE"
  custom_setting {
    id: "snpe_output_layers"
    value: "Postprocessor/BatchMultiClassNonMaxSuppression"
  }
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v3_0/SNPE/ssd_mobiledet_qat_htp.dlc"
  model_checksum: "65937d5b58414d86fc42872945e2f5cf"
}

benchmark_setting {
  benchmark_id: "natural_language_processing"
  accelerator: "snpe_dsp"
  accelerator_desc: "HTP"
  framework: "SNPE"
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
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v3_0/SNPE/mobilebert_quantized_htp.dlc"
  model_checksum: "6dc1f1a47a764381b00c8423b07caac1"
}

benchmark_setting {
  benchmark_id: "image_segmentation_v2"
  accelerator: "snpe_dsp"
  accelerator_desc: "HTP"
  framework: "SNPE"
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
    value: "int_32"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v3_0/SNPE/mobile_mosaic_htp.dlc"
  model_checksum: "e921828320d251a5f7160952bbd750ec"
}

benchmark_setting {
  benchmark_id: "super_resolution"
  accelerator: "snpe_dsp"
  accelerator_desc: "HTP"
  framework: "SNPE"
  custom_setting {
    id: "bg_load"
    value: "true"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v3_0/SNPE/snusr_htp.dlc"
  model_checksum: "3dc4b1e7ae23620704d76b56f88527d0"
})SETTINGS";

#endif
