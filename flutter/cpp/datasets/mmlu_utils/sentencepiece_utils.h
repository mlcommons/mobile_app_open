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

inline static std::string FormatGemmaUserPrompt(
    std::string_view user_content, std::string_view system_content = "",
    bool add_generation_prompt = true, bool add_bos = true) {
  static constexpr const char* kBOS = "<bos>";
  static constexpr const char* kStartTurn = "<start_of_turn>";
  static constexpr const char* kEndTurn = "<end_of_turn>";

  std::string out;
  out.reserve(user_content.size() + 64);

  if (add_bos) out += kBOS;

  out += kStartTurn;
  out += "user";
  out += "\n";
  if (!system_content.empty()) {
    out.append(system_content);
    out += "\n\n";
  }
  out.append(user_content);
  out += kEndTurn;
  out += "\n";

  if (add_generation_prompt) {
    out += kStartTurn;
    out += "model";
    out += "\n";
  }
  return out;
}

inline static std::string FormatQwenUserPrompt(
    std::string_view user_content, std::string_view system_content = "",
    bool add_generation_prompt = true, bool add_empty_think = true) {
  static constexpr const char* kStartTurn = "<|im_start|>";
  static constexpr const char* kEndTurn = "<|im_end|>";

  std::string out;
  out.reserve(user_content.size() + 64);

  if (!system_content.empty()) {
    out += kStartTurn;
    out += "system\n";
    out.append(system_content);
    out += kEndTurn;
    out += "\n";
  }

  out += kStartTurn;
  out += "user\n";
  out.append(user_content);
  out += kEndTurn;
  out += "\n";

  if (add_generation_prompt) {
    out += kStartTurn;
    out += "assistant\n";
    if (add_empty_think) out += "<think>\n\n</think>\n\n";
  }
  return out;
}

inline static std::string FormatUserPrompt(
    std::string_view prompt_format, std::string_view user_content,
    std::string_view system_content = "") {
  if (prompt_format == "gemma") {
    return FormatGemmaUserPrompt(user_content, system_content);
  }
  if (prompt_format == "qwen") {
    return FormatQwenUserPrompt(user_content, system_content);
  }
  return FormatLlamaUserPrompt(user_content, system_content);
}

}  // namespace mobile
}  // namespace mlperf
#endif  // MLPERF_DATASETS_MMLU_UTILS_SENTENCEPIECE_UTILS_H_
