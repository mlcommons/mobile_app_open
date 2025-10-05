/* Copyright 2025 The MLPerf Authors. All Rights Reserved.

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

#ifndef MLPERF_DATASETS_MMLU_UTILS_SENTENCEPIECE_UTILS_H_
#define MLPERF_DATASETS_MMLU_UTILS_SENTENCEPIECE_UTILS_H_

#include <fstream>

#include "src/sentencepiece_processor.h"

namespace mlperf {
namespace mobile {

static sentencepiece::SentencePieceProcessor *LoadSentencePieceProcessor(
                      std::string path) {
  std::ifstream input(path, std::ios::binary);
  std::string serialized_proto = std::string(
      std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
  auto processor = new sentencepiece::SentencePieceProcessor();
  processor->LoadFromSerializedProto(serialized_proto).ok();
  return processor;
}

}  // namespace mobile
}  // namespace mlperf
#endif  // MLPERF_DATASETS_MMLU_UTILS_SENTENCEPIECE_UTILS_H_
