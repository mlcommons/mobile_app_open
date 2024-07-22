#include "coco_gen.h"

namespace mlperf {
namespace mobile {
namespace {}  // namespace

CocoGen::CocoGen(Backend* backend, const std::string& input_tfrecord)
    : Dataset(backend),
      sample_reader_(input_tfrecord),
      samples_(sample_reader_.Size()) {}

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

std::vector<uint8_t> CocoGen::ProcessOutput(const int sample_idx,
                                            const std::vector<void*>& outputs) {
  void* output = outputs.at(0);
  if (output_format_[0].type == DataType::Uint8) {
    std::vector<uint8_t> output_pixels(512 * 512 * 3);
    uint8_t* temp_data = reinterpret_cast<uint8_t*>(output);
    std::copy(temp_data, temp_data + output_pixels.size(),
              output_pixels.begin());
    return output_pixels;
  }
}

bool CocoGen::HasAccuracy() { return true; }

float CocoGen::ComputeAccuracy() { return 0.0; }

std::string CocoGen::ComputeAccuracyString() {}

}  // namespace mobile
}  // namespace mlperf
