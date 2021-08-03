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
#include <iostream>
#include <list>
#include <string>

#include "android/cpp/c/type.h"
#include "android/cpp/proto/mlperf_task.pb.h"
#include "android/cpp/utils.h"
#include "google/protobuf/text_format.h"

namespace mlperf {
namespace mobile {
namespace {
const std::string test_settings = R"SETTINGS(
common_setting {
  id: "num_threads"
  name: "Number of threads"
  value {
    value: "4"
    name: "4 threads"
  }
  acceptable_value {
    value: "1"
    name: "Single thread"
  }
  acceptable_value {
    value: "2"
    name: "2 threads"
  }
  acceptable_value {
    value: "4"
    name: "4 threads"
  }
  acceptable_value {
    value: "8"
    name: "8 threads"
  }
  acceptable_value {
    value: "16"
    name: "16 threads"
  }
}

common_setting {
  id: "configuration"
  name: "Configuration"
  value {
    value: "Test Configuration"
    name: "Default"
  }
}

common_setting {
  id: "share_results"
  name: "Share results"
  value {
    value: "0"
    name: "false"
  }
  acceptable_value {
    value: "1"
    name: "true"
  }
  acceptable_value {
    value: "0"
    name: "false"
  }
}

common_setting {
  id: "cooldown"
  name: "Cooldown"
  value {
    value: "0"
    name: "false"
  }
  acceptable_value {
    value: "1"
    name: "true"
  }
  acceptable_value {
    value: "0"
    name: "false"
  }
}

benchmark_setting {
  benchmark_id: "IC_tpu_uint8"
  accelerator: "nnapi"
  accelerator_desc: "NNAPI"
  configuration: "TFLite"
  src: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilenet_edgetpu_224_1.0_uint8.tflite"
  custom_setting {
    id: "bgLoad2"
    value: "true"
  }
}

benchmark_setting {
  benchmark_id: "IC_tpu_uint8_offline"
  accelerator: "nnapi"
  accelerator_desc: "NNAPI"
  configuration: "TFLite"
  batch_size: 2
  src: "https://github.com/mlcommons/mobile_models/raw/main/v0_7/tflite/mobilenet_edgetpu_224_1.0_uint8.tflite"
  custom_setting {
    id: "bgLoad2"
    value: "true"
  }
})SETTINGS";

void dumpCSettings(mlperf_backend_configuration_t c_settings) {
  std::cout << "  accelerator: " << std::string(c_settings.accelerator)
            << std::endl;
  std::cout << "  batch_size: " << c_settings.batch_size << std::endl;
  std::cout << "  count: " << c_settings.count << std::endl;
  for (auto idx = 0; idx < c_settings.count; idx++) {
    std::cout << "  setting[" << c_settings.keys[idx]
              << "] = " << std::string(c_settings.values[idx]) << std::endl;
  }
}

void dumpCustomSetting(const mlperf::mobile::CustomSetting &custom_setting) {
  std::cout << "      ID: " << custom_setting.id() << std::endl;
  std::cout << "      Value: " << custom_setting.value() << std::endl;
}

void dumpSetting(const mlperf::mobile::Setting &setting) {
  std::cout << "    ID: " << setting.id() << std::endl;
  std::cout << "    Name: " << setting.name() << std::endl;
  std::cout << "    Value: " << std::endl;
  std::cout << "      value: " << setting.value().value() << std::endl;
  std::cout << "      name: " << setting.value().name() << std::endl;
}

void dumpBenchmarkSetting(const BenchmarkSetting &benchmark_setting) {
  std::cout << "    benchmark_id: " << benchmark_setting.benchmark_id()
            << std::endl;
  std::cout << "    accelerator: " << benchmark_setting.accelerator()
            << std::endl;
  std::cout << "    accelerator_desc: " << benchmark_setting.accelerator_desc()
            << std::endl;
  std::cout << "    configuration: " << benchmark_setting.configuration()
            << std::endl;
  std::cout << "    batch_size: " << benchmark_setting.batch_size()
            << std::endl;
  std::cout << "    src: " << benchmark_setting.src() << std::endl;
  std::cout << "    Custom Settings: " << std::endl;
  for (auto s : benchmark_setting.custom_setting()) {
    dumpCustomSetting(s);
  }
}

void dumpSettingList(SettingList &setting_list) {
  std::cout << "  Common Settings:" << std::endl;
  for (auto setting : setting_list.setting()) {
    dumpSetting(setting);
  }
  std::cout << "  Benchmark Setting:" << std::endl;
  dumpBenchmarkSetting(setting_list.benchmark_setting());
}

// test serialization of the schema
int test_proto() {
  // Load the text
  BackendSetting backend_setting;
  if (!google::protobuf::TextFormat::ParseFromString(test_settings,
                                                     &backend_setting)) {
    printf("Failed to parse the proto file. Please check its format.\n");
    return 1;
  }

  // Simulate the code flow
  std::list<std::string> benchmarks;
  benchmarks.push_back("IC_tpu_uint8");
  benchmarks.push_back("IC_tpu_uint8_offline");
  for (auto benchmark_id : benchmarks) {
    // Convert to SettingList
    SettingList setting_list = createSettingList(backend_setting, benchmark_id);

    std::cout << "SettingList for " << benchmark_id << ":\n";
    dumpSettingList(setting_list);
    std::cout << std::endl;

    // Run CppToCSettings
    std::cout << "c_settings for " << benchmark_id << ":\n";
    mlperf_backend_configuration_t c_settings = CppToCSettings(setting_list);
    dumpCSettings(c_settings);
  }
  // Dump values
  return 0;
}
}  // namespace
}  // namespace mobile
}  // namespace mlperf

int main() { return mlperf::mobile::test_proto(); }
