/* Copyright 2024 The MLPerf Authors. All Rights Reserved.

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
#ifndef MLPERF_DATASETS_COCO_GEN_H_
#define MLPERF_DATASETS_COCO_GEN_H_

#include <stddef.h>
#include <stdint.h>

#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "flutter/cpp/dataset.h"
#include "flutter/cpp/datasets/coco_gen_utils/types.h"
#include "flutter/cpp/datasets/squad_utils/tfrecord_reader.h"

namespace mlperf {
namespace mobile {

class CocoGen : public Dataset {
 public:
  // CocoGen need a TFRecord file
  CocoGen(Backend* backend, const std::string& input_tfrecord);

  // Returns the name of the dataset.
  const std::string& Name() override { return name_; }

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
    data.push_back(samples_.at(sample_idx)->get_tokenized_ids());
    return data;
  }

  // ProcessOutput processes the output data before sending to mlperf.
  std::vector<uint8_t> ProcessOutput(
      const int sample_idx, const std::vector<void*>& outputs) override;

  virtual bool HasAccuracy() override;

  // ComputeAccuracy Calculate the accuracy if the processed outputs.
  float ComputeAccuracy() override;

  // ComputeAccuracyString returns a string representing the accuracy.
  inline std::string ComputeAccuracyString() override;

 private:
  const std::string name_ = "CocoGen";
  // The random access reader to read input TFRecord file.
  TFRecordReader sample_reader_;

  // Loaded samples in RAM.
  std::vector<std::unique_ptr<CaptionRecord>> samples_;
  std::vector<uint8_t*> outputs_;
};

}  // namespace mobile
}  // namespace mlperf
#endif  // MLPERF_DATASETS_COCO_GEN_H_
