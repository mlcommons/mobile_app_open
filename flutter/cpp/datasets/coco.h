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
#ifndef MLPERF_DATASETS_COCO_H_
#define MLPERF_DATASETS_COCO_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "allocator.h"
#include "android/cpp/dataset.h"
#include "tensorflow/lite/tools/evaluation/proto/evaluation_stages.pb.h"
#include "tensorflow/lite/tools/evaluation/stages/image_preprocessing_stage.h"

namespace mlperf {
namespace mobile {

// Implements the COCO 2017 dataset for object detection.
class Coco : public Dataset {
 public:
  // Coco assumes that there is a single input resevered for the image data
  // and 4 outputs which are bboxes, label indexes, probabilities, number of
  // detections respectively. In a bbox, the order of the data is xmin, ymin,
  // xmax, ymax.
  Coco(Backend* backend, const std::string& image_dir,
       const std::string& grouth_truth_file, int offset, int num_classes,
       int image_width, int image_height);

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
  const std::string name_ = "Coco";
  // The ground truth file contains bboxes.
  const std::string groundtruth_file_;
  // The offset of the first class. Some models with an additional null class
  // have offset=1 when the null class has index=0.
  int offset_;
  // Number of classes in the output of the model.
  int num_classes_;
  // List of the fullpath of images under image_dir.
  std::vector<std::string> image_list_;
  // List of the name of images under image_dir.
  std::vector<std::string> name_list_;
  // Loaded samples in RAM.
  std::vector<std::vector<std::vector<uint8_t, BackendAllocator<uint8_t>>*>>
      samples_;
  // Groundtruth and predicted results.
  absl::flat_hash_map<std::string, tflite::evaluation::ObjectDetectionResult>
      predicted_objects_;
  // preprocessing_stage_ conducts preprocessing of images.
  std::unique_ptr<tflite::evaluation::ImagePreprocessingStage>
      preprocessing_stage_;

  // The width and height of the input images.
  int image_width_, image_height_;
};

}  // namespace mobile
}  // namespace mlperf
#endif  // MLPERF_DATASETS_COCO_H_
