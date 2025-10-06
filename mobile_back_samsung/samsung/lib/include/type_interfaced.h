/* Copyright 2023 The MLPerf Authors. All Rights Reserved.

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
#ifndef TYPE_INTERFACED_H_
#define TYPE_INTERFACED_H_

/**
 * @file type_interfaced.h
 * @brief converted mlperf type for samsung backend
 * @date 2023-07-10
 * @author Myungjong Kim (mj.kim010@samsung.com)
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef enum {
  INTF_MLPERF_SUCCESS = 0,
  INTF_MLPERF_FAILURE = 1,
} intf_mlperf_status_t;

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
} intf_mlperf_data_t;

const int kMaxMLPerfBackendConfigs_intf = 256;
typedef struct {
  const char* delegate_selected;
  const char* accelerator;
  const char* accelerator_desc;
  uint32_t batch_size;
  int count = 0;
  const char* keys[kMaxMLPerfBackendConfigs_intf];
  const char* values[kMaxMLPerfBackendConfigs_intf];
} intf_mlperf_backend_configuration_t;


typedef void (*ft_callback)(void* context);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // TYPE_INTERFACED_H_
