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

#include "flutter/cpp/datasets/coco_gen_utils/clip_score.h"

#include "flutter/cpp/utils.h"
#include "tensorflow/lite/c/c_api.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"

namespace mlperf {
namespace mobile {

CLIPScorePredictor::CLIPScorePredictor(const std::string& model_path) {
  if (model_path == "") {
    canPredict = false;
    return;
  } else
    canPredict = true;

  // Load the model
  model = tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
  if (!model) {
    LOG(FATAL) << "Failed to load TFLite model from path: " << model_path;
  }

  // Create the interpreter
  tflite::ops::builtin::BuiltinOpResolver resolver;
  tflite::InterpreterBuilder(*model, resolver)(&interpreter);
  if (!interpreter) {
    LOG(FATAL) << "Failed to create TFLite interpreter";
  }

  // Allocate tensor buffers.
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    LOG(FATAL) << "Failed to allocate tensors for the interpreter";
  }
}

bool CLIPScorePredictor::getCanPredict() { return canPredict; }

float CLIPScorePredictor::predict(const std::vector<int32_t>& attention_mask,
                                  const std::vector<int32_t>& input_ids,
                                  const std::vector<float>& pixel_values) {
  constexpr int kInputIndexMask = 0;
  constexpr int kInputIndexIds = 1;
  constexpr int kInputIndexPixels = 2;
  constexpr int kOutputIndexLogitsPerImage = 1;
  constexpr int kOutputIndexLogitsPerText = 2;

  // Ensure the input tensor dimensions match
  if (!verifyInputSizes(kInputIndexMask, attention_mask.size(),
                        sizeof(int32_t)) ||
      !verifyInputSizes(kInputIndexIds, input_ids.size(), sizeof(int32_t)) ||
      !verifyInputSizes(kInputIndexPixels, pixel_values.size(),
                        sizeof(float))) {
    LOG(FATAL) << "Input tensor sizes do not match the expected dimensions";
    return -1.0f;
  }

  // Copy inputs to input tensors
  std::memcpy(
      interpreter->typed_tensor<int32_t>(interpreter->inputs()[kInputIndexIds]),
      input_ids.data(), input_ids.size() * sizeof(int32_t));
  std::memcpy(interpreter->typed_tensor<int32_t>(
                  interpreter->inputs()[kInputIndexMask]),
              attention_mask.data(), attention_mask.size() * sizeof(int32_t));
  std::memcpy(interpreter->typed_tensor<float>(
                  interpreter->inputs()[kInputIndexPixels]),
              pixel_values.data(), pixel_values.size() * sizeof(float));

  // Run inference
  if (interpreter->Invoke() != kTfLiteOk) {
    LOG(FATAL) << "Failed to invoke TFLite interpreter";
    return -1.0f;
  }

  // Extract outputs
  std::vector<float> logits_per_text;
  std::vector<float> logits_per_image;
  if (!extractOutput(kOutputIndexLogitsPerImage, logits_per_image) ||
      !extractOutput(kOutputIndexLogitsPerText, logits_per_text)) {
    LOG(FATAL) << "Failed to extract output tensors";
    return -1.0f;
  }

  assert(logits_per_text[0] == logits_per_image[0]);
  float score = logits_per_text[0];
  return score;
}

bool CLIPScorePredictor::verifyInputSizes(int input_index, size_t input_size,
                                          size_t element_size) const {
  TfLiteTensor* input_tensor =
      interpreter->tensor(interpreter->inputs()[input_index]);
  if (input_tensor->bytes != input_size * element_size) {
    auto total_size = input_size * element_size;
    LOG(ERROR) << "Input tensor at index " << input_index << " has size "
               << total_size << " (=" << input_size << "*" << element_size
               << ") but expected size is " << input_tensor->bytes;
    return false;
  }
  return true;
}

bool CLIPScorePredictor::extractOutput(int output_index,
                                       std::vector<float>& output_data) const {
  TfLiteTensor* output_tensor =
      interpreter->tensor(interpreter->outputs()[output_index]);
  if (output_tensor == nullptr) {
    LOG(ERROR) << "Failed to get output tensor at index " << output_index;
    return false;
  }

  size_t output_size = output_tensor->bytes / sizeof(float);
  if (output_size == 0) {
    LOG(ERROR) << "Output tensor at index " << output_index << " has zero size";
    return false;
  }

  output_data.resize(output_size);
  std::memcpy(
      output_data.data(),
      interpreter->typed_tensor<float>(interpreter->outputs()[output_index]),
      output_size * sizeof(float));

  return true;
}

}  // namespace mobile
}  // namespace mlperf
