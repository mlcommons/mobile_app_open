#include "coco_gen.h"

namespace mlperf {
namespace mobile {
namespace {}  // namespace

CocoGen::CocoGen(Backend* backend, const std::string& input_tfrecord)
    : Dataset(backend), sample_reader_(input_tfrecord) {}

void CocoGen::LoadSamplesToRam(const std::vector<QuerySampleIndex>& samples) {}

void CocoGen::UnloadSamplesFromRam(
    const std::vector<QuerySampleIndex>& samples) {}

std::vector<uint8_t> CocoGen::ProcessOutput(const int sample_idx,
                                            const std::vector<void*>& outputs) {
}

bool CocoGen::HasAccuracy() { return true; }

float CocoGen::ComputeAccuracy() { return 0.0; }

std::string CocoGen::ComputeAccuracyString() {}

}  // namespace mobile
}  // namespace mlperf
