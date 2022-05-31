/* Copyright 2020-2021 The MLPerf Authors. All Rights Reserved.
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
#include "flutter/cpp/datasets/imagepairs.h"

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <streambuf>
#include <string>
#include <unordered_set>

#include "flutter/cpp/utils.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/tools/evaluation/proto/evaluation_stages.pb.h"
#include "tensorflow/lite/tools/evaluation/stages/image_preprocessing_stage.h"
#include "tensorflow/lite/tools/evaluation/utils.h"

namespace mlperf {
namespace mobile {
namespace {
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

IMAGEPAIRS::IMAGEPAIRS(Backend *backend, const std::string &image_dir,
                       const std::string &ground_truth_dir, int num_channels,
                       int scale, int image_width, int image_height)
    : Dataset(backend),
      num_channels_(num_channels),
      scale_(scale),
      image_width_(image_width),
      image_height_(image_height) {
  if (input_format_.size() != 1 || output_format_.size() != 1) {
    LOG(FATAL) << "IMAGEPAIRS model only supports 1 input and 1 output";
    return;
  }

  // Finds all images under image_dir.
  std::unordered_set<std::string> exts{".rgb8", ".jpg", ".jpeg"};
  image_list_ = GetSortedFileNames(image_dir, exts);
  if (image_list_.empty()) {
    LOG(FATAL) << "Failed to list all the image files in provided path";
    return;
  }

  samples_ = std::vector<
      std::vector<std::vector<uint8_t, BackendAllocator<uint8_t>> *>>(
      image_list_.size());
  // Finds all ground truth files under ground_truth_dir.

  std::unordered_set<std::string> gt_exts{".raw", ".jpg", ".jpeg"};
  ground_truth_list_ = GetSortedFileNames(ground_truth_dir, gt_exts);
  if (ground_truth_list_.empty()) {
    LOG(ERROR) << "Failed to list all the ground truth files in provided path. "
                  "Only measuring performance.";
  }

  // Prepares the preprocessing stage.
  tflite::evaluation::ImagePreprocessingConfigBuilder builder(
      "image_preprocessing", DataType2TfType(input_format_.at(0).type));

  preprocessing_stage_.reset(
      new tflite::evaluation::ImagePreprocessingStage(builder.build()));
  if (preprocessing_stage_->Init() != kTfLiteOk) {
    LOG(FATAL) << "Failed to init preprocessing stage";
  }

  counted_ = std::vector<bool>(image_list_.size(), false);
  psnr_ = 0;
}

void IMAGEPAIRS::LoadSamplesToRam(
    const std::vector<QuerySampleIndex> &samples) {
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

void IMAGEPAIRS::UnloadSamplesFromRam(
    const std::vector<QuerySampleIndex> &samples) {
  for (QuerySampleIndex sample_idx : samples) {
    for (std::vector<uint8_t, BackendAllocator<uint8_t>> *v :
         samples_.at(sample_idx)) {
      delete v;
    }
    samples_.at(sample_idx).clear();
  }
}

std::vector<uint8_t> IMAGEPAIRS::ProcessOutput(
    const int sample_idx, const std::vector<void *> &outputs) {
  if (ground_truth_list_.empty()) {
    return std::vector<uint8_t>();
  }

  if (!counted_[sample_idx]) {
    std::string filename = ground_truth_list_.at(sample_idx);

    preprocessing_stage_->SetImagePath(&filename);
    if (preprocessing_stage_->Run() != kTfLiteOk) {
      LOG(FATAL) << "Failed to run preprocessing stage";
    }

    // Move data out of preprocessing_stage_ so it can be reused.
    int total_byte = output_format_[0].size * GetByte(output_format_[0]);

    void *data_void = preprocessing_stage_->GetPreprocessedImageData();
    std::vector<uint8_t, BackendAllocator<uint8_t>> *data_uint8 =
        new std::vector<uint8_t, BackendAllocator<uint8_t>>(total_byte);

    std::copy(static_cast<uint8_t *>(data_void),
              static_cast<uint8_t *>(data_void) + total_byte,
              data_uint8->begin());

    // Allow backend to convert data layout if needed
    backend_->ConvertInputs(total_byte, image_width_, image_height_,
                            data_uint8->data());

    // LOG(INFO) << "GT size : " << data_uint8->size();
    auto ground_truth_vector = data_uint8->data();  // auto is GOD

    // old image reading
    // std::ifstream stream(filename, std::ios::in | std::ios::binary);
    // // LOG(INFO) << "stream : " << stream;
    // std::vector<uint8_t> ground_truth_vector(
    //     (std::istreambuf_iterator<char>(stream)),
    //     std::istreambuf_iterator<char>());

    float *outputFloat = reinterpret_cast<float *>(outputs[0]);
    int32_t *outputInt32 = reinterpret_cast<int32_t *>(outputs[0]);
    int8_t *outputInt8 = reinterpret_cast<int8_t *>(outputs[0]);
    uint8_t *outputUint8 = reinterpret_cast<uint8_t *>(outputs[0]);

    bool isOutputFloat = (output_format_.at(0).type == DataType::Float32);
    bool isOutputInt32 = (output_format_.at(0).type == DataType::Int32);
    bool isOutputInt8 = (output_format_.at(0).type == DataType::Int8);
    bool isOutputUint8 = (output_format_.at(0).type == DataType::Uint8);

    // LOG(INFO) << "Output is .. Float? : " << isOutputFloat;
    // LOG(INFO) << "Output is .. Int32? : " << isOutputInt32;
    // LOG(INFO) << "Output is .. Int8? : " << isOutputInt8;
    // LOG(INFO) << "Output is .. Uint8? : " << isOutputUint8;

    // psnr calculating code
    int n_pixels =
        image_width_ * image_height_ * num_channels_ * scale_ * scale_;
    float mse = 0;
    for (int i = 0; i < n_pixels; i++) {
      uint8_t p;
      if (isOutputFloat) {
        p = (uint8_t)(0x000000ff & (int32_t)outputFloat[i]);
      } else if (isOutputUint8) {
        p = outputUint8[i];
      } else {
        p = (uint8_t)(0x000000ff & outputInt8[i]);
      }
      mse += (float(ground_truth_vector[i]) - float(p)) *
             (float(ground_truth_vector[i]) - float(p));
    }
    mse = mse / n_pixels;
    float sample_psnr_ = -10 * log10(mse / (255.0 * 255.0));
    LOG(INFO) << "[" << filename << "] psnr : " << sample_psnr_;

    psnr_ += sample_psnr_;

    counted_[sample_idx] = true;
  }

  return std::vector<uint8_t>();
}

float IMAGEPAIRS::ComputeAccuracy() {
  if (ground_truth_list_.empty()) {
    return 0.0;
  }

  return psnr_ / (float)ground_truth_list_.size();
}

std::string IMAGEPAIRS::ComputeAccuracyString() {
  float result = ComputeAccuracy();
  if (result == 0.0f) {
    return std::string("N/A");
  }
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << result << " dB";

  return stream.str();
}

}  // namespace mobile
}  // namespace mlperf
