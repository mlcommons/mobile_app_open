/* Copyright 2020-2023 Samsung System LSI. All Rights Reserved.

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

#ifndef MBE_CONFIG_1200_H
#define MBE_CONFIG_1200_H

const std::string mbe1200_config = R"SETTINGS(
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
  accelerator: "npu"
  accelerator_desc: "npu"
  framework: "ENN"
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  model_path: "local:///MLPerf_sideload/ic.nnc"
  model_checksum: "a1f2b667ba702531fb85d83bb4ff4e94"
  single_stream_expected_latency_ns: 900000
}
benchmark_setting {
  benchmark_id: "object_detection"
  accelerator: "npu"
  accelerator_desc: "npu"
  framework: "ENN"
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  model_path: "local:///MLPerf_sideload/od.nnc"
  model_checksum: "0d11a82c8b39da82f620dd972caac3f2"
  single_stream_expected_latency_ns: 1000000
}
benchmark_setting {
  benchmark_id: "image_segmentation_v1"
  accelerator: "npu"
  accelerator_desc: "npu"
  framework: "ENN"
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Int32"
  }
  model_path: "local:///MLPerf_sideload/is.nnc"
  model_checksum: "e852f29a81cafad37e803a0d1af7c533"
  single_stream_expected_latency_ns: 1000000
}
benchmark_setting {
  benchmark_id: "image_segmentation_v2"
  accelerator: "npu"
  accelerator_desc: "npu"
  framework: "ENN"
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Int32"
  }
  model_path: "local:///MLPerf_sideload/sm_uint8.nnc"
  model_checksum: "34fa011cfbf145ccc06072840974d9a1"
  single_stream_expected_latency_ns: 1000000
}
benchmark_setting {
  benchmark_id: "super_resolution"
  accelerator: "samsung_npu"
  accelerator_desc: "NPU"
  framework: "ENN"
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  model_path: "local:///MLPerf_sideload/sr.nnc"
  model_checksum: "6e725dffed620377b9eff463e106e6dd"
  single_stream_expected_latency_ns: 1000000
}
benchmark_setting {
  benchmark_id: "image_classification_offline"
  accelerator: "npu"
  accelerator_desc: "npu"
  framework: "ENN"
  batch_size: 48
  custom_setting {
    id: "i_type"
    value: "Uint8"
  }
  custom_setting {
    id: "o_type"
    value: "Float32"
  }
  model_path: "local:///MLPerf_sideload/ic_offline.nnc"
  model_checksum: "a1f2b667ba702531fb85d83bb4ff4e94"
  single_stream_expected_latency_ns: 1000000
})SETTINGS";
#endif
