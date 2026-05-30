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
#include <string>

#include "src/sentencepiece_processor.h"

namespace mlperf {
namespace mobile {

static sentencepiece::SentencePieceProcessor* LoadSentencePieceProcessor(
    std::string path) {
  std::ifstream input(path, std::ios::binary);
  std::string serialized_proto = std::string(
      std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
  auto processor = new sentencepiece::SentencePieceProcessor();
  processor->LoadFromSerializedProto(serialized_proto).ok();
  return processor;
}

inline static std::string FormatLlamaUserPrompt(
    std::string_view user_content, std::string_view system_content = "",
    bool add_generation_prompt = true, bool add_bos = true) {
  static constexpr const char* kBOS = "<|begin_of_text|>";
  static constexpr const char* kStartHeader = "<|start_header_id|>";
  static constexpr const char* kEndHeader = "<|end_header_id|>";
  static constexpr const char* kEOT = "<|eot_id|>";

  std::string out;
  out.reserve(user_content.size() + 64);

  if (add_bos) out += kBOS;

  if (!system_content.empty()) {
    out += kStartHeader;
    out += "system";
    out += kEndHeader;
    out += "\n";
    out.append(system_content);
    out += "\n";
    out += kEOT;
  }

  out += kStartHeader;
  out += "user";
  out += kEndHeader;
  out += "\n";
  out.append(user_content);
  out += "\n";
  out += kEOT;

  if (add_generation_prompt) {
    out += kStartHeader;
    out += "assistant";
    out += kEndHeader;
    out += "\n";
  }
  return out;
}

}  // namespace mobile
}  // namespace mlperf
#endif  // MLPERF_DATASETS_MMLU_UTILS_SENTENCEPIECE_UTILS_H_
