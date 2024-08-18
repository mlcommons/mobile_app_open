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
      score_predictor_(input_clip_model) {}

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
    output_pixels_map[sample_idx] = output_pixels;
    attention_mask_map[sample_idx] = record->get_attention_mask_vector();
    input_ids_map[sample_idx] = record->get_input_ids_vector();
    return output_pixels;
  } else {
    return std::vector<uint8_t>();
  }
}

bool CocoGen::HasAccuracy() { return !sample_ids_.empty(); }

float CocoGen::ComputeAccuracy() {
  float total_score = 0.0f;
  float total_samples = static_cast<float>(sample_ids_.size());
  for (int sample_idx : sample_ids_) {
    std::vector<int32_t> input_ids = input_ids_map[sample_idx];
    std::vector<int32_t> attention_mask = attention_mask_map[sample_idx];
    std::vector<uint8_t> output_pixels = output_pixels_map[sample_idx];
    std::vector<float> pixel_values(OUTPUT_SIZE);
    for (int i = 0; i < OUTPUT_SIZE; i++) {
      pixel_values[i] = static_cast<float>(output_pixels[i]);
    }
    float score =
        score_predictor_.predict(attention_mask, input_ids, pixel_values);
    LOG(INFO) << "sample_idx: " << sample_idx << " score: " << score;
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