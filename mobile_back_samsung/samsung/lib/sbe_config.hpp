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
const std::string sbe2300_config = R"SETTINGS(
benchmark_setting {
  benchmark_id: "IC_tpu_uint8"
  accelerator: "samsung npu"
  accelerator_desc: "NPU"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1009"
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
    id: "extension"
    value: "true"
  }
  custom_setting {
    id: "lazy_mode"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_1/Samsung/ic_single_fence.nnc"
  md5_checksum: "a451da1f48b1fad01c17fb7a49e5822e"
  single_stream_expected_latency_ns: 500000
}

benchmark_setting {
  benchmark_id: "IS_uint8_mosaic"
  accelerator: "samsung npu"
  accelerator_desc: "NPU"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1009"
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
    id: "extension"
    value: "true"
  }
  custom_setting {
    id: "lazy_mode"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_1/Samsung/sm_uint8_fence.nnc"
  md5_checksum: "08fa7b354f82140a8863fed57c2d499b"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "OD_uint8"
  accelerator: "samsung npu"
  accelerator_desc: "NPU"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1009"
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
    id: "extension"
    value: "true"
  }
  custom_setting {
    id: "lazy_mode"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_1/Samsung/od_fence.nnc"
  md5_checksum: "8a7e1808446072545c990f3d219255c6"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "LU_gpu_float32"
  accelerator: "npu"
  accelerator_desc: "npu"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "mode"
    value: "3"
  }
  custom_setting {
    id: "preset"
    value: "1009"
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
    id: "extension"
    value: "true"
  }
  custom_setting {
    id: "lazy_mode"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_1/Samsung/mobile_bert_fence.nnc"
  md5_checksum: "5fb666b684a9bd0b68d497128b990137"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "IC_tpu_uint8_offline"
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
    id: "extension"
    value: "false"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_1/Samsung/ic_offline.nnc"
  md5_checksum: "6885f281a3d7a7ec3549d629dff8c8ac"
  single_stream_expected_latency_ns: 1000000
})SETTINGS";

const std::string sbe2200_config = R"SETTINGS(
benchmark_setting {
  benchmark_id: "IC_tpu_uint8"
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
    id: "extension"
    value: "true"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_single.nnc"
  md5_checksum: "a49175f3f4f37f59780995cec540dbf2"
  single_stream_expected_latency_ns: 900000
}
benchmark_setting {
  benchmark_id: "IS_uint8_mosaic"
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
    id: "extension"
    value: "false"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/sm_uint8.nnc"
  md5_checksum: "f715f55818863f371336ad29ecba1183"
  single_stream_expected_latency_ns: 1000000
}
benchmark_setting {
  benchmark_id: "OD_uint8"
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
    id: "extension"
    value: "true"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/od.nnc"
  md5_checksum: "6b34201b6696fa75311d0d43820e03d2"
  single_stream_expected_latency_ns: 1000000
}
benchmark_setting {
  benchmark_id: "LU_gpu_float32"
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
    id: "extension"
    value: "false"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/mobile_bert_gpu.nnc"
  md5_checksum: "d98dfcc37ad33fa7081d6fbb5bc6ddd1"
  single_stream_expected_latency_ns: 1000000
}
benchmark_setting {
  benchmark_id: "IC_tpu_uint8_offline"
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
    id: "extension"
    value: "false"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_offline.nnc"
  md5_checksum: "8832370c770fa820dfde83e039e3243c"
  single_stream_expected_latency_ns: 1000000
})SETTINGS";

const std::string sbe1200_config = R"SETTINGS(
benchmark_setting {
  benchmark_id: "IS_uint8_mosaic"
  accelerator: "npu"
  accelerator_desc: "npu"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Int32"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/is.nnc"
  md5_checksum: "d7cbd596179beb3c0fe51b745769fc69"
  single_stream_expected_latency_ns: 1000000
}
benchmark_setting {
  benchmark_id: "IC_tpu_uint8"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_single.nnc"
  md5_checksum: "a49175f3f4f37f59780995cec540dbf2"
  single_stream_expected_latency_ns: 900000
}

benchmark_setting {
  benchmark_id: "OD_uint8"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/od.nnc"
  md5_checksum: "6b34201b6696fa75311d0d43820e03d2"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "IC_tpu_uint8_offline"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_offline.nnc"
  md5_checksum: "8832370c770fa820dfde83e039e3243c"
  single_stream_expected_latency_ns: 1000000
})SETTINGS";

const std::string sbe2100_config = R"SETTINGS(
benchmark_setting {
  benchmark_id: "IC_tpu_uint8"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/Samsung/ic.nnc"
  md5_checksum: "955ef2ac3c134820eab901f3dac9f732"
  single_stream_expected_latency_ns: 900000
}

benchmark_setting {
  benchmark_id: "IS_uint8"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/Samsung/is.nnc"
  md5_checksum: "b501ed669da753b08a151639798af37e"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "IS_uint8_mosaic"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/Samsung/sm_uint8.nnc"
  md5_checksum: "483eee2df253ecc135a6e8701cc0c909"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "OD_uint8"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/Samsung/od.nnc"
  md5_checksum: "a3c7b5e8d6b978c05807e8926584758c"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "LU_gpu_float32"
  accelerator: "gpu"
  accelerator_desc: "gpu"
  configuration: "Samsung Exynos"
  custom_setting {
    id: "i_type"
    value: "Int32"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/Samsung/lu.nnc"
  md5_checksum: "215ee3b9224d15dc50b30d56fa7b7396"
  single_stream_expected_latency_ns: 1000000
}

benchmark_setting {
  benchmark_id: "IC_tpu_uint8_offline"
  accelerator: "npudsp"
  accelerator_desc: "npu"
  configuration: "Samsung Exynos"
  batch_size: 8192
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/Samsung/ic_offline.nncgo"
  md5_checksum: "c38acf6c66ca32c525c14ce25ead823a"
  single_stream_expected_latency_ns: 1000000
})SETTINGS";

}	// namespace sbe
#endif
