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
#include "flutter/cpp/mlperf_driver.h"

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "flutter/cpp/backend.h"
#include "flutter/cpp/dataset.h"
#include "flutter/cpp/utils.h"
#include "loadgen/loadgen.h"
#include "loadgen/query_sample_library.h"
#include "loadgen/system_under_test.h"
#include "loadgen/test_settings.h"

namespace mlperf {
namespace mobile {

// A method to be called by the backend as soon as the first token is generated (only for token based benchmarks)
static void FirstTokenCallback(void* context) {
  auto ft_responses = *(reinterpret_cast<std::vector<::mlperf::QuerySampleResponse>*>(context));
  ::mlperf::FirstTokenComplete(ft_responses.data(), ft_responses.size());
}

void MlperfDriver::IssueQuery(
    const std::vector<::mlperf::QuerySample>& samples) {
  std::vector<::mlperf::QuerySampleResponse> responses;
  std::vector<::mlperf::QuerySampleResponse> ft_responses;
  std::vector<std::vector<uint8_t>> response_data;

  if (scenario_ == "Offline") {
    for (int idx = 0; idx < samples.size(); idx += batch_) {
      std::vector<::mlperf::QuerySample> sample;
      for (int b = 0; b < batch_; b++) {
        int sample_index =
            idx + b < samples.size()
                ? idx + b
                : samples.size() - 1;  // add extra data for filling batch
        sample.emplace_back(samples.at(sample_index));
        std::vector<void*> inputs = dataset_->GetData(sample.back().index);
        backend_->SetInputs(inputs, b);
      }

      // TODO maybe don't do these 2 lines for non token stuff
      // TODO figure out what this vector sample variable is
      ft_responses.clear();
      ft_responses.push_back({sample.back().index, reinterpret_cast<std::uintptr_t>(nullptr), 0});

      backend_->IssueQuery(&FirstTokenCallback, reinterpret_cast<void*>(&ft_responses));

      for (int b = 0; b < batch_; b++) {
        if (idx + b == samples.size()) break;  // ignore extra data
        // Report to mlperf.
        std::vector<void*> outputs = backend_->GetPredictedOutputs(b);
        response_data.push_back(
            dataset_->ProcessOutput(sample[b].index, outputs));
        responses.push_back(
            {sample[b].id,
             reinterpret_cast<std::uintptr_t>(response_data[idx + b].data()),
             response_data[idx + b].size()});
      }
      backend_->FlushQueries();
      query_counter_ += batch_;
    }
  } else {
    for (int idx = 0; idx < samples.size(); ++idx) {
      ::mlperf::QuerySample sample = samples.at(idx);
      std::vector<void*> inputs = dataset_->GetData(sample.index);
      backend_->SetInputs(inputs);

      // TODO maybe don't do these 2 lines for non token stuff
      ft_responses.clear();
      ft_responses.push_back({sample.id, reinterpret_cast<std::uintptr_t>(nullptr), 0});

      backend_->IssueQuery(&FirstTokenCallback, reinterpret_cast<void*>(&ft_responses));

      // Report to mlperf.
      std::vector<void*> outputs = backend_->GetPredictedOutputs();
      response_data.push_back(dataset_->ProcessOutput(sample.index, outputs));
      if (use_tokens_) {
        responses.push_back(
            {sample.id,
             reinterpret_cast<std::uintptr_t>(response_data[idx].data()),
             response_data[idx].size(),
             dataset_->GetOutputTokenCount(sample.index)});
      } else {
        responses.push_back(
            {sample.id,
             reinterpret_cast<std::uintptr_t>(response_data[idx].data()),
             response_data[idx].size()});
      }
      backend_->FlushQueries();
      query_counter_ += 1;
    }
  }
  ::mlperf::QuerySamplesComplete(responses.data(), responses.size());
}

void MlperfDriver::RunMLPerfTest(const std::string& mode, int min_query_count,
                                 double min_duration, double max_duration,
                                 int single_stream_expected_latency_ns,
                                 const std::string& output_dir,
                                 bool use_tokens) {
  ::mlperf::LogSettings log_settings;
  log_settings.log_output.outdir = output_dir;
  log_settings.log_output.copy_summary_to_stdout = true;

  ::mlperf::TestSettings mlperf_settings;
  // https://github.com/mlcommons/inference/blob/master/mlperf.conf
  mlperf_settings.qsl_rng_seed = 3066443479025735752UL;
  mlperf_settings.sample_index_rng_seed = 10688027786191513374UL;
  mlperf_settings.schedule_rng_seed = 14962580496156340209UL;

  // mlperf_settings.min_query_count = 1;
  // mlperf_settings.max_query_count = 2;
  // mlperf_settings.performance_sample_count_override = 5;
  use_tokens_ = use_tokens;
  mlperf_settings.use_token_latencies = use_tokens;
  // mlperf_settings.server_target_qps = 0.1;
  mlperf_settings.mode = Str2TestMode(mode);
  mlperf_settings.min_duration_ms =
      static_cast<uint64_t>(std::ceil(min_duration * 1000.0));
  // Note: max_duration_ms works only in SingleStream scenario.
  // See https://github.com/mlcommons/inference/issues/1397
  mlperf_settings.max_duration_ms =
      static_cast<uint64_t>(std::ceil(max_duration * 1000.0));
  mlperf_settings.enforce_max_duration = true;

  if (scenario_ == "Offline") {
    mlperf_settings.scenario = ::mlperf::TestScenario::Offline;
  } else {
    // Run MLPerf in SingleStream mode by default.
    mlperf_settings.scenario = ::mlperf::TestScenario::SingleStream;
    mlperf_settings.single_stream_expected_latency_ns =
        single_stream_expected_latency_ns;
  }

  ::mlperf::StartTest(this, dataset_.get(), mlperf_settings, log_settings);
}

}  // namespace mobile
}  // namespace mlperf
