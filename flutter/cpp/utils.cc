#include "flutter/cpp/utils.h"

#if defined(_WIN64) || defined(_WIN32)
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include "tensorflow/lite/tools/evaluation/utils.h"
#endif

namespace mlperf {
namespace mobile {

#if defined(_WIN64) || defined(_WIN32)
std::vector<std::string> GetSortedFileNames(
    const std::string &directory,
    const std::unordered_set<std::string> &extensions) {
  std::vector<std::string> result;
  for (const auto &entry : fs::directory_iterator(directory)) {
    if (!fs::is_regular_file(entry.path())) continue;
    if (!extensions.empty()) {
      std::string ext = entry.path().extension().string();
      std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
      if (extensions.count(ext) == 0) {
        continue;
      }
    }
    result.emplace_back(entry.path().string());
  }
  std::sort(result.begin(), result.end());
  return result;
}
#else
std::vector<std::string> GetSortedFileNames(
    const std::string &directory,
    const std::unordered_set<std::string> &extensions) {
  std::vector<std::string> result;
  TfLiteStatus ret = tflite::evaluation::GetSortedFileNames(
      tflite::evaluation::StripTrailingSlashes(directory), &result, extensions);
  if (ret == kTfLiteError) return {};
  return result;
}
#endif

// Get the number of bytes required for a type.
int GetByte(DataType type) {
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

// Convert string to mlperf::TestMode.
::mlperf::TestMode Str2TestMode(const std::string &mode) {
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

bool AddBackendConfiguration(mlperf_backend_configuration_t *configs,
                             const std::string &key, const std::string &value) {
  if (configs->count >= kMaxMLPerfBackendConfigs) {
    return false;
  }
  // Copy data in case of key, value deallocated.
  char *c_key = new char[key.length() + 1];
  strcpy(c_key, key.c_str());
  char *c_value = new char[value.length() + 1];
  strcpy(c_value, value.c_str());
  configs->keys[configs->count] = c_key;
  configs->values[configs->count] = c_value;
  configs->count++;
  return true;
}

void DeleteBackendConfiguration(mlperf_backend_configuration_t *configs) {
  delete configs->accelerator;
  delete configs->accelerator_desc;
  for (int i = 0; i < configs->count; ++i) {
    delete configs->keys[i];
    delete configs->values[i];
  }
  configs->count = 0;
}

mlperf_backend_configuration_t CppToCSettings(const SettingList &settings) {
  mlperf_backend_configuration_t c_settings;
  char *accelerator =
      new char[settings.benchmark_setting().accelerator().length() + 1];
  strcpy(accelerator, settings.benchmark_setting().accelerator().c_str());
  c_settings.accelerator = accelerator;
  c_settings.batch_size = settings.benchmark_setting().batch_size();
  char *accelerator_desc =
      new char[settings.benchmark_setting().accelerator_desc().length() + 1];
  strcpy(accelerator_desc, settings.benchmark_setting().accelerator_desc().c_str());
  c_settings.accelerator_desc = accelerator_desc;

  // Add common settings
  for (Setting s : settings.setting()) {
    AddBackendConfiguration(&c_settings, s.id(), s.value().value());
  }
  for (CustomSetting s : settings.benchmark_setting().custom_setting()) {
    AddBackendConfiguration(&c_settings, s.id(), s.value());
  }
  return c_settings;
}

SettingList createSettingList(const BackendSetting &backend_setting,
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
