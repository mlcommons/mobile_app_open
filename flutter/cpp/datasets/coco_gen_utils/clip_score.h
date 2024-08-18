/* Copyright 2024 The MLPerf Authors. All Rights Reserved.

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

#ifndef MLPERF_DATASETS_COCO_GEN_UTILS_CLIP_SCORE_H
#define MLPERF_DATASETS_COCO_GEN_UTILS_CLIP_SCORE_H

#include <iostream>
#include <memory>
#include <vector>

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/model.h"

namespace mlperf {
namespace mobile {

class CLIPScorePredictor {
 public:
  explicit CLIPScorePredictor(const std::string& model_path);

  float predict(const std::vector<int32_t>& attention_mask,
                const std::vector<int32_t>& input_ids,
                const std::vector<float>& pixel_values);

 private:
  std::unique_ptr<tflite::FlatBufferModel> model;
  std::unique_ptr<tflite::Interpreter> interpreter;

  bool verifyInputSizes(int input_index, size_t input_size,
                        size_t element_size) const;
  bool extractOutput(int output_index, std::vector<float>& output_data) const;
};

}  // namespace mobile
}  // namespace mlperf

#endif  // MLPERF_DATASETS_COCO_GEN_UTILS_CLIP_SCORE_H
