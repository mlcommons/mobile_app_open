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
#ifndef MLPERF_DATASETS_COCO_GEN_UTILS_TYPES_H_
#define MLPERF_DATASETS_COCO_GEN_UTILS_TYPES_H_

#include "tensorflow/core/example/example.pb.h"
#include "tensorflow/core/example/feature_util.h"
#include "tensorflow/core/platform/types.h"

namespace mlperf {
namespace mobile {

// CaptionRecord is equivlent to records in the ground truth tfrecord file.
struct CaptionRecord {
  explicit CaptionRecord(const tensorflow::tstring& record) {
    using string = std::string;
    tensorflow::Example example;
    example.ParseFromString(record);

    id = tensorflow::GetFeatureValues<int64_t>("id", example)[0];
    // std::cout << "id: " << id << "\n";

    auto caption_list = tensorflow::GetFeatureValues<string>("caption", example);
    caption = std::vector<std::string>(caption_list.begin(), caption_list.end());

    auto tokenized_id_list = tensorflow::GetFeatureValues<int64_t>("tokenized_ids", example);
    tokenized_ids = std::vector<int64_t>(tokenized_id_list.begin(), tokenized_id_list.end());

    auto filename_list = tensorflow::GetFeatureValues<string>("file_name", example);
    filename = std::vector<std::string>(filename_list.begin(), filename_list.end());

    auto clip_score_list = tensorflow::GetFeatureValues<float>("clip_score", example);
    clip_score = std::vector<float>(clip_score_list.begin(), clip_score_list.end())[0];
  }

  void dump() {
    std::cout << "  id: " << id << "\n";
    std::cout << "  caption: " << caption[0] << "\n";
    std::cout << "  token_id: ";
    for (auto t: tokenized_ids) {
            std::cout << t << ", ";
    }
    std::cout << "\n";
    std::cout << "  file_name: " << filename[0] << "\n";
    std::cout << "  clip_score: " << clip_score << "\n";
  }

  int64_t id;
  std::vector<std::string> caption;
  std::vector<int64_t> tokenized_ids;
  std::vector<std::string> filename;
  float clip_score;
};

}
}

#endif  // MLPERF_DATASETS_COCO_GEN_UTILS_TYPES_H_
