#include <google/protobuf/text_format.h>

#include <cstring>

#include "android/cpp/backends/external.h"
#include "android/cpp/proto/backend_setting.pb.h"
#include "main.h"

extern "C" struct dart_ffi_backend_match_result *dart_ffi_backend_match(
    const char *lib_path, const char *manufacturer, const char *model) {
  LOG(INFO) << "checking backend '" << lib_path << "'...";
  ::mlperf::mobile::BackendFunctions backend(lib_path);
  if (*lib_path != '\0' && !backend.isLoaded()) {
    LOG(INFO) << "backend '" << lib_path << "' can't be loaded";
    return nullptr;
  }

  mlperf_device_info_t device_info{model, manufacturer};

  // backends should set pbdata to a statically allocated string, so we don't
  // need to free it
  const char *pbdata = nullptr;

  auto result = new dart_ffi_backend_match_result;
  result->pbdata = nullptr;
  result->matches =
      backend.match(&result->error_message, &pbdata, &device_info);
  if (!result->matches) {
    return result;
  }

  if (result->error_message != nullptr) {
    result->matches = false;
    LOG(INFO)
        << "Backend generally matches but can't work on this specific device: "
        << result->error_message;
    return result;
  }

  LOG(INFO) << "checking pbdata";

  if (pbdata == nullptr || strlen(pbdata) == 0) {
    result->matches = false;
    LOG(INFO) << "Backend hasn't filled settings";
    return result;
  }

  ::mlperf::mobile::BackendSetting setting;
  if (!google::protobuf::TextFormat::ParseFromString(pbdata, &setting)) {
    result->matches = false;
    LOG(INFO) << "Can't parse backend settings before serialization:\n"
              << pbdata << std::endl;
    return result;
  }

  std::string res;
  if (!setting.SerializeToString(&res)) {
    result->matches = false;
    LOG(INFO) << "Can't serialize backend settings";
    return result;
  }

  result->pbdata_size = res.length();
  result->pbdata = new char[result->pbdata_size];
  memcpy(result->pbdata, res.data(), result->pbdata_size);

  LOG(INFO) << "backend '" << lib_path << "' matches";

  return result;
}

extern "C" void dart_ffi_backend_match_free(
    dart_ffi_backend_match_result *result) {
  (void)result->error_message;  // should be statically allocated
  delete result->pbdata;        // pbdata is allocated in dart_ffi_backend_match
  delete result;
}
