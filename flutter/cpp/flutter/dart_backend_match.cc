#include "./dart_backend_match.h"

#include <google/protobuf/text_format.h>

#include <cstring>

#include "flutter/cpp/backends/external.h"
#include "flutter/cpp/proto/backend_setting.pb.h"

extern "C" struct dart_ffi_backend_match_result *dart_ffi_backend_match(
    const char *lib_path, const char *manufacturer, const char *model) {
  if (strlen(lib_path) == 0)
    LOG(INFO) << "checking built-in backend...";
  else
    LOG(INFO) << "checking backend '" << lib_path << "' ...";
  ::mlperf::mobile::BackendFunctions backend(lib_path);
  if (*lib_path != '\0' && !backend.isLoaded()) {
    LOG(ERROR) << "backend can't be loaded";
    return nullptr;
  }

  mlperf_device_info_t device_info{model, manufacturer};

  // backends should allocate string values in static memory,
  // so we don't need to free it
  const char *pb_settings = nullptr;
  const char *error_message = nullptr;

  auto result = new dart_ffi_backend_match_result;
  result->pbdata = nullptr;
  result->error_message = nullptr;
  result->matches = backend.match(&error_message, &pb_settings, &device_info);
  if (!result->matches) {
    return result;
  }

  if (error_message != nullptr) {
    result->error_message = strdup(error_message);
  }

  if (result->error_message != nullptr) {
    result->matches = false;
    LOG(ERROR) << "Device is recognized but not supported: "
               << result->error_message;
    return result;
  }

  LOG(INFO) << "checking pbdata";

  if (pb_settings == nullptr || strlen(pb_settings) == 0) {
    result->matches = false;
    LOG(ERROR) << "Backend hasn't filled settings";
    return result;
  }

  ::mlperf::mobile::BackendSetting setting;
  if (!google::protobuf::TextFormat::ParseFromString(pb_settings, &setting)) {
    result->matches = false;
    LOG(ERROR) << "Can't parse backend settings before serialization:\n"
               << pb_settings << std::endl;
    return result;
  }

  std::string res;
  if (!setting.SerializeToString(&res)) {
    result->matches = false;
    LOG(ERROR) << "Can't serialize backend settings";
    return result;
  }

  result->pbdata_size = res.length();
  result->pbdata = new char[result->pbdata_size];
  memcpy(result->pbdata, res.data(), result->pbdata_size);

  LOG(INFO) << "backend matches";

  return result;
}

extern "C" void dart_ffi_backend_match_free(
    dart_ffi_backend_match_result *result) {
  free(result->error_message);
  delete result->pbdata;
  delete result;
}
