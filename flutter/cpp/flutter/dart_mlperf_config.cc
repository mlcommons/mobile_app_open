#include <google/protobuf/text_format.h>

#include <cstring>

#include "cpp/backends/external.h"
#include "cpp/proto/mlperf_task.pb.h"
#include "main.h"

// The main purpose of this function
// is to allow Dart to read config from tasks.pbtxt file.
// We can't read it directly in Dart
// because Dart doesn't have any equivalent of protobuf::TextFormat yet.
// Check this issue to see if this has been fixed:
//   https://github.com/google/protobuf.dart/issues/125
extern "C" dart_ffi_mlperf_config_result *dart_ffi_mlperf_config(
    const char *pb_content) {
  if (pb_content == nullptr) {
    return nullptr;
  }

  ::mlperf::mobile::MLPerfConfig config;
  if (!google::protobuf::TextFormat::ParseFromString(pb_content, &config)) {
    std::cout << "Can't parse config before serialization:\n"
              << pb_content << std::endl;
    return nullptr;
  }

  std::string res;
  if (!config.SerializeToString(&res)) {
    std::cout << "Can't serialize config\n";
    return nullptr;
  }

  auto data = new dart_ffi_mlperf_config_result;
  data->size = res.length();
  data->data = new char[data->size];
  memcpy(data->data, res.data(), data->size);

  return data;
}

extern "C" void dart_ffi_mlperf_config_free(
    dart_ffi_mlperf_config_result *result) {
  delete result->data;
  delete result;
}
