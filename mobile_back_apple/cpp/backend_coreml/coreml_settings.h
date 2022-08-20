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

benchmark_setting {
  benchmark_id: "image_classification"
  accelerator: "ANE"
  accelerator_desc: "Apple Neural Engine"
  configuration: "Default"
  model_path: "https://github.com/mlcommons/mobile_models/raw/coreml/beta/CoreML/MobilenetEdgeTPU.mlmodel"
  model_checksum: "39483b20b878d46144ab4cfe9a3e5600"
}

benchmark_setting {
  benchmark_id: "image_classification_offline"
  accelerator: "ANE"
  accelerator_desc: "Apple Neural Engine"
  configuration: "Default"
  batch_size: 32
  model_path: "https://github.com/mlcommons/mobile_models/raw/coreml/beta/CoreML/MobilenetEdgeTPU.mlmodel"
  model_checksum: "39483b20b878d46144ab4cfe9a3e5600"
}

benchmark_setting {
  benchmark_id: "object_detection"
  accelerator: "ANE"
  accelerator_desc: "Apple Neural Engine"
  configuration: "Default"
  model_path: "https://github.com/mlcommons/mobile_models/raw/coreml/beta/CoreML/MobileDet.mlmodel"
  model_checksum: "ef849fbf2132e205158f05ca42db25f4"
}

benchmark_setting {
  benchmark_id: "natural_language_processing"
  accelerator: "ANE"
  accelerator_desc: "Apple Neural Engine"
  configuration: "Default"
  model_path: "https://github.com/mlcommons/mobile_models/raw/coreml/beta/CoreML/MobileBERT.mlmodel"
  model_checksum: "c7d544b5b3bd6cd9df7ebe8f04ecb7f9"
}

benchmark_setting {
  benchmark_id: "image_segmentation_v2"
  accelerator: "ANE"
  accelerator_desc: "Apple Neural Engine"
  configuration: "Default"
  model_path: "https://github.com/mlcommons/mobile_models/raw/coreml/beta/CoreML/Mosaic.mlmodel"
  model_checksum: "362d6b5bb1b8e10ae5b4e223f60d4d10"
}

)SETTINGS";

#endif
