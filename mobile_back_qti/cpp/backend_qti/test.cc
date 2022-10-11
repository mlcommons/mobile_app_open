/* Copyright (c) 2020-2022 Qualcomm Innovation Center, Inc. All rights reserved.

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

#include "cpp/c/backend_c.h"
#include "cpp/c/type.h"
#include "tensorflow/lite/shared_library.h"

void *GetSymbol(void *handle, const char *name) {
  void *result;
  result = tflite::SharedLibrary::GetLibrarySymbol(handle, name);

  if (!result) {
    std::cout << "Unable to load symbol " << name << ": "
              << tflite::SharedLibrary::GetError();
  }
  return result;
}

bool (*match)(const char **, const char **,
              const mlperf_device_info_t *device_info) = nullptr;
mlperf_backend_ptr_t (*create)(const char *, mlperf_backend_configuration_t *,
                               const char *) = nullptr;

int main() {
  std::cout << "In main" << std::endl;
  void *handle = tflite::SharedLibrary::LoadLibrary("libqtibackend.so");

  if (handle == nullptr) {
    std::cout << "Failed to load QTI backend" << std::endl;
    return 1;
  }

  match = reinterpret_cast<decltype(match)>(
      GetSymbol(handle, "mlperf_backend_matches_hardware"));

  if (match == nullptr) {
    std::cout << "Failed to load match function" << std::endl;
    return 1;
  }
  const char *errmsg, *settings;
  mlperf_device_info_t device_info = {"QRD", "Qualcomm"};
  bool rv = match(&errmsg, &settings, &device_info);

  if (!rv) {
    std::cout << "HW doesn't match" << std::endl;
    return 1;
  }
#if 0
  create =
      reinterpret_cast<decltype(create)>(GetSymbol("mlperf_backend_create"));

  const SettingList& settings;
  mlperf_backend_configuration_t backend_config_ = CppToCSettings(settings);
  mlperf_backend_ptr_t backendptr = create("test1.tflite", &config, "./");
  int32_t input_count = get_input_count(backend_ptr);
  int32_t output_count = get_output_count(backend_ptr);
#endif
}
