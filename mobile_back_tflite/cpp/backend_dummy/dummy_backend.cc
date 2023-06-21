
#include "flutter/cpp/c/backend_c.h"

bool mlperf_backend_matches_hardware(const char** not_allowed_message,
                                     const char** settings,
                                     const mlperf_device_info_t* device_info,
                                     const char *native_lib_path) {
  return false;
}

mlperf_backend_ptr_t mlperf_backend_create(
    const char* model_path, mlperf_backend_configuration_t* configs,
    const char* native_lib_path) {
  return nullptr;
}

const char* mlperf_backend_vendor_name(mlperf_backend_ptr_t backend_ptr) {
  return "";
}

const char* mlperf_backend_accelerator_name(mlperf_backend_ptr_t backend_ptr) {
  return "";
}

const char* mlperf_backend_name(mlperf_backend_ptr_t backend_ptr) { return ""; }

void mlperf_backend_delete(mlperf_backend_ptr_t backend_ptr) {}

mlperf_status_t mlperf_backend_issue_query(mlperf_backend_ptr_t backend_ptr) {
  return MLPERF_FAILURE;
}

mlperf_status_t mlperf_backend_flush_queries(mlperf_backend_ptr_t backend_ptr) {
  return MLPERF_FAILURE;
}

int32_t mlperf_backend_get_input_count(mlperf_backend_ptr_t backend_ptr) {
  return 0;
}

mlperf_data_t mlperf_backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                            int32_t i) {
  mlperf_data_t result;
  result.type = mlperf_data_t::Float32;
  result.size = 0;
  return result;
}
mlperf_status_t mlperf_backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                         int32_t batchIndex, int32_t i,
                                         void* data) {
  return MLPERF_FAILURE;
}

int32_t mlperf_backend_get_output_count(mlperf_backend_ptr_t backend_ptr) {
  return 0;
}

mlperf_data_t mlperf_backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                             int32_t i) {
  mlperf_data_t result;
  result.type = mlperf_data_t::Float32;
  result.size = 0;
  return result;
}

mlperf_status_t mlperf_backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                          uint32_t batchIndex, int32_t i,
                                          void** data) {
  return MLPERF_FAILURE;
}
