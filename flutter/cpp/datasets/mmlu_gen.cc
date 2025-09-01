#include "flutter/cpp/datasets/mmlu_gen.h"
#include "tensorflow/core/example/example.pb.h"
#include "tensorflow/core/example/feature_util.h"

#include <fstream>
#include <iostream>

namespace mlperf {
namespace mobile {

MmluGen::MmluGen(Backend* backend, const std::string& input_tfrecord/*, const std::string& input_sppp*/)
    : sample_reader_(input_tfrecord), Dataset(backend) {
  std::cout << "MMLUT-DATASET: " << "Initializing with TFRecord " << input_tfrecord << " with sample size " << std::to_string(sample_reader_.Size()) << std::endl;
  // Load all TFRecord samples into memory
  //TODO move to MmluGen::LoadSamplesToRam?
  for (size_t i = 0; i < sample_reader_.Size(); i++) {
    tensorflow::tstring record = sample_reader_.ReadRecord(i);
    tensorflow::Example example;
    example.ParseFromString(record);
    std::string input = tensorflow::GetFeatureValues<std::string>("input", example).Get(0);
    std::string answer = tensorflow::GetFeatureValues<std::string>("answer", example).Get(0);

    auto sample = std::make_unique<PromptSample>();
    sample->input = input;
    sample->answer = answer;

    std::cout << "MMLUT-DATASET: " << "Loading TFRecord Data index " << std::to_string(i) << " with answer {" << answer << "}" << std::endl;

    samples_.push_back(std::move(sample));
    sample_output_token_counts_.push_back(0);
  }
  //LoadSentencePieceProcessor(input_sppp);
}

void MmluGen::LoadSamplesToRam(const std::vector<QuerySampleIndex>& samples) {
  std::cout << "MMLUT-DATASET: " << "Loading " << std::to_string(samples.size()) << " samples..." << std::endl;
  for (auto id : samples) {
    loaded_sample_ids_.insert(id);
  }
}

void MmluGen::UnloadSamplesFromRam(const std::vector<QuerySampleIndex>& samples) {
  for (auto id : samples) {
    loaded_sample_ids_.erase(id);
  }
}

std::vector<void*> MmluGen::GetData(int sample_idx) {
  std::cout << "MMLUT-DATASET: " << "Getting data at index " << std::to_string(sample_idx) << " (Answer is " << samples_[sample_idx]->answer << ")" << std::endl;
  std::vector<void*> data;
  if (sample_idx < samples_.size()) {
    data.push_back(reinterpret_cast<void*>(const_cast<char*>(samples_[sample_idx]->input.c_str())));
  }
  return data;
}

std::vector<uint8_t> MmluGen::ProcessOutput(const int sample_idx, const std::vector<void*>& outputs) {
  if (sample_idx >= samples_.size() || outputs.empty()) return {0};

  sample_output_token_counts_[sample_idx] = reinterpret_cast<std::vector<int>*>(outputs[1])->size();
  const char* prediction = reinterpret_cast<const char*>(outputs[0]);
  char predicted_char = prediction[1];  // Assume second token is the answer because of whitespace (e.g., 'A', 'B', ...)
  std::cout << "MMLUT-DATASET: " << "Predicted answer: " << predicted_char << std::endl;
  const std::string& correct = samples_[sample_idx]->answer;
  bool is_correct = (predicted_char == correct[0]);

  total_++;
  if (is_correct) correct_++;

  std::cout << "MMLUT-DATASET: " << "Accuracy: " << std::to_string(correct_) << "/" << std::to_string(total_) << std::endl;

  return {static_cast<uint8_t>(is_correct)};
}


int64_t MmluGen::GetOutputTokenCount(const int sample_idx) {
  return sample_output_token_counts_[sample_idx];
}

bool MmluGen::HasAccuracy() {
  return true;
}

float MmluGen::ComputeAccuracy() {
  return total_ > 0 ? static_cast<float>(correct_) / total_ : 0.0f;
}

std::string MmluGen::ComputeAccuracyString() {
  float acc = ComputeAccuracy();
  return "Accuracy: " + std::to_string(acc * 100.0f) + "%";
}

//void MmluGen::loadSentencePieceProcessor(std::string path) {
//  std::ifstream input(path, std::ios::binary);
//  std::string serialized_proto = std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
//  if(!sp_processor->LoadFromSerializedProto(serialized_proto).ok()) LOG(FATAL) << "Could not load SP Processor";
//}

}  // namespace mobile
}  // namespace mlperf
