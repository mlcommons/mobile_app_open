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
#include "flutter/cpp/datasets/coco.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "flutter/cpp/dataset.h"
#include "flutter/cpp/utils.h"
#include "src/google/protobuf/text_format.h"
#include "tensorflow/lite/tools/evaluation/proto/evaluation_stages.pb.h"
#include "tensorflow/lite/tools/evaluation/stages/image_preprocessing_stage.h"
#include "tensorflow/lite/tools/evaluation/stages/object_detection_average_precision_stage.h"
#include "tensorflow/lite/tools/evaluation/utils.h"

namespace mlperf {
namespace mobile {
namespace {
// TODO(b/145480762) Remove this code when preprocessing code is refactored.
inline TfLiteType DataType2TfType(DataType::Type type) {
  switch (type) {
    case DataType::Float32:
      return kTfLiteFloat32;
    case DataType::Uint8:
      return kTfLiteUInt8;
    case DataType::Int8:
      return kTfLiteInt8;
    case DataType::Float16:
      return kTfLiteFloat16;
    default:
      break;
  }
  return kTfLiteNoType;
}
}  // namespace

Coco::Coco(Backend *backend, const std::string &image_dir,
           const std::string &grouth_truth_file, int offset, int num_classes,
           int image_width, int image_height)
    : Dataset(backend),
      groundtruth_file_(grouth_truth_file),
      offset_(offset),
      num_classes_(num_classes),
      image_width_(image_width),
      image_height_(image_height) {
  if (input_format_.size() != 1 || output_format_.size() != 4) {
    LOG(FATAL) << "Coco only supports 1 input and 4 outputs";
    return;
  }

  // Finds all images under image_dir.
  std::unordered_set<std::string> exts{".rgb8", ".jpg", ".jpeg"};
  image_list_ = GetSortedFileNames(image_dir, exts);
  if (image_list_.empty()) {
    LOG(FATAL) << "Failed to list all the images file in provided path";
    return;
  }
  // Get filenames of the listed images. filenames are onverted to .jpg to match
  // filenames in the ground truth. They are used as keys to compare results.
  for (const auto &image_name : image_list_) {
    std::string filename = image_name.substr(image_name.find_last_of("/") + 1);
    filename.replace(filename.find_last_of("."), std::string::npos, ".jpg");
    name_list_.push_back(filename);
  }
  samples_ = std::vector<
      std::vector<std::vector<uint8_t, BackendAllocator<uint8_t>> *>>(
      image_list_.size());
  // Prepares the preprocessing stage.
  tflite::evaluation::ImagePreprocessingConfigBuilder builder(
      "image_preprocessing", DataType2TfType(input_format_.at(0).type));
  builder.AddResizingStep(image_width, image_height, false);
  builder.AddDefaultNormalizationStep();
  preprocessing_stage_.reset(
      new tflite::evaluation::ImagePreprocessingStage(builder.build()));
  if (preprocessing_stage_->Init() != kTfLiteOk) {
    LOG(FATAL) << "Failed to init preprocessing stage";
  }
}

void Coco::LoadSamplesToRam(const std::vector<QuerySampleIndex> &samples) {
  for (QuerySampleIndex sample_idx : samples) {
    // Preprocessing.
    if (sample_idx >= image_list_.size()) {
      LOG(FATAL) << "Sample index out of bound";
    }
    std::string filename = image_list_.at(sample_idx);
    preprocessing_stage_->SetImagePath(&filename);
    if (preprocessing_stage_->Run() != kTfLiteOk) {
      LOG(FATAL) << "Failed to run preprocessing stage";
    }

    // Move data out of preprocessing_stage_ so it can be reused.
    int total_byte = input_format_[0].size * GetByte(input_format_[0]);
    void *data_void = preprocessing_stage_->GetPreprocessedImageData();
    std::vector<uint8_t, BackendAllocator<uint8_t>> *data_uint8 =
        new std::vector<uint8_t, BackendAllocator<uint8_t>>(total_byte);
    std::copy(static_cast<uint8_t *>(data_void),
              static_cast<uint8_t *>(data_void) + total_byte,
              data_uint8->begin());

    // Allow backend to convert data layout if needed
    backend_->ConvertInputs(total_byte, image_width_, image_height_,
                            data_uint8->data());

    samples_.at(sample_idx).push_back(data_uint8);
  }
}

void Coco::UnloadSamplesFromRam(const std::vector<QuerySampleIndex> &samples) {
  for (QuerySampleIndex sample_idx : samples) {
    for (std::vector<uint8_t, BackendAllocator<uint8_t>> *v :
         samples_.at(sample_idx)) {
      delete v;
    }
    samples_.at(sample_idx).clear();
  }
}

std::vector<uint8_t> Coco::ProcessOutput(const int sample_idx,
                                         const std::vector<void *> &outputs) {
  int num_detections = static_cast<int>(*reinterpret_cast<float *>(outputs[3]));
  float *detected_label_boxes = reinterpret_cast<float *>(outputs[0]);
  float *detected_label_indices = reinterpret_cast<float *>(outputs[1]);
  float *detected_label_probabilities = reinterpret_cast<float *>(outputs[2]);

  std::vector<float> data;
  tflite::evaluation::ObjectDetectionResult predict_;
  for (int i = 0; i < num_detections; ++i) {
    const int bounding_box_offset = i * 4;
    // Add for reporting to mlperf log.
    data.push_back(static_cast<float>(sample_idx));                 // Image id
    data.push_back(detected_label_boxes[bounding_box_offset + 0]);  // ymin
    data.push_back(detected_label_boxes[bounding_box_offset + 1]);  // xmin
    data.push_back(detected_label_boxes[bounding_box_offset + 2]);  // ymax
    data.push_back(detected_label_boxes[bounding_box_offset + 3]);  // xmax
    data.push_back(detected_label_probabilities[i]);                // Score
    data.push_back(detected_label_indices[i] + offset_);            // Class
    // Add for evaluation inside this class.
    auto *object = predict_.add_objects();
    auto *bbox = object->mutable_bounding_box();
    bbox->set_normalized_top(detected_label_boxes[bounding_box_offset + 0]);
    bbox->set_normalized_left(detected_label_boxes[bounding_box_offset + 1]);
    bbox->set_normalized_bottom(detected_label_boxes[bounding_box_offset + 2]);
    bbox->set_normalized_right(detected_label_boxes[bounding_box_offset + 3]);
    object->set_class_id(static_cast<int>(detected_label_indices[i]) + offset_);
    object->set_score(detected_label_probabilities[i]);
  }
  predicted_objects_[name_list_[sample_idx]] = predict_;

  // Mlperf interpret data as uint8_t* and log it as a HEX string.
  std::vector<uint8_t> result(data.size() * 4);
  uint8_t *temp_data = reinterpret_cast<uint8_t *>(data.data());
  std::copy(temp_data, temp_data + result.size(), result.begin());
  return result;
}

float Coco::ComputeAccuracy() {
  // Reads the ground truth file.
  std::ifstream t(groundtruth_file_);
  std::string proto_str((std::istreambuf_iterator<char>(t)),
                        std::istreambuf_iterator<char>());
  tflite::evaluation::ObjectDetectionGroundTruth ground_truth_proto;
  google::protobuf::TextFormat::ParseFromString(proto_str, &ground_truth_proto);
  absl::flat_hash_map<std::string, tflite::evaluation::ObjectDetectionResult>
      groundtruth_objects;
  for (auto image_ground_truth : ground_truth_proto.detection_results()) {
    groundtruth_objects[image_ground_truth.image_name()] = image_ground_truth;
  }

  // Configs for ObjectDetectionAveragePrecisionStage.
  tflite::evaluation::EvaluationStageConfig eval_config;
  eval_config.set_name("average_precision");
  eval_config.mutable_specification()
      ->mutable_object_detection_average_precision_params()
      ->set_num_classes(num_classes_);
  tflite::evaluation::ObjectDetectionAveragePrecisionStage eval_stage_(
      eval_config);

  // Init and run.
  if (eval_stage_.Init() == kTfLiteError) {
    LOG(ERROR) << "Init evaluation stage failed";
    return -1.0f;
  }

  for (auto const &element : predicted_objects_) {
    eval_stage_.SetEvalInputs(element.second,
                              groundtruth_objects[element.first]);
    if (eval_stage_.Run() == kTfLiteError) {
      LOG(ERROR) << "Run evaluation stage failed";
      return -1.0f;
    }
  }

  // Read the result.
  auto metrics = eval_stage_.LatestMetrics()
                     .process_metrics()
                     .object_detection_average_precision_metrics();
  return metrics.overall_mean_average_precision();
}

std::string Coco::ComputeAccuracyString() {
  float result = ComputeAccuracy();
  if (result < 0.0f) {
    return std::string("N/A");
  }
  std::stringstream stream;
  stream << std::fixed << std::setprecision(4) << result << " mAP";
  return stream.str();
}

}  // namespace mobile
}  // namespace mlperf
