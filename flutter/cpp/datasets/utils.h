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
#ifndef MLPERF_DATASETS_UTILS_H_
#define MLPERF_DATASETS_UTILS_H_

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

}  // namespace mobile
}  // namespace mlperf
#endif  // MLPERF_DATASETS_UTILS_H_
