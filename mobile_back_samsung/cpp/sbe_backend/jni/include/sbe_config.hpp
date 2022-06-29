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
    value: "Samsung's Exynos Neural Network SDK running\non Exynos 990 mobile processor."
    name: "Samsung Exynos"
  }
  acceptable_value {
    value: "Samsung's Exynos Neural Network SDK running\non Exynos 990 mobile processor."
    name: "Samsung Exynos"
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
    value: "200"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/Samsung/ic_single.nnc"
  single_stream_expected_latency_ns: 900000
}

benchmark_setting {
  benchmark_id: "IS_uint8"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/is.nnc"
  single_stream_expected_latency_ns: 1000000
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/sm_uint8.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/od.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/mobile_bert_gpu.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_offline.nnc"
  single_stream_expected_latency_ns: 100000
})SETTINGS";

const std::string sbe2300_wo_fpc_config = R"SETTINGS(
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
    value: "Samsung's Exynos Neural Network SDK running\non Exynos 990 mobile processor."
    name: "Samsung Exynos"
  }
  acceptable_value {
    value: "Samsung's Exynos Neural Network SDK running\non Exynos 990 mobile processor."
    name: "Samsung Exynos"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_single.nnc"
  single_stream_expected_latency_ns: 900000
}

benchmark_setting {
  benchmark_id: "IS_uint8"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/is.nnc"
  single_stream_expected_latency_ns: 1000000
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/sm_uint8.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/od.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/mobile_bert_gpu.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_offline.nnc"
  single_stream_expected_latency_ns: 1000000
})SETTINGS";

const std::string sbe2200_config_fence_off = R"SETTINGS(
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
    value: "Samsung's Exynos Neural Network SDK running\non Exynos 990 mobile processor."
    name: "Samsung Exynos"
  }
  acceptable_value {
    value: "Samsung's Exynos Neural Network SDK running\non Exynos 990 mobile processor."
    name: "Samsung Exynos"
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
    value: "200"
  }
  custom_setting {
    id: "lazy_mode"
    value: "false"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_single.nnc"
  single_stream_expected_latency_ns: 900000
}

benchmark_setting {
  benchmark_id: "IS_uint8"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/is.nnc"
  single_stream_expected_latency_ns: 1000000
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/sm_uint8.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/od.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/mobile_bert_gpu.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_offline.nnc"
  single_stream_expected_latency_ns: 1000000
})SETTINGS";

const std::string sbe2200_config_fpc_off_fence_off = R"SETTINGS(
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
    value: "Samsung's Exynos Neural Network SDK running\non Exynos 990 mobile processor."
    name: "Samsung Exynos"
  }
  acceptable_value {
    value: "Samsung's Exynos Neural Network SDK running\non Exynos 990 mobile processor."
    name: "Samsung Exynos"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_single.nnc"
  single_stream_expected_latency_ns: 900000
}

benchmark_setting {
  benchmark_id: "IS_uint8"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/is.nnc"
  single_stream_expected_latency_ns: 1000000
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/sm_uint8.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/od.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/mobile_bert_gpu.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_offline.nnc"
  single_stream_expected_latency_ns: 1000000
})SETTINGS";


const std::string sbe2200_config = R"SETTINGS(
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
    value: "Samsung's Exynos Neural Network SDK running\non Exynos 990 mobile processor."
    name: "Samsung Exynos"
  }
  acceptable_value {
    value: "Samsung's Exynos Neural Network SDK running\non Exynos 990 mobile processor."
    name: "Samsung Exynos"
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
    value: "300"
  }
  custom_setting {
    id: "lazy_mode"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_single_fence.nnc"
  single_stream_expected_latency_ns: 900000
}

benchmark_setting {
  benchmark_id: "IS_uint8"
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
    id: "fpc_mode"
    value: "false"
  }
  custom_setting {
    id: "freezing"
    value: "0"
  }
  custom_setting {
    id: "lazy_mode"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/is_fence.nnc"
  single_stream_expected_latency_ns: 1000000
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
    id: "fpc_mode"
    value: "false"
  }
  custom_setting {
    id: "freezing"
    value: "0"
  }
  custom_setting {
    id: "lazy_mode"
    value: "true"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/sm_uint8_fence.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/od_fence.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/mobile_bert_gpu.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_offline.nnc"
  single_stream_expected_latency_ns: 1000000
})SETTINGS";

const std::string sbe2200_release_config = R"SETTINGS(
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
    value: "Samsung's Exynos Neural Network SDK running\non Exynos 990 mobile processor."
    name: "Samsung Exynos"
  }
  acceptable_value {
    value: "Samsung's Exynos Neural Network SDK running\non Exynos 990 mobile processor."
    name: "Samsung Exynos"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_single_fence.nnc"
  single_stream_expected_latency_ns: 900000
}

benchmark_setting {
  benchmark_id: "IS_uint8"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/is.nnc"
  single_stream_expected_latency_ns: 1000000
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/sm_uint8.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/od_fence.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/mobile_bert_gpu.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/ic_offline.nnc"
  single_stream_expected_latency_ns: 1000000
})SETTINGS";

const std::string sbe1200_config = R"SETTINGS(
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
    value: "Samsung's Exynos Neural Network SDK running\non Exynos 990 mobile processor."
    name: "Samsung Exynos"
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
    value: "Int32"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/is.nnc"
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
    value: "Int32"
  }
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/sm_uint8.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v2_0/Samsung/mobile_bert_gpu.nnc"
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
  single_stream_expected_latency_ns: 1000000
})SETTINGS";

const std::string sbe2100_config = R"SETTINGS(
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
    value: "Samsung's Exynos Neural Network SDK running\non Exynos 990 mobile processor."
    name: "Samsung Exynos"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/Samsung/ic.nnc"
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/Samsung/od.nnc"
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
  single_stream_expected_latency_ns: 1000000
})SETTINGS";

}	// namespace sbe
#endif
