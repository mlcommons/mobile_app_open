/* Copyright 2020-2023 Samsung Electronics Co. LTD  All Rights Reserved.

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
#include <android/log.h>
#include <array>
#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>

#include "flutter/cpp/c/backend_c.h"
#include "flutter/cpp/c/type.h"
#include "type_interfaced.h"
#include "mbe_utils.hpp"
#include "mbe_core_holder.hpp"

using namespace mbe;

static mbe_core_holder *mbe_core = nullptr;

int convert_backend_configuration(mlperf_backend_configuration_t *in_config, intf_mlperf_backend_configuration_t *out_config) {
  if (!in_config || !out_config) {
    return -1;
  }
  out_config->delegate_selected = in_config->delegate_selected;
  out_config->accelerator = in_config->accelerator;
  out_config->accelerator_desc = in_config->accelerator_desc;
  out_config->batch_size = in_config->batch_size;
  out_config->count = in_config->count;
  memcpy(out_config->keys, in_config->keys, kMaxMLPerfBackendConfigs_intf);
  memcpy(out_config->values, in_config->values, kMaxMLPerfBackendConfigs_intf);
  return 0;
}

int convert_backend_status(intf_mlperf_status_t in_status, mlperf_status_t *out_status) {
  if (in_status == INTF_MLPERF_SUCCESS) {
    *out_status = MLPERF_SUCCESS;
    return 0;
  } else if (in_status == INTF_MLPERF_FAILURE) {
    *out_status = MLPERF_FAILURE;
    return 0;
  } else {
    return -1;
  }
}

int convert_backend_data(intf_mlperf_data_t *in_data, mlperf_data_t *out_data) {
  if (!in_data || !out_data) {
    return -1;
  }
  out_data->size = in_data->size;

  mlperf_data_t::Type type_ret;
  switch (in_data->type) {
    case intf_mlperf_data_t::Type::Float32:
      type_ret = mlperf_data_t::Type::Float32;
      break;
    case intf_mlperf_data_t::Type::Uint8:
      type_ret = mlperf_data_t::Type::Uint8;
      break;
    case intf_mlperf_data_t::Type::Int8:
      type_ret = mlperf_data_t::Type::Int8;
      break;
    case intf_mlperf_data_t::Type::Float16:
      type_ret = mlperf_data_t::Type::Float16;
      break;
    case intf_mlperf_data_t::Type::Int32:
      type_ret = mlperf_data_t::Type::Int32;
      break;
    case intf_mlperf_data_t::Type::Int64:
      type_ret = mlperf_data_t::Type::Int64;
      break;
    default:
      return -1;
  }
  out_data->type = type_ret;
  return 0;
}

bool mlperf_backend_matches_hardware(const char **not_allowed_message,
                                     const char **settings,
                                     const mlperf_device_info_t *device_info) {
  MLOGD("Check a support manufacturer: %s  model: %s",
        device_info->manufacturer, device_info->model);
  MLOGD("+ mlperf_backend_matches_hardware");

  *not_allowed_message = nullptr;

  int selected =
      core_ctrl::support_mbe(device_info->manufacturer, device_info->model);
  if (selected != CORE_INVALID) {
    MLOGV("backend core selected [%d]", selected);
    *settings = core_ctrl::get_benchmark_config(selected);
    return true;
  }

  MLOGE("Soc Not supported. Trying next backend");
  *not_allowed_message = "UnsupportedSoc";
  return false;
}

const char *mlperf_backend_vendor_name(mlperf_backend_ptr_t backend_ptr) {
  static const char name[] = "samsung";
  return name;
}

const char *mlperf_backend_accelerator_name(mlperf_backend_ptr_t backend_ptr) {
  return "samsung npu";
}

const char *mlperf_backend_name(mlperf_backend_ptr_t backend_ptr) {
  static const char name[] = "samsung";
  return name;
}

mlperf_status_t mlperf_backend_flush_queries(mlperf_backend_ptr_t backend_ptr) {
  return MLPERF_SUCCESS;
}

mlperf_backend_ptr_t mlperf_backend_create(
    const char *model_path, mlperf_backend_configuration_t *configs,
    const char *native_lib_path) {
  MLOGD("mlperf_backend_create");

  if (mbe_core) {
    MLOGD("mbe object exist.");
    mbe_core->unload_core_library();
  }

  MLOGD("ready to load mbe library");
  mbe_core = new mbe_core_holder();
  bool ret = mbe_core->load_core_library(native_lib_path);
  if (ret == false) return nullptr;
  intf_mlperf_backend_configuration_t intf_configs;
  if (convert_backend_configuration(configs, &intf_configs)) {
    return nullptr;
  }

  mbe_core->create_fp(model_path, &intf_configs, native_lib_path);
  MLOGD("ptr of mbe_core[%p]", mbe_core);
  return (mlperf_backend_ptr_t)mbe_core;
}

int32_t mlperf_backend_get_input_count(mlperf_backend_ptr_t backend_ptr) {
  mbe_core_holder *ptr = (mbe_core_holder *)backend_ptr;
  MLOGD("+ mlperf_backend_get_input_count with ptr[%p]", ptr);
  return ptr->get_input_count_fp();
}

mlperf_data_t mlperf_backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                            int32_t i) {
  mbe_core_holder *ptr = (mbe_core_holder *)backend_ptr;
  MLOGD("+ mlperf_backend_get_input_type with ptr[%p]", ptr);
  intf_mlperf_data_t intf_data = ptr->get_input_type_fp(i);

  mlperf_data_t data = {mlperf_data_t::Type::Uint8, 0};
  if (convert_backend_data(&intf_data, &data)) {
    MLOGE("fail to convert backend_data. return size 0.\n");
  }
  return data;
}

mlperf_status_t mlperf_backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                         int32_t batchIndex, int32_t i,
                                         void *data) {
  mbe_core_holder *ptr = (mbe_core_holder *)backend_ptr;
  MLOGD("+ mlperf_backend_set_input with mbe_core_holder[%p]", ptr);
  intf_mlperf_status_t intf_status =  ptr->set_input_fp(batchIndex, i, data);

  mlperf_status_t status;
  if (convert_backend_status(intf_status, &status)) {
    return MLPERF_FAILURE;
  }
  return status;
}

int32_t mlperf_backend_get_output_count(mlperf_backend_ptr_t backend_ptr) {
  mbe_core_holder *ptr = (mbe_core_holder *)backend_ptr;
  MLOGD("+ mlperf_backend_get_output_count with ptr[%p]", ptr);
  return ptr->get_output_count_fp();
}

mlperf_data_t mlperf_backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                             int32_t i) {
  mbe_core_holder *ptr = (mbe_core_holder *)backend_ptr;
  MLOGD("+ mlperf_backend_get_output_type with ptr[%p]", ptr);
  intf_mlperf_data_t intf_data = ptr->get_output_type_fp(i);

  mlperf_data_t data = {mlperf_data_t::Type::Uint8, 0};
  if (convert_backend_data(&intf_data, &data)) {
    MLOGE("fail to convert backend_data. return size 0.\n");
  }
  return data;
}

mlperf_status_t mlperf_backend_issue_query(mlperf_backend_ptr_t backend_ptr) {
  mbe_core_holder *ptr = (mbe_core_holder *)backend_ptr;
  MLOGD("+ mlperf_backend_issue_query with ptr[%p]", ptr);
  intf_mlperf_status_t intf_status = ptr->issue_query_fp();

  mlperf_status_t status;
  if (convert_backend_status(intf_status, &status)) {
    return MLPERF_FAILURE;
  }
  return status;
}

mlperf_status_t mlperf_backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                          uint32_t batchIndex, int32_t i,
                                          void **data) {
  mbe_core_holder *ptr = (mbe_core_holder *)backend_ptr;
  MLOGD("+ mlperf_backend_get_output with ptr[%p]", ptr);
  intf_mlperf_status_t intf_status = ptr->get_output_fp(batchIndex, i, data);

  mlperf_status_t status;
  if (convert_backend_status(intf_status, &status)) {
    return MLPERF_FAILURE;
  }
  return status;
}

void mlperf_backend_convert_inputs(void *backend_ptr, int bytes, int width,
                                   int height, uint8_t *data) {
  mbe_core_holder *ptr = (mbe_core_holder *)backend_ptr;
  MLOGD("+ mlperf_backend_convert_inputs with ptr[%p]", ptr);
  ptr->convert_inputs_fp(bytes, width, height, data);
}

void mlperf_backend_delete(mlperf_backend_ptr_t backend_ptr) {
  mbe_core_holder *ptr = (mbe_core_holder *)backend_ptr;
  MLOGD("+ mlperf_backend_delete with ptr[%p]", ptr);
  ptr->delete_fp();

  delete (ptr);
  ptr = nullptr;
}

void *mlperf_backend_get_buffer(size_t size) {
  MLOGD("+ mlperf_backend_get_buffer with size[%zu]", size);
  return mbe_core->get_buffer_fp(size);
}

void mlperf_backend_release_buffer(void *p) {
  MLOGD("+ mlperf_backend_release_buffer with ptr[%p]", p);
  mbe_core->release_buffer_fp(p);
}
