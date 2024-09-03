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

#include "coco_gen.h"

#include <iomanip>

namespace mlperf {
namespace mobile {
namespace {}  // namespace

CocoGen::CocoGen(Backend* backend, const std::string& input_tfrecord,
                 const std::string& input_clip_model)
    : Dataset(backend),
      sample_reader_(input_tfrecord),
      samples_(sample_reader_.Size()),
      score_predictor_(input_clip_model) {
  if (input_format_.size() != 1 || output_format_.size() != 1) {
    LOG(FATAL) << "Coco_gen only supports 1 input and 1 output";
    return;
  }

  isModelFound = score_predictor_.getCanPredict();
}

void CocoGen::LoadSamplesToRam(const std::vector<QuerySampleIndex>& samples) {
  for (QuerySampleIndex sample_idx : samples) {
    tensorflow::tstring record = sample_reader_.ReadRecord(sample_idx);
    samples_.at(sample_idx) =
        std::move(std::make_unique<CaptionRecord>(CaptionRecord(record)));
  }
}

void CocoGen::UnloadSamplesFromRam(
    const std::vector<QuerySampleIndex>& samples) {
  for (QuerySampleIndex sample_idx : samples) {
    samples_.at(sample_idx).release();
  }
}

#define OUTPUT_SIZE 512 * 512 * 3
std::vector<uint8_t> CocoGen::ProcessOutput(const int sample_idx,
                                            const std::vector<void*>& outputs) {
  if (!isModelFound) return std::vector<uint8_t>();
  void* output = outputs.at(0);
  std::vector<uint8_t> output_pixels(OUTPUT_SIZE);
  if (output_format_[0].type == DataType::Uint8) {
    uint8_t* temp_data = reinterpret_cast<uint8_t*>(output);
    std::copy(temp_data, temp_data + output_pixels.size(),
              output_pixels.begin());
  } else if (output_format_[0].type == DataType::Float32) {
    float* temp_data = reinterpret_cast<float*>(output);

    // [-1.0, 1.0] -> [0, 255]
    for (int i = 0; i < OUTPUT_SIZE; i++) {
      output_pixels[i] = (uint8_t)((*(temp_data + i) + 1) / 2 * 255);
    }
  }
  if (!output_pixels.empty()) {
    sample_ids_.insert(sample_idx);
    CaptionRecord* record = samples_.at(sample_idx).get();
    LOG(INFO) << "caption: " << record->get_caption();
    caption_map[sample_idx] = record->get_caption();
    output_pixels_map[sample_idx] = output_pixels;
    attention_mask_map[sample_idx] = record->get_attention_mask_vector();
    input_ids_map[sample_idx] = record->get_input_ids_vector();

    return output_pixels;
  } else {
    return std::vector<uint8_t>();
  }
}

bool CocoGen::HasAccuracy() { return isModelFound; }

float CocoGen::ComputeAccuracy() {
  float total_score = 0.0f;
  float total_samples = static_cast<float>(sample_ids_.size());
  for (int sample_idx : sample_ids_) {
    std::string caption = caption_map[sample_idx];
    std::vector<int32_t> input_ids = input_ids_map[sample_idx];
    std::vector<int32_t> attention_mask = attention_mask_map[sample_idx];
    std::vector<uint8_t> output_pixels = output_pixels_map[sample_idx];
    std::vector<float> pixel_values(OUTPUT_SIZE);
    for (int i = 0; i < OUTPUT_SIZE; i++) {
      pixel_values[i] = static_cast<float>((output_pixels[i] / 128.0) - 1.0);
    }
    float score =
        score_predictor_.predict(attention_mask, input_ids, pixel_values);
    LOG(INFO) << "sample_idx: " << sample_idx << " caption: " << caption
              << " score: " << score;
    total_score += score;
  }
  float avg_score = total_score / total_samples;
  return avg_score;
}

std::string CocoGen::ComputeAccuracyString() {
  float result = ComputeAccuracy();
  if (result < 0.0f) {
    return {"N/A"};
  }
  std::stringstream stream;
  stream << std::fixed << std::setprecision(4) << result;
  return stream.str();
}

}  // namespace mobile
}  // namespace mlperf