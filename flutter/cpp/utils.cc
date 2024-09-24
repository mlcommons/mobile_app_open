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
  configs->keys[configs->count] = strdup(key.c_str());
  configs->values[configs->count] = strdup(value.c_str());
  configs->count++;
  return true;
}

void DeleteBackendConfiguration(mlperf_backend_configuration_t *configs) {
  delete configs->delegate_selected;
  delete configs->accelerator;
  delete configs->accelerator_desc;
  for (int i = 0; i < configs->count; ++i) {
    delete configs->keys[i];
    delete configs->values[i];
  }
  configs->count = 0;
}

mlperf_backend_configuration_t CppToCSettings(const SettingList &settings) {
  mlperf_backend_configuration_t c_settings = {};
  auto &bs = settings.benchmark_setting();

  for (const auto &d : bs.delegate_choice()) {
    if (d.delegate_name() == bs.delegate_selected()) {
      c_settings.delegate_selected = strdup(d.delegate_name().c_str());
      c_settings.accelerator = strdup(d.accelerator_name().c_str());
      c_settings.accelerator_desc = strdup(d.accelerator_desc().c_str());
      c_settings.batch_size = d.batch_size();
      for (const auto &s : d.custom_setting()) {
        AddBackendConfiguration(&c_settings, s.id(), s.value());
      }
      break;
    }
  }

  // Add common settings
  for (const auto &s : settings.setting()) {
    AddBackendConfiguration(&c_settings, s.id(), s.value().value());
  }

  for (const auto &s : bs.custom_setting()) {
    AddBackendConfiguration(&c_settings, s.id(), s.value());
  }

  return c_settings;
}

// Split the string by a given delimiter
std::vector<std::string> _splitString(const std::string &str, char delimiter) {
  std::vector<std::string> tokens;
  std::stringstream ss(str);
  std::string token;
  while (std::getline(ss, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

// Parse the key:value string list
std::unordered_map<std::string, std::string> _parseKeyValueList(
    const std::string &input) {
  std::unordered_map<std::string, std::string> keyValueMap;
  std::vector<std::string> pairs = _splitString(input, ',');  // Split by comma

  for (const std::string &pair : pairs) {
    std::vector<std::string> keyValue =
        _splitString(pair, ':');  // Split by colon
    if (keyValue.size() == 2) {
      keyValueMap[keyValue[0]] = keyValue[1];
    } else {
      LOG(ERROR) << "Invalid key:value pair: " << pair;
    }
  }
  return keyValueMap;
}

// Create the setting list for backend
SettingList CreateSettingList(const BackendSetting &backend_setting,
                              const std::string &custom_config,
                              const std::string &benchmark_id) {
  SettingList setting_list;
  int setting_index = 0;
  for (const auto &setting : backend_setting.common_setting()) {
    setting_list.add_setting();
    (*setting_list.mutable_setting(setting_index)) = setting;
    setting_index++;
  }

  // Copy the benchmark specific settings
  for (const auto &bm_setting : backend_setting.benchmark_setting()) {
    if (bm_setting.benchmark_id() == benchmark_id) {
      setting_list.mutable_benchmark_setting()->CopyFrom(bm_setting);

      auto parsed = _parseKeyValueList(custom_config);
      for (const auto &kv : parsed) {
        CustomSetting custom_setting = CustomSetting();
        custom_setting.set_id(kv.first);
        custom_setting.set_value(kv.second);
        setting_list.mutable_benchmark_setting()->mutable_custom_setting()->Add(
            std::move(custom_setting));
      }
      break;
    }
  }
  LOG(INFO) << "setting_list:" << std::endl << setting_list.DebugString();
  return setting_list;
}

}  // namespace mobile
}  // namespace mlperf
