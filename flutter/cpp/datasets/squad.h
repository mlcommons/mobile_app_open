/* Copyright 2020 The MLPerf Authors. All Rights Reserved.

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
#ifndef MLPERF_DATASETS_SQUAD_H_
#define MLPERF_DATASETS_SQUAD_H_

#include <stddef.h>
#include <stdint.h>

#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "flutter/cpp/dataset.h"
#include "flutter/cpp/datasets/squad_utils/tfrecord_reader.h"
#include "flutter/cpp/datasets/squad_utils/types.h"

namespace mlperf {
namespace mobile {

// Implements the SQuAD 1.1 dataset for language understanding task.
class Squad : public Dataset {
 public:
  // SQuAD need two TFRecord files, one for inference samples, one for ground
  // truth answers.
  Squad(Backend* backend, const std::string& input_tfrecord,
        const std::string& gt_tfrecord);

  // Returns the name of the dataset.
  const std::string& Name() const override { return name_; }

  // Total number of samples in library.
  size_t TotalSampleCount() override { return samples_.size(); }

  // Loads the requested query samples into memory.
  inline void LoadSamplesToRam(
      const std::vector<QuerySampleIndex>& samples) override;

  // Unloads the requested query samples from memory.
  inline void UnloadSamplesFromRam(
      const std::vector<QuerySampleIndex>& samples) override;

  // GetData returns the data of a specific input.
  std::vector<void*> GetData(int sample_idx) override {
    std::vector<void*> data;
    data.push_back(samples_.at(sample_idx)->get_input_ids());
    data.push_back(samples_.at(sample_idx)->get_input_mask());
    data.push_back(samples_.at(sample_idx)->get_segment_ids());
    return data;
  }

  // ProcessOutput processes the output data before sending to mlperf.
  std::vector<uint8_t> ProcessOutput(
      const int sample_idx, const std::vector<void*>& outputs) override;

  // ComputeAccuracy Calculate the accuracy if the processed outputs.
  float ComputeAccuracy() override;

  // ComputeAccuracyString returns a string representing the accuracy.
  inline std::string ComputeAccuracyString() override;

 private:
  const std::string name_ = "SQuAD 1.1";
  // The random access reader to read input TFRecord file.
  TFRecordReader sample_reader_;
  // The random access reader to read ground truth TFRecord file.
  std::unique_ptr<TFRecordReader> gt_reader_;
  // Loaded samples in RAM.
  std::vector<std::unique_ptr<ISampleRecord>> samples_;
  // Store predictions to compute accuracy.
  std::vector<std::unique_ptr<MobileBertPrediction>> predictions_;
  // Store the list of samples related to a question.
  std::unordered_map<std::string, std::vector<uint32_t>> qas_id_to_samples_;
  // Store the list of samples related to a question.
  std::unordered_map<std::string, uint32_t> qas_id_to_ground_truth_;
};

}  // namespace mobile
}  // namespace mlperf
#endif  // MLPERF_DATASETS_SQUAD_H_
