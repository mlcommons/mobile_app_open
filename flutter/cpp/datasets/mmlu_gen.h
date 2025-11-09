#ifndef MLPERF_DATASETS_MMLU_GEN_H_
#define MLPERF_DATASETS_MMLU_GEN_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include "flutter/cpp/dataset.h"
#include "flutter/cpp/datasets/squad_utils/tfrecord_reader.h"
#include "src/sentencepiece_processor.h"

namespace mlperf {
namespace mobile {

class MmluGen : public Dataset {
 public:
  MmluGen(Backend* backend, const std::string& input_tfrecord,
          const std::string& sp_path, bool zero_shot);

  const std::string& Name() override { return name_; }

  size_t TotalSampleCount() override { return samples_.size(); }

  size_t PerformanceSampleCount() override { return samples_.size(); }

  void LoadSamplesToRam(const std::vector<QuerySampleIndex>& samples) override;

  void UnloadSamplesFromRam(
      const std::vector<QuerySampleIndex>& samples) override;

  std::vector<void*> GetData(int sample_idx) override;

  std::vector<uint8_t> ProcessOutput(
      const int sample_idx, const std::vector<void*>& outputs) override;

  int64_t GetOutputTokenCount(const int sample_idx) override;

  bool HasAccuracy() override;

  bool ComputeSampleAccuracy(const int sample_idx);

  float ComputeAccuracy() override;

  std::string ComputeAccuracyString() override;

 private:
  const std::string name_ = "MmluGen";

  char find_answer_char(const std::string& input);

  TFRecordReader sample_reader_;

  struct PromptSample {
    std::string input;
    std::vector<int> input_tokens;
    std::string answer;
  };

  std::vector<std::unique_ptr<PromptSample>> samples_;
  std::vector<std::vector<int>> sample_output_tokens_;
  std::unordered_set<size_t> used_sample_ids_;
  std::set<int> loaded_sample_ids_;
  std::unique_ptr<sentencepiece::SentencePieceProcessor> sp_processor;
  static constexpr int token_limit_ = 4;
};

}  // namespace mobile
}  // namespace mlperf

#endif  // MLPERF_DATASETS_MMLU_GEN_H_
