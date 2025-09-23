#ifndef MLPERF_DATASETS_MMLU_GEN_H_
#define MLPERF_DATASETS_MMLU_GEN_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "flutter/cpp/dataset.h"
#include "flutter/cpp/datasets/squad_utils/tfrecord_reader.h"

namespace mlperf {
namespace mobile {

class MmluGen : public Dataset {
 public:
  MmluGen(Backend* backend, const std::string& input_tfrecord);

  const std::string& Name() override { return name_; }

  size_t TotalSampleCount() override { return samples_.size(); }

  size_t PerformanceSampleCount() override { return 1; }

  void LoadSamplesToRam(const std::vector<QuerySampleIndex>& samples) override;

  void UnloadSamplesFromRam(
      const std::vector<QuerySampleIndex>& samples) override;

  std::vector<void*> GetData(int sample_idx) override;

  std::vector<uint8_t> ProcessOutput(
      const int sample_idx, const std::vector<void*>& outputs) override;

  int64_t GetOutputTokenCount(const int sample_idx) override;

  bool HasAccuracy() override;

  float ComputeAccuracy() override;

  std::string ComputeAccuracyString() override;

 private:
  const std::string name_ = "MmluGen";

  TFRecordReader sample_reader_;

  struct PromptSample {
    std::string input;
    std::string answer;
  };

  std::vector<std::unique_ptr<PromptSample>> samples_;
  std::vector<int64_t> sample_output_token_counts_;
  std::set<int> loaded_sample_ids_;

  size_t correct_ = 0;
  size_t total_ = 0;
};

}  // namespace mobile
}  // namespace mlperf

#endif  // MLPERF_DATASETS_MMLU_GEN_H_
