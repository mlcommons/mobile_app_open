#ifndef MLPERF_DATASETS_IFEVAL_H_
#define MLPERF_DATASETS_IFEVAL_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "flutter/cpp/dataset.h"
#include "flutter/cpp/datasets/ifeval_utils/types.h"
#include "flutter/cpp/datasets/squad_utils/tfrecord_reader.h"
#include "src/sentencepiece_processor.h"
#include "tensorflow/core/example/example.pb.h"

namespace mlperf {
namespace mobile {
namespace ifeval {
struct GroupAccuracy {
  size_t change_case_correct = 0, combination_correct = 0,
         detectable_content_correct = 0, detectable_format_correct = 0,
         keywords_correct = 0, language_correct = 0,
         length_constraints_correct = 0, punctuation_correct = 0,
         startend_correct = 0;
  size_t change_case_total = 0, combination_total = 0,
         detectable_content_total = 0, detectable_format_total = 0,
         keywords_total = 0, language_total = 0, length_constraints_total = 0,
         punctuation_total = 0, startend_total = 0;
};
}  // namespace ifeval
class IFEval : public Dataset {
 public:
  IFEval(Backend* backend, const std::string& input_tfrecord,
         const std::string& sp_path, bool loose_follow);

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

  bool ComputeSampleAccuracy(const int sample_idx,
                             ifeval::GroupAccuracy& accuracy);

  float ComputeAccuracy() override;

  std::string ComputeAccuracyString() override;

  inline std::vector<std::unique_ptr<ifeval::Instruction>> BuildInstructions(
      const tensorflow::Example& ex);

  inline void ProcessResult(ifeval::InstructionGroup group, bool is_correct,
                            ifeval::GroupAccuracy& accuracy);

 private:
  const std::string name_ = "IFEval";

  TFRecordReader sample_reader_;

  std::vector<std::unique_ptr<ifeval::Sample>> samples_;
  std::vector<std::vector<int>> sample_output_tokens_;
  std::unordered_set<size_t> used_sample_ids_;
  std::set<int> loaded_sample_ids_;
  std::unique_ptr<sentencepiece::SentencePieceProcessor> sp_processor;

  bool loose_follow_;
};

}  // namespace mobile
}  // namespace mlperf

#endif  // MLPERF_DATASETS_IFEVAL_H_
