/* Copyright 2019 The MLPerf Authors. All Rights Reserved.

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
#ifndef MLPERF_UTILS_H_
#define MLPERF_UTILS_H_

#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

#include "flutter/cpp/c/type.h"
#include "flutter/cpp/proto/backend_setting.pb.h"
#include "loadgen/test_settings.h"
#include "tensorflow/core/platform/logging.h"

namespace mlperf {
namespace mobile {

// equivalent of tflite::evaluation::GetSortedFileNames.
// Original function is absent on Windows.
// When building not for Windows this function
// is a bridge to original function
std::vector<std::string> GetSortedFileNames(
    const std::string &directory,
    const std::unordered_set<std::string> &extensions);

// Requirements of the data including multiple inputs.
using DataType = mlperf_data_t;
using DataFormat = std::vector<DataType>;

// Get the number of bytes required for a type.
int GetByte(DataType type);

// Return topK indexes with highest probability.
template <typename T>
std::vector<int32_t> GetTopK(T *values, int num_elem, int k, int offset) {
  std::vector<int32_t> indices(num_elem - offset);
  std::iota(indices.begin(), indices.end(), 0);
  std::sort(indices.begin(), indices.end(), [&values, offset](int a, int b) {
    return values[a + offset] > values[b + offset];
  });
  indices.resize(k);
  return indices;
}

// Convert string to mlperf::TestMode.
::mlperf::TestMode Str2TestMode(const std::string &mode);

bool AddBackendConfiguration(mlperf_backend_configuration_t *configs,
                             const std::string &key, const std::string &value);

void DeleteBackendConfiguration(mlperf_backend_configuration_t *configs);

mlperf_backend_configuration_t CppToCSettings(const SettingList &settings);

SettingList createSettingList(const BackendSetting &backend_setting,
                              std::string benchmark_id);

}  // namespace mobile
}  // namespace mlperf

#endif  // MLPERF_UTILS_H_
