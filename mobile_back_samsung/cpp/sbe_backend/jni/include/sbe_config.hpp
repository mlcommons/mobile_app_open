/* Copyright 2020 The MLPerf Authors. All Rights Reserved.

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

#ifndef SBE_CONFIG_H
#define SBE_CONFIG_H

/**
 * @file sbe_config.hpp
 * @brief description of benchmark_setting for samsung backend core
 * @date 2022-01-04
 * @author soobong Huh (soobong.huh@samsung.com)
 */

namespace sbe {

const std::string sbe2200_flutter_config = R"SETTINGS(
benchmark_setting {
  benchmark_id: "image_classification"
  accelerator: "samsung npu"
  accelerator_desc: "NPU"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1001"
  }
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  custom_setting {
    id: "fpc_mode"
    value: "true"
  }
  custom_setting {
    id: "freezing"
    value: "300"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_single.nnc"
  model_checksum: "a49175f3f4f37f59780995cec540dbf2"
  single_stream_expected_latency_ns: 900000
}

benchmark_setting {
  benchmark_id: "image_segmentation_v1"
  accelerator: "samsung npu"
  accelerator_desc: "NPU"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1004"
  }
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Uint8"
  }
  custom_setting {
    id: "fpc_mode"
    value: "false"
  }
  custom_setting {
    id: "freezing"
    value: "0"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/is.nnc"
  model_checksum: "d7cbd596179beb3c0fe51b745769fc69"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "image_segmentation_v2"
  accelerator: "samsung npu"
  accelerator_desc: "NPU"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1004"
  }
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Uint8"
  }
  custom_setting {
    id: "fpc_mode"
    value: "false"
  }
  custom_setting {
    id: "freezing"
    value: "0"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/sm_uint8.nnc"
  model_checksum: "f715f55818863f371336ad29ecba1183"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "object_detection"
  accelerator: "samsung npu"
  accelerator_desc: "NPU"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1003"
  }
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  custom_setting {
    id: "fpc_mode"
    value: "true"
  }
  custom_setting {
    id: "freezing"
    value: "200"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/od.nnc"
  model_checksum: "6b34201b6696fa75311d0d43820e03d2"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "natural_language_processing"
  accelerator: "gpu"
  accelerator_desc: "gpu"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1000"
  }
  custom_setting {
    id: "i_type"
    value: "Int32"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  custom_setting {
    id: "fpc_mode"
    value: "false"
  }
  custom_setting {
    id: "freezing"
    value: "0"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/mobile_bert_gpu.nnc"
  model_checksum: "d98dfcc37ad33fa7081d6fbb5bc6ddd1"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "image_classification_offline"
  accelerator: "samsung npu"
  accelerator_desc: "npu"
  configuration: "Samsung Exynos"
  batch_size: 8192
  custom_setting {
    id: "scenario"
    value: "offline"
  }
  custom_setting {
    id: "mode"
    value: "1"
  }
  custom_setting {
    id: "preset"
    value: "1002"
  }
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  custom_setting {
    id: "fpc_mode"
    value: "false"
  }
  custom_setting {
    id: "freezing"
    value: "0"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_offline.nnc"
  model_checksum: "8832370c770fa820dfde83e039e3243c"
  single_stream_expected_latency_ns: 1000000
})SETTINGS";

const std::string sbe2200_config_fence_off = R"SETTINGS(
benchmark_setting {
  benchmark_id: "image_classification"
  accelerator: "samsung npu"
  accelerator_desc: "NPU"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1001"
  }
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  custom_setting {
    id: "fpc_mode"
    value: "true"
  }
  custom_setting {
    id: "freezing"
    value: "300"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_single.nnc"
  model_checksum: "a49175f3f4f37f59780995cec540dbf2"
  single_stream_expected_latency_ns: 900000
}

benchmark_setting {
  benchmark_id: "image_segmentation_v1"
  accelerator: "samsung npu"
  accelerator_desc: "NPU"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1004"
  }
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Uint8"
  }
  custom_setting {
    id: "fpc_mode"
    value: "false"
  }
  custom_setting {
    id: "freezing"
    value: "0"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/is.nnc"
  model_checksum: "d7cbd596179beb3c0fe51b745769fc69"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "image_segmentation_v2"
  accelerator: "samsung npu"
  accelerator_desc: "NPU"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1004"
  }
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Uint8"
  }
  custom_setting {
    id: "fpc_mode"
    value: "false"
  }
  custom_setting {
    id: "freezing"
    value: "0"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/sm_uint8.nnc"
  model_checksum: "f715f55818863f371336ad29ecba1183"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "object_detection"
  accelerator: "samsung npu"
  accelerator_desc: "NPU"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1003"
  }
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  custom_setting {
    id: "fpc_mode"
    value: "true"
  }
  custom_setting {
    id: "freezing"
    value: "200"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/od.nnc"
  model_checksum: "6b34201b6696fa75311d0d43820e03d2"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "natural_language_processing"
  accelerator: "gpu"
  accelerator_desc: "gpu"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1000"
  }
  custom_setting {
    id: "i_type"
    value: "Int32"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  custom_setting {
    id: "fpc_mode"
    value: "false"
  }
  custom_setting {
    id: "freezing"
    value: "0"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/mobile_bert_gpu.nnc"
  model_checksum: "d98dfcc37ad33fa7081d6fbb5bc6ddd1"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "image_classification_offline"
  accelerator: "samsung npu"
  accelerator_desc: "npu"
  configuration: "Samsung Exynos"
  batch_size: 8192
  custom_setting {
    id: "scenario"
    value: "offline"
  }
  custom_setting {
    id: "mode"
    value: "1"
  }
  custom_setting {
    id: "preset"
    value: "1002"
  }
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  custom_setting {
    id: "fpc_mode"
    value: "false"
  }
  custom_setting {
    id: "freezing"
    value: "0"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_offline.nnc"
  model_checksum: "8832370c770fa820dfde83e039e3243c"
  single_stream_expected_latency_ns: 1000000
})SETTINGS";

const std::string sbe2200_config = R"SETTINGS(
benchmark_setting {
  benchmark_id: "image_classification"
  accelerator: "samsung npu"
  accelerator_desc: "NPU"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1007"
  }
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  custom_setting {
    id: "fpc_mode"
    value: "true"
  }
  custom_setting {
    id: "freezing"
    value: "200"
  }
  custom_setting {
    id: "lazy_mode"
    value: "true"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_single_fence.nnc"
  model_checksum: "81af8ea507065da2c04a89229a0e4c45"
  single_stream_expected_latency_ns: 900000
}

benchmark_setting {
  benchmark_id: "image_segmentation_v1"
  accelerator: "samsung npu"
  accelerator_desc: "NPU"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1004"
  }
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Uint8"
  }
  custom_setting {
    id: "fpc_mode"
    value: "false"
  }
  custom_setting {
    id: "freezing"
    value: "0"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/is_fence.nnc"
  model_checksum: "a727276c80d7a93073266113fba9beec"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "image_segmentation_v2"
  accelerator: "samsung npu"
  accelerator_desc: "NPU"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1004"
  }
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Uint8"
  }
  custom_setting {
    id: "fpc_mode"
    value: "false"
  }
  custom_setting {
    id: "freezing"
    value: "0"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/sm_uint8_fence.nnc"
  model_checksum: "190169754dc4557725fbe456e31a238e"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "object_detection"
  accelerator: "samsung npu"
  accelerator_desc: "NPU"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1008"
  }
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  custom_setting {
    id: "fpc_mode"
    value: "true"
  }
  custom_setting {
    id: "freezing"
    value: "200"
  }
  custom_setting {
    id: "lazy_mode"
    value: "true"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/od_fence.nnc"
  model_checksum: "e3760bd134eb93438345d7ddbf34ee48"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "natural_language_processing"
  accelerator: "gpu"
  accelerator_desc: "gpu"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1000"
  }
  custom_setting {
    id: "i_type"
    value: "Int32"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  custom_setting {
    id: "fpc_mode"
    value: "false"
  }
  custom_setting {
    id: "freezing"
    value: "0"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/mobile_bert_gpu.nnc"
  model_checksum: "d98dfcc37ad33fa7081d6fbb5bc6ddd1"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "image_classification_offline"
  accelerator: "samsung npu"
  accelerator_desc: "npu"
  configuration: "Samsung Exynos"
  batch_size: 8192
  custom_setting {
    id: "scenario"
    value: "offline"
  }
  custom_setting {
    id: "mode"
    value: "1"
  }
  custom_setting {
    id: "preset"
    value: "1002"
  }
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  custom_setting {
    id: "fpc_mode"
    value: "false"
  }
  custom_setting {
    id: "freezing"
    value: "0"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_offline.nnc"
  model_checksum: "8832370c770fa820dfde83e039e3243c"
  single_stream_expected_latency_ns: 1000000
})SETTINGS";

const std::string sbe1200_config = R"SETTINGS(
benchmark_setting {
  benchmark_id: "image_segmentation_v1"
  accelerator: "npu"
  accelerator_desc: "npu"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Uint8"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/is.nnc"
  model_checksum: "d7cbd596179beb3c0fe51b745769fc69"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "image_classification"
  accelerator: "npu"
  accelerator_desc: "npu"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_single.nnc"
  model_checksum: "a49175f3f4f37f59780995cec540dbf2"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "object_detection"
  accelerator: "npu"
  accelerator_desc: "npu"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/od.nnc"
  model_checksum: "6b34201b6696fa75311d0d43820e03d2"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "image_classification_offline"
  accelerator: "npu"
  accelerator_desc: "npu"
  configuration: "Samsung Exynos"
  batch_size: 48
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  model_path: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_offline.nnc"
  model_checksum: "8832370c770fa820dfde83e039e3243c"
  single_stream_expected_latency_ns: 1000000
})SETTINGS";

}  // namespace sbe
#endif
