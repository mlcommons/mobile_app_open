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

#include "android/cpp/c/type.h"
#include "android/cpp/proto/backend_setting.pb.h"
#include "loadgen/test_settings.h"
#include "tensorflow/core/platform/logging.h"

namespace mlperf {
namespace mobile {

// Requirements of the data including multiple inputs.
using DataType = mlperf_data_t;
using DataFormat = std::vector<DataType>;

// Get the number of bytes required for a type.
inline int GetByte(DataType type) {
  switch (type.type) {
    case DataType::Uint8:
      return 1;
    case DataType::Int8:
      return 1;
    case DataType::Float16:
      return 2;
    case DataType::Int32:
    case DataType::Float32:
      return 4;
    case DataType::Int64:
      return 8;
  }
}

// Return topK indexes with highest probability.
template <typename T>
inline std::vector<int32_t> GetTopK(T* values, int num_elem, int k,
                                    int offset) {
  std::vector<int32_t> indices(num_elem - offset);
  std::iota(indices.begin(), indices.end(), 0);
  std::sort(indices.begin(), indices.end(), [&values, offset](int a, int b) {
    return values[a + offset] > values[b + offset];
  });
  indices.resize(k);
  return indices;
}

// Convert string to mlperf::TestMode.
inline ::mlperf::TestMode Str2TestMode(const std::string& mode) {
  if (mode == "PerformanceOnly") {
    return ::mlperf::TestMode::PerformanceOnly;
  } else if (mode == "AccuracyOnly") {
    return ::mlperf::TestMode::AccuracyOnly;
  } else if (mode == "SubmissionRun") {
    return ::mlperf::TestMode::SubmissionRun;
  } else {
    LOG(FATAL) << "Mode " << mode << " is not supported";
    return ::mlperf::TestMode::PerformanceOnly;
  }
}

inline bool AddBackendConfiguration(mlperf_backend_configuration_t* configs,
                                    const std::string& key,
                                    const std::string& value) {
  if (configs->count >= kMaxMLPerfBackendConfigs) {
    return false;
  }
  // Copy data in case of key, value deallocated.
  char* c_key = new char[key.length() + 1];
  strcpy(c_key, key.c_str());
  char* c_value = new char[value.length() + 1];
  strcpy(c_value, value.c_str());
  configs->keys[configs->count] = c_key;
  configs->values[configs->count] = c_value;
  configs->count++;
  return true;
}

inline void DeleteBackendConfiguration(
    mlperf_backend_configuration_t* configs) {
  delete configs->accelerator;
  for (int i = 0; i < configs->count; ++i) {
    delete configs->keys[i];
    delete configs->values[i];
  }
  configs->count = 0;
}

inline mlperf_backend_configuration_t CppToCSettings(
    const SettingList& settings) {
  mlperf_backend_configuration_t c_settings;
  char* accelerator =
      new char[settings.benchmark_setting().accelerator().length() + 1];
  strcpy(accelerator, settings.benchmark_setting().accelerator().c_str());
  c_settings.accelerator = accelerator;
  c_settings.batch_size = settings.benchmark_setting().batch_size();

  // Add common setings
  for (Setting s : settings.setting()) {
    AddBackendConfiguration(&c_settings, s.id(), s.value().value());
  }
  for (CustomSetting s : settings.benchmark_setting().custom_setting()) {
    AddBackendConfiguration(&c_settings, s.id(), s.value());
  }
  return c_settings;
}

inline SettingList createSettingList(const BackendSetting& backend_setting,
                                     std::string benchmark_id) {
  SettingList setting_list;
  int setting_index = 0;

  for (auto setting : backend_setting.common_setting()) {
    setting_list.add_setting();
    (*setting_list.mutable_setting(setting_index)) = setting;
    setting_index++;
  }

  // Copy the benchmark specific settings
  setting_index = 0;
  for (auto bm_setting : backend_setting.benchmark_setting()) {
    if (bm_setting.benchmark_id() == benchmark_id) {
      LOG(INFO) << "benchmark_setting:";
      LOG(INFO) << "  accelerator: " << bm_setting.accelerator();
      LOG(INFO) << "  configuration: " << bm_setting.configuration();
      for (auto custom_setting : bm_setting.custom_setting()) {
        LOG(INFO) << "  custom_setting: " << custom_setting.id() << " "
                  << custom_setting.value();
      }
      setting_list.mutable_benchmark_setting()->CopyFrom(bm_setting);
      LOG(INFO) << "SettingsList.benchmark_setting:";
      LOG(INFO) << "  accelerator: "
                << setting_list.benchmark_setting().accelerator();
      LOG(INFO) << "  configuration: "
                << setting_list.benchmark_setting().configuration();
      for (auto custom_setting :
           setting_list.benchmark_setting().custom_setting()) {
        LOG(INFO) << "  custom_setting: " << custom_setting.id() << " "
                  << custom_setting.value();
      }
    }
  }
  return setting_list;
}

}  // namespace mobile
}  // namespace mlperf

#endif  // MLPERF_UTILS_H_
