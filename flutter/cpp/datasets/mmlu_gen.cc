#include "flutter/cpp/datasets/mmlu_gen.h"

#include <fstream>
#include <iostream>

#include "flutter/cpp/datasets/mmlu_utils/sentencepiece_utils.h"
#include "tensorflow/core/example/example.pb.h"
#include "tensorflow/core/example/feature_util.h"

namespace mlperf {
namespace mobile {

// TODO add eos and bos tokens as config parameters
MmluGen::MmluGen(Backend* backend, const std::string& input_tfrecord, const std::string& sp_path, bool zero_shot)
    : sample_reader_(input_tfrecord), Dataset(backend) {

  sp_processor = std::unique_ptr<sentencepiece::SentencePieceProcessor>(LoadSentencePieceProcessor(sp_path));
  start_token_id = sp_processor->PieceToId(start_token);
  end_token_id = sp_processor->PieceToId(end_token);

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

    if (zero_shot) input = input.substr(input.rfind("\n\n")+2); // input-formatted shots are separated by 2 new lines

    std::vector<int> input_tokens;

    sp_processor->Encode(input.c_str(), &input_tokens).ok();

    input_tokens.insert(input_tokens.begin(), start_token_id);

    auto sample = std::make_unique<PromptSample>();
    sample->input = input;
    sample->input_tokens = input_tokens;
    sample->answer = answer;

    samples_.push_back(std::move(sample));
    sample_output_token_counts_.push_back(0);
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
    data.push_back(reinterpret_cast<void*>(const_cast<std::vector<int>*>(&(samples_[sample_idx]->input_tokens))));
    data.push_back(reinterpret_cast<void*>(const_cast<int*>(&end_token_id)));
  }
  return data;
}

std::vector<uint8_t> MmluGen::ProcessOutput(const int sample_idx,
                                            const std::vector<void*>& outputs) {
  if (sample_idx >= samples_.size() || outputs.empty()) return {0};

  const auto& output_tokens = *(reinterpret_cast<std::vector<int>*>(outputs[0]));
  LOG(INFO) << "~getout " << output_tokens.size() << std::endl;

  sample_output_token_counts_[sample_idx] = output_tokens.size();

  std::string prediction;
  sp_processor->Decode(output_tokens, &prediction).ok();

  char predicted_char = find_answer_char(prediction);
  const std::string& correct = samples_[sample_idx]->answer;

  bool is_correct = (predicted_char == correct[0]);

  total_++;
  if (is_correct) correct_++;

  return {static_cast<uint8_t>(is_correct)};
}

int64_t MmluGen::GetOutputTokenCount(const int sample_idx) {
  return sample_output_token_counts_[sample_idx];
}

bool MmluGen::HasAccuracy() { return true; }

float MmluGen::ComputeAccuracy() {
  return total_ > 0 ? static_cast<float>(correct_) / total_ : 0.0f;
}

std::string MmluGen::ComputeAccuracyString() {
  float acc = ComputeAccuracy();
  return "Accuracy: " + std::to_string(acc * 100.0f) + "%";
}

char MmluGen::find_answer_char(const std::string& input) {

  const unsigned char* c = reinterpret_cast<const unsigned char*>(input.c_str());

  while (*c) {
    // skip leading whitespace
    while (*c && std::isspace(*c)) ++c;
    if (!*c) break;

    const unsigned char* start = c; // start of word

    // quick check: is the word exactly 1 char long?
    ++c; // move to potential second char
    if (!*c || std::isspace(*c) || *c == '<') {
      if (*start == 'A' ||
          *start == 'B' ||
          *start == 'C' ||
          *start == 'D' ||
          *start == 'a' ||
          *start == 'b' ||
          *start == 'c' ||
          *start == 'd')
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
