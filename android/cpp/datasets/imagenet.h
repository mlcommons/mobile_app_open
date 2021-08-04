/* Copyright 2019-2021 The MLPerf Authors. All Rights Reserved.

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
#ifndef MLPERF_DATASETS_IMAGENET_H_
#define MLPERF_DATASETS_IMAGENET_H_

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "allocator.h"
#include "android/cpp/dataset.h"
#include "tensorflow/lite/tools/evaluation/stages/image_preprocessing_stage.h"

namespace mlperf {
namespace mobile {

// Implements the LSVRC2012 dataset for image classification.
class Imagenet : public Dataset {
 public:
  // Imagenet assumes that there is a single input resevered for the image data
  // and single output which contains the probabilities of every classes. The
  // order of images under image_dir should be the same as the original
  // LSVRC2012 dataset.
  Imagenet(Backend* backend, const std::string& image_dir,
           const std::string& groundtruth_file, int offset, int image_width,
           int image_height);

  // Returns the name of the dataset.
  const std::string& Name() const override { return name_; }

  // Total number of samples in library.
  size_t TotalSampleCount() override { return samples_.size(); }

  // Loads the requested query samples into memory.
  void LoadSamplesToRam(const std::vector<QuerySampleIndex>& samples) override;

  // Unloads the requested query samples from memory.
  void UnloadSamplesFromRam(
      const std::vector<QuerySampleIndex>& samples) override;

  // GetData returns the data of a specific input.
  std::vector<void*> GetData(int sample_idx) override {
    std::vector<void*> data;
    for (std::vector<uint8_t, BackendAllocator<uint8_t>>* v :
         samples_.at(sample_idx)) {
      data.push_back(v->data());
    }
    return data;
  }

  // ProcessOutput processes the output data before sending to mlperf.
  std::vector<uint8_t> ProcessOutput(
      const int sample_idx, const std::vector<void*>& outputs) override;

  // ComputeAccuracy Calculate the accuracy if the processed outputs.
  float ComputeAccuracy() override;

  // ComputeAccuracyString returns a string representing the accuracy.
  std::string ComputeAccuracyString() override;

 private:
  const std::string name_ = "Imagenet";
  // The ground truth file contains class indexes of each image.
  const std::string groundtruth_file_;

  // Offset is the index of the first non-background class. Some models with an
  // additional background class will have offset=1 when the background class
  // has index=0.
  int offset_;
  // List of the fullpath of images.
  std::vector<std::string> image_list_;
  // Loaded samples in RAM.
  std::vector<std::vector<std::vector<uint8_t, BackendAllocator<uint8_t>>*>>
      samples_;
  // Predictions from sample_idex to class_index (offset is subtracted).
  std::unordered_map<int32_t, int32_t> predictions_;
  // preprocessing_stage_ conducts preprocessing of images.
  std::unique_ptr<tflite::evaluation::ImagePreprocessingStage>
      preprocessing_stage_;

  // The width and height of the input images.
  int image_width_, image_height_;
};

}  // namespace mobile
}  // namespace mlperf
#endif  // MLPERF_DATASETS_IMAGENET_H_
