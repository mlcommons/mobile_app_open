#include <google/protobuf/text_format.h>

#include <cstring>

#include "cpp/backends/external.h"
#include "cpp/proto/backend_setting.pb.h"
#include "main.h"

extern "C" struct dart_ffi_backend_match_result *dart_ffi_backend_match(
    const char *lib_path, const char *manufacturer, const char *model) {
  ::mlperf::mobile::BackendFunctions backend(lib_path);
  if (*lib_path != '\0' && !backend.isLoaded()) {
    std::cerr << "backend '" << lib_path << "' can't be loaded\n";
    return nullptr;
  }

  mlperf_device_info_t device_info{model, manufacturer};

  // backends should set pbdata to a statically allocated string, so we don't
  // need to free it
  const char *pbdata = nullptr;

  auto result = new dart_ffi_backend_match_result;
  result->matches =
      backend.match(&result->error_message, &pbdata, &device_info);

  if (pbdata == nullptr) {
    std::cout << "Backend haven't filled settings" << std::endl;
    return result;
  }

  ::mlperf::mobile::BackendSetting setting;
  if (!google::protobuf::TextFormat::ParseFromString(pbdata, &setting)) {
    std::cout << "Can't parse backend settings before serialization:\n"
              << pbdata << std::endl;
    return result;
  }

  std::string res;
  if (!setting.SerializeToString(&res)) {
    std::cout << "Can't serialize backend settings\n";
    return result;
  }

  result->pbdata_size = res.length();
  result->pbdata = new char[result->pbdata_size];
  memcpy(result->pbdata, res.data(), result->pbdata_size);

  return result;
}

extern "C" void dart_ffi_backend_match_free(
    dart_ffi_backend_match_result *result) {
  (void)result->error_message;  // should be statically allocated
  delete result->pbdata;        // pbdata is allocated in dart_ffi_backend_match
  delete result;
}
