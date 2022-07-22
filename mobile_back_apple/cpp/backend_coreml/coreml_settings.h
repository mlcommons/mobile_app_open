/* Copyright 2022 The MLPerf Authors. All Rights Reserved.

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
#ifndef COREML_SETTINGS_H
#define COREML_SETTINGS_H

#include <string>

const std::string coreml_settings = R"SETTINGS(
common_setting {
  id: "num_threads"
  name: "Number of threads"
  value {
    value: "4"
    name: "4 threads"
  }
}

benchmark_setting {
  benchmark_id: "IC_tpu_float32"
  accelerator: "coreml"
  accelerator_desc: "Neural Engine"
  configuration: "Core ML"
  src: "https://github.com/mlcommons/mobile_app_open/raw/360-add-coreml-backend/mobile_back_apple/dev-resources/MobilenetEdgeTPU_multi_array_with_shape.mlmodel"
  md5_checksum: "3cbc33aa94c291443010c017d301cd05"
}

)SETTINGS";

#endif

// benchmark_setting {
//  benchmark_id: "IC_tpu_float32_offline"
//  accelerator: "coreml"
//  accelerator_desc: "Neural Engine"
//  configuration: "Core ML"
//  batch_size: 32
//  src:
//  "https://github.com/mlcommons/mobile_app_open/raw/54c01d96f948a588c1bd38d1bc6e27cfe4c8b5bc/mobile_back_apple/dev-resources/MobilenetEdgeTPU_multi_array.mlmodel"
//  md5_checksum: "7921496525e1cd5464745f04e43cc7fd"
//}
//
// benchmark_setting {
//  benchmark_id: "OD_float32"
//  accelerator: "coreml"
//  accelerator_desc: "Neural Engine"
//  configuration: "Core ML"
//  src:
//  "https://raw.githubusercontent.com/mlcommons/mobile_models/main/v1_0/assets/imagenet_val_tiny.txt"
//  md5_checksum: ""
//}
//
// benchmark_setting {
//  benchmark_id: "LU_float32"
//  accelerator: "coreml"
//  accelerator_desc: "Neural Engine"
//  configuration: "Core ML"
//  src:
//  "https://raw.githubusercontent.com/mlcommons/mobile_models/main/v1_0/assets/imagenet_val_tiny.txt"
//  md5_checksum: ""
//}
//
// benchmark_setting {
//  benchmark_id: "IS_float32_mosaic"
//  accelerator: "coreml"
//  accelerator_desc: "Neural Engine"
//  configuration: "Core ML"
//  src:
//  "https://raw.githubusercontent.com/mlcommons/mobile_models/main/v1_0/assets/imagenet_val_tiny.txt"
//  md5_checksum: ""
//}
