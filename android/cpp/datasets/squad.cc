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
#include "android/cpp/datasets/squad.h"

#include <cstdint>
#include <cstring>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <fstream>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "android/cpp/datasets/squad_utils/common.h"
#include "android/cpp/utils.h"
#include "tensorflow/core/platform/file_system.h"

namespace mlperf {
namespace mobile {
namespace {
// Number of candidates for the answers.
int K = 20;
// Max length of the answer.
int kMaxAnswerLength = 30;

// PrelimPrediction is the candidate for the final prediction.
struct PrelimPrediction {
  int sample_index;
  int start_index;
  int end_index;
  float score;
  PrelimPrediction(int sample_idx, int start_idx, int end_idx, float value)
      : sample_index(sample_idx),
        start_index(start_idx),
        end_index(end_idx),
        score(value) {}
};
}  // namespace

Squad::Squad(Backend* backend, const std::string& input_tfrecord,
             const std::string& gt_tfrecord)
    : Dataset(backend),
      sample_reader_(input_tfrecord),
      samples_(sample_reader_.Size()),
      predictions_(sample_reader_.Size()) {

    if (std::ifstream(gt_tfrecord).good()) {
      gt_reader_ = std::make_unique<TFRecordReader>(gt_tfrecord);
    } else {
      LOG(ERROR) << "Could not read the ground truth file";
    };

  // Check input and output formats.
  if (input_format_.size() != 3 || output_format_.size() != 2) {
    LOG(FATAL) << "MobileBert only supports 3 inputs and 2 outputs";
  }

  if (input_format_[0].type != input_format_[1].type &&
      input_format_[1].type != input_format_[2].type) {
    LOG(FATAL) << "All inputs should either be Int32 or Float32";
  }
  if (input_format_[0].type != DataType::Int32 &&
      input_format_[0].type != DataType::Float32) {
    LOG(FATAL)
        << "Input type other than int32 and Float32 are not supported yet";
  }
  if (output_format_[0].type != DataType::Float32 ||
      output_format_[1].type != DataType::Float32) {
    LOG(FATAL) << "Output type other than Float32 is not supported yet";
  }

  // Map questions to their list of input samples.
  for (uint32_t idx = 0; idx < sample_reader_.Size(); ++idx) {
    SampleRecord<int32_t> sample(sample_reader_.ReadRecord(idx));
    qas_id_to_samples_[sample.qas_id_].push_back(idx);
  }

  // Map the question id to its ground truth data.
  if (gt_reader_ != nullptr) {
    for (uint32_t idx = 0; idx < gt_reader_->Size(); ++idx) {
      GroundTruthRecord record(gt_reader_->ReadRecord(idx));
      qas_id_to_ground_truth_[record.qas_id] = idx;
    }
  }
}

void Squad::LoadSamplesToRam(const std::vector<QuerySampleIndex>& samples) {
  // using int64 = google::protobuf::int64;
  for (QuerySampleIndex sample_idx : samples) {
    tensorflow::tstring record = sample_reader_.ReadRecord(sample_idx);
    samples_.at(sample_idx)
        .reset(SampleRecordFactory::create(record, input_format_[0].type));
  }
}

void Squad::UnloadSamplesFromRam(const std::vector<QuerySampleIndex>& samples) {
  for (QuerySampleIndex sample_idx : samples) {
    samples_.at(sample_idx).release();
  }
}

std::vector<uint8_t> Squad::ProcessOutput(const int sample_idx,
                                          const std::vector<void*>& outputs) {
  float* start_logit = reinterpret_cast<float*>(outputs[1]);
  uint32_t start_logit_size = output_format_[1].size;
  float* end_logit = reinterpret_cast<float*>(outputs[0]);
  uint32_t end_logit_size = output_format_[0].size;

  predictions_.at(sample_idx)
      .reset(new MobileBertPrediction(start_logit, start_logit_size, end_logit,
                                      end_logit_size));
  return predictions_.at(sample_idx)->GetData();
}

float Squad::ComputeAccuracy() {
  float final_score = 0.0f;
  if (gt_reader_ == nullptr) return final_score;
  for (auto& it : qas_id_to_samples_) {
    const std::string& qas_id = it.first;
    // Find candidates for the best prediction.
    PrelimPrediction best_pred(0, 0, 0, -std::numeric_limits<float>::max());
    for (uint32_t sample_index : it.second) {
      SampleRecord<int32_t> sample(sample_reader_.ReadRecord(sample_index));
      // Get top start and end indexes based on their logit.
      std::vector<int32_t> top_start_indexes =
          GetTopK(predictions_[sample_index]->start_logit_.data(),
                  predictions_[sample_index]->start_logit_.size(), K, 0);
      std::vector<int32_t> top_end_indexes =
          GetTopK(predictions_[sample_index]->end_logit_.data(),
                  predictions_[sample_index]->end_logit_.size(), K, 0);

      for (int32_t start_index : top_start_indexes) {
        for (int32_t end_index : top_end_indexes) {
          // Ignore all couple of invalid indexes.
          if (end_index < start_index) continue;
          if (end_index - start_index + 1 > kMaxAnswerLength) continue;
          if (start_index >= sample.span_tokens_.size() ||
              end_index >= sample.span_tokens_.size())
            continue;

          // Ignore couples contain query tokens.
          if (start_index < sample.query_tokens_length_ ||
              end_index < sample.query_tokens_length_)
            continue;

          // Only keep the couple with max context.
          if (!sample.token_is_max_context_[start_index -
                                            sample.query_tokens_length_])
            continue;

          // Store the valid candidate.
          float score = predictions_[sample_index]->start_logit_[start_index] +
                        predictions_[sample_index]->end_logit_[end_index];
          if (score > best_pred.score) {
            best_pred =
                PrelimPrediction(sample_index, start_index, end_index, score);
          }
        }
      }
    }

    // Get the text from span tokens.
    SampleRecord<int32_t> sample(
        sample_reader_.ReadRecord(best_pred.sample_index));
    std::string pred_tokens = sample.span_tokens_[best_pred.start_index];
    for (int i = best_pred.start_index + 1; i <= best_pred.end_index; ++i) {
      absl::StrAppend(&pred_tokens, " ", sample.span_tokens_[i]);
    }
    // De-tokenize WordPieces that have been split off.
    pred_tokens = absl::StrReplaceAll(pred_tokens, {{" ##", ""}, {"##", ""}});
    // Clean whitespace.
    absl::RemoveExtraAsciiWhitespace(&pred_tokens);

    // Get the text from original tokens.
    int doc_start = sample.token_index_map_[best_pred.start_index -
                                            sample.query_tokens_length_];
    int doc_end = sample.token_index_map_[best_pred.end_index -
                                          sample.query_tokens_length_];
    GroundTruthRecord gt_record(
        gt_reader_->ReadRecord(qas_id_to_ground_truth_[qas_id]));
    if (gt_record.tokens.size() <= doc_start ||
        gt_record.words.size() <= doc_start)
      continue;
    std::string orig_tokens = gt_record.tokens[doc_start];
    std::string orig_words = gt_record.words[doc_start];
    for (int i = doc_start + 1; i <= doc_end; ++i) {
      if (gt_record.tokens.size() <= i || gt_record.words.size() <= i) continue;
      absl::StrAppend(&orig_tokens, " ", gt_record.tokens[i]);
      absl::StrAppend(&orig_words, " ", gt_record.words[i]);
    }

    // Get the final answer.
    std::string final_text =
        get_final_text(pred_tokens, orig_tokens, orig_words);
    final_score += F1Score(gt_record.answers, final_text);
  }
  return final_score / qas_id_to_samples_.size();
}

std::string Squad::ComputeAccuracyString() {
  float result = ComputeAccuracy();
  if (result == 0.0f) {
    return std::string("N/A");
  }
  std::stringstream stream;
  stream << std::fixed << std::setprecision(4) << result * 100;
  return stream.str();
}
}  // namespace mobile
}  // namespace mlperf
