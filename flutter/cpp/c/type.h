/* Copyright 2020 The MLPerf Authors. All Rights Reserved.

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
#ifndef MLPERF_C_TYPE_H_
#define MLPERF_C_TYPE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef void* mlperf_backend_ptr_t;

typedef enum {
  MLPERF_SUCCESS = 0,
  MLPERF_FAILURE = 1,
} mlperf_status_t;

// Requirements of the data for a specific input. It contains the type and size
// of that input.
typedef struct {
  enum Type {
    Float32 = 0,
    Uint8 = 1,
    Int8 = 2,
    Float16 = 3,
    Int32 = 4,
    Int64 = 5,
  };

  enum Type type;
  int64_t size;
} mlperf_data_t;

const int kMaxMLPerfBackendConfigs = 256;
typedef struct {
  const char* delegate_selected;
  const char* accelerator;
  const char* accelerator_desc;
  uint32_t batch_size;
  int count = 0;
  const char* keys[kMaxMLPerfBackendConfigs];
  const char* values[kMaxMLPerfBackendConfigs];
} mlperf_backend_configuration_t;

typedef struct {
  const char* model;
  const char* manufacturer;
} mlperf_device_info_t;

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // MLPERF_C_TYPE_H_
