/* Copyright (c) 2020-2021 Samsung Electronics Co., Ltd. All rights reserved.

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

#ifndef SAMSUNG_SETTINGS_H
#define SAMSUNG_SETTINGS_H

const std::string samsung_settings = R"SETTINGS(
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
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/Samsung/is.nnc"
  md5Checksum: ""
}

benchmark_setting {
  benchmark_id: "IC_tpu_uint8"
  accelerator: "npu"
  accelerator_desc: "npu"
  configuration: "Samsung Exynos"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/Samsung/ic.nnc"
  md5Checksum: ""
}


benchmark_setting {
  benchmark_id: "OD_uint8"
  accelerator: "npu"
  accelerator_desc: "npu"
  configuration: "Samsung Exynos"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/Samsung/od.nnc"
  md5Checksum: ""
}

benchmark_setting {
  benchmark_id: "LU_gpu_float32"
  accelerator: "gpu"
  accelerator_desc: "gpu"
  configuration: "Samsung Exynos"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/Samsung/lu.nnc"
  md5Checksum: ""
}


benchmark_setting {
  benchmark_id: "IC_tpu_uint8_offline"
  accelerator: "npu"
  accelerator_desc: "npu"
  configuration: "Samsung Exynos"
  batch_size: 2048
  src: "https://github.com/mlcommons/mobile_models/raw/main/v1_0/Samsung/ic_offline.nncgo"
  md5Checksum: ""
})SETTINGS";
#endif
