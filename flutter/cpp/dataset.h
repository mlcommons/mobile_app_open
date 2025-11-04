/* Copyright 2019 The MLPerf Authors. All Rights Reserved.

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
#ifndef MLPERF_DATASET_H_
#define MLPERF_DATASET_H_

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "flutter/cpp/backend.h"
#include "flutter/cpp/utils.h"
#include "loadgen/query_sample_library.h"

namespace mlperf {
namespace mobile {

// Dataset is an interface adapting different datasets for MLPerfDriver. Each
// dataset should implement their methods to load, pre-process input data and
// post-process the output data.
class Dataset : public ::mlperf::QuerySampleLibrary {
 public:
  Dataset(Backend* backend)
      : backend_(backend),
        input_format_(backend->GetInputFormat()),
        output_format_(backend->GetOutputFormat()),
        backend_name_(backend->Name()) {}

  ~Dataset() override {}

  // The number of samples that are guaranteed to fit in RAM.
  size_t PerformanceSampleCount() override {
    int sample_size = 0;
    for (const DataType& data_type : input_format_) {
      sample_size += data_type.size * GetByte(data_type);
    }
    // Since it runs on mobile device, use 500MB at most.
    return 500 * 1e6 / sample_size;
  }

  // GetData returns the data of a specific input.
  virtual std::vector<void*> GetData(int sample_idx) = 0;

  // ProcessOutput processes the output data before sending to mlperf.
  // The argument sample_idx should only be used to help the
  // Dataset calculating the accuracy; it should not affect the returned
  // result by any means.
  virtual std::vector<uint8_t> ProcessOutput(
      const int sample_idx, const std::vector<void*>& outputs) = 0;

  // Should be called after ProcessOutput.
  virtual int64_t GetOutputTokenCount(const int sample_idx) { return 0; }

  virtual bool HasAccuracy() { return false; }

  // ComputeAccuracy calculates the accuracy of the processed outputs. This
  // function is optional, you don't need to implement it if you want to use
  // other scripts for accuracy calculation.
  virtual float ComputeAccuracy() { return -1.0f; }

  // ComputeAccuracyString is same as ComputeAccuracy but returns a string so
  // different metrics may have different formats.
  virtual std::string ComputeAccuracyString() { return std::string("N/A"); }

 protected:
  const DataFormat input_format_;
  const DataFormat output_format_;
  const std::string backend_name_;
  Backend* backend_;
};

}  // namespace mobile
}  // namespace mlperf
#endif  // MLPERF_DATASET_H_
