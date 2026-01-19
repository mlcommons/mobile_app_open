#include "flutter/cpp/datasets/mmlu_gen.h"

#include <fstream>
#include <iostream>

#include "flutter/cpp/datasets/mmlu_utils/sentencepiece_utils.h"
#include "tensorflow/core/example/example.pb.h"
#include "tensorflow/core/example/feature_util.h"

namespace mlperf {
namespace mobile {

MmluGen::MmluGen(Backend* backend, const std::string& input_tfrecord,
                 const std::string& sp_path, bool zero_shot)
    : sample_reader_(input_tfrecord), Dataset(backend) {
  sp_processor = std::unique_ptr<sentencepiece::SentencePieceProcessor>(
      LoadSentencePieceProcessor(sp_path));

  // Load all TFRecord samples into memory
  // NOTE this can be moved to LoadSamplesToRam, but will cause delays between
  // queries due to IO reads happening between them
  for (size_t i = 0; i < sample_reader_.Size(); i++) {
    tensorflow::tstring record = sample_reader_.ReadRecord(i);
    tensorflow::Example example;
    example.ParseFromString(record);
    std::string input =
        tensorflow::GetFeatureValues<std::string>("input", example).Get(0);
    std::string answer =
        tensorflow::GetFeatureValues<std::string>("answer", example).Get(0);

    if (zero_shot) {
      // input-formatted shots are separated by 2 new lines, so we find the last
      // one which is the actual question
      std::string question_formatted = input.substr(input.rfind("\n\n") + 2);
      // input-formatted starts with a preface followed by 2 new lines, we want
      // that too.
      std::string preface = input.substr(0, input.find("\n\n") + 2);

      input = preface + "Question: " + question_formatted;
    }

    // std::string input_formatted = FormatLlamaUserPrompt(input, "Provide only
    // the answer letter, do not provide any explanation or preface.");

    std::vector<int> input_tokens;
    sp_processor->Encode(input.c_str(), &input_tokens).ok();

    // input token sanity check
    if (input_tokens.size() > input_token_limit_) {
      LOG(WARNING) << "Input token limit exceeded for entry "
                   << std::to_string(i) << ". Truncating.";
      input_tokens.erase(input_tokens.begin(), input_tokens.end() - input_token_limit_);
    }

    auto sample = std::make_unique<PromptSample>();
    sample->input = input;
    sample->input_tokens = input_tokens;
    sample->answer = answer;

    samples_.push_back(std::move(sample));
    sample_output_tokens_.push_back(std::vector<int>());
  }
}

void MmluGen::LoadSamplesToRam(const std::vector<QuerySampleIndex>& samples) {
  for (auto id : samples) {
    loaded_sample_ids_.insert(id);
  }
}

void MmluGen::UnloadSamplesFromRam(
    const std::vector<QuerySampleIndex>& samples) {
  for (auto id : samples) {
    loaded_sample_ids_.erase(id);
  }
}

std::vector<void*> MmluGen::GetData(int sample_idx) {
  std::vector<void*> data;

  if (sample_idx < samples_.size()) {
    data.push_back(reinterpret_cast<void*>(
        const_cast<std::vector<int>*>(&(samples_[sample_idx]->input_tokens))));
    data.push_back(reinterpret_cast<void*>(const_cast<int*>(&token_limit_)));
  }
  return data;
}

std::vector<uint8_t> MmluGen::ProcessOutput(const int sample_idx,
                                            const std::vector<void*>& outputs) {
  if (sample_idx >= samples_.size() || outputs.empty()) return {0};

  const auto& output_tokens =
      *(reinterpret_cast<std::vector<int>*>(outputs[0]));

  sample_output_tokens_[sample_idx] = output_tokens;
  used_sample_ids_.insert(sample_idx);

  return {1};
}

int64_t MmluGen::GetOutputTokenCount(const int sample_idx) {
  return sample_output_tokens_[sample_idx].size();
}

bool MmluGen::HasAccuracy() { return true; }

bool MmluGen::ComputeSampleAccuracy(const int sample_idx) {
  std::string prediction;
  sp_processor->Decode(sample_output_tokens_[sample_idx], &prediction).ok();

  char predicted_char = find_answer_char(prediction);
  const std::string& correct = samples_[sample_idx]->answer;

  return (predicted_char == correct[0]);
}

float MmluGen::ComputeAccuracy() {
  int total(0), correct(0);

  for (auto sample_id : used_sample_ids_) {
    total++;
    if (ComputeSampleAccuracy(sample_id)) correct++;
  }

  return total > 0 ? static_cast<float>(correct) / total : 0.0f;
}

std::string MmluGen::ComputeAccuracyString() {
  float acc = ComputeAccuracy();
  return "Accuracy: " + std::to_string(acc * 100.0f) + "%";
}

char MmluGen::find_answer_char(const std::string& input) {
  const unsigned char* c =
      reinterpret_cast<const unsigned char*>(input.c_str());

  while (*c) {
    // skip leading whitespace
    while (*c && std::isspace(*c)) ++c;
    if (!*c) break;

    const unsigned char* start = c;  // start of word

    // quick check: is the word exactly 1 char long?
    ++c;  // move to potential second char
    if (!*c || std::isspace(*c) || *c == '<') {
      if (*start == 'A' || *start == 'B' || *start == 'C' || *start == 'D' ||
          *start == 'a' || *start == 'b' || *start == 'c' || *start == 'd')
        return *start;
    } else {
      // skip rest of this (longer) word quickly
      while (*c && !std::isspace(*c)) ++c;
    }
  }
  return 0;
}

}  // namespace mobile
}  // namespace mlperf
