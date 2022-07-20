#import <CoreML/CoreML.h>

#include <cstring>

#include "coreml_settings.h"
#include "coreml_util.h"
#include "flutter/cpp/c/backend_c.h"
#include "flutter/cpp/c/type.h"
#include "flutter/cpp/utils.h"

struct CoreMLBackendData {
  const char *name = "Core ML";
  const char *vendor = "Apple";
  const char *accelerator = "Neural Engine";
  CoreMLExecutor *coreMLExecutor{nullptr};
  uint32_t real_batch_size = 1;
  int32_t original_tensor_size = 0;
};

static bool backendExists = false;

// Return the name of the backend
const char *mlperf_backend_vendor_name(mlperf_backend_ptr_t backend_ptr) {
  return ((CoreMLBackendData *)backend_ptr)->vendor;
}

// Return the name of the accelerator.
const char *mlperf_backend_accelerator_name(mlperf_backend_ptr_t backend_ptr) {
  return ((CoreMLBackendData *)backend_ptr)->accelerator;
}

// Return the name of this backend.
const char *mlperf_backend_name(mlperf_backend_ptr_t backend_ptr) {
  return ((CoreMLBackendData *)backend_ptr)->name;
}

// Should return true if current hardware is supported.
bool mlperf_backend_matches_hardware(const char **not_allowed_message,
                                     const char **settings,
                                     const mlperf_device_info_t *device_info) {
  (void)device_info;
  *not_allowed_message = nullptr;
  *settings = coreml_settings.c_str();
  return true;
}

// Create a new backend and return the pointer to it.
mlperf_backend_ptr_t mlperf_backend_create(
    const char *model_path, mlperf_backend_configuration_t *configs,
    const char *native_lib_path) {
  // Verify only one instance of the backend exists at any time
  if (backendExists) {
    LOG(ERROR) << "Only one backend instance should exist at a time";
    return nullptr;
  }

  CoreMLBackendData *backend_data = new CoreMLBackendData();
  backendExists = true;

  // Load the model.
  CoreMLExecutor *coreMLExecutor =
      [[CoreMLExecutor alloc] initWithModelPath:model_path];
  if (!coreMLExecutor) {
    LOG(ERROR) << "Cannot create CoreMLExecutor";
    return nullptr;
  }
  LOG(INFO) << "CoreMLExecutor created";
  backend_data->coreMLExecutor = coreMLExecutor;
  return backend_data;
}

// Destroy the backend pointer and its data.
void mlperf_backend_delete(mlperf_backend_ptr_t backend_ptr) {
  CoreMLBackendData *backend_data = (CoreMLBackendData *)backend_ptr;
  [backend_data->coreMLExecutor release];
  delete backend_data;
  backendExists = false;
}

// Run the inference for a sample.
mlperf_status_t mlperf_backend_issue_query(mlperf_backend_ptr_t backend_ptr) {
  LOG(INFO) << "mlperf_backend_issue_query()";
  CoreMLBackendData *backend_data = (CoreMLBackendData *)backend_ptr;
  if ([backend_data->coreMLExecutor issueQueries]) return MLPERF_SUCCESS;
  return MLPERF_FAILURE;
}

// Flush the staged queries immediately.
mlperf_status_t mlperf_backend_flush_queries(mlperf_backend_ptr_t backend_ptr) {
  LOG(INFO) << "mlperf_backend_flush_queries()";
  CoreMLBackendData *backend_data = (CoreMLBackendData *)backend_ptr;
  if ([backend_data->coreMLExecutor flushQueries]) return MLPERF_SUCCESS;
  return MLPERF_FAILURE;
}

// Return the number of inputs of the model.
int32_t mlperf_backend_get_input_count(mlperf_backend_ptr_t backend_ptr) {
  return [((CoreMLBackendData *)backend_ptr)->coreMLExecutor getInputCount];
}

// Return the type of the ith input.
mlperf_data_t mlperf_backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                            int32_t i) {
  enum mlperf_data_t::Type datatype = mlperf_data_t::Type::Float32;
  auto size = [((CoreMLBackendData *)backend_ptr)->coreMLExecutor getInputSize];
  mlperf_data_t data = {.type = datatype, .size = size};
  return data;
}

// Set the data for ith input.
mlperf_status_t mlperf_backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                         int32_t batchIndex, int32_t i,
                                         void *data) {
  LOG(INFO) << "mlperf_backend_set_input()";
  CoreMLBackendData *backend_data = (CoreMLBackendData *)backend_ptr;
  if ([backend_data->coreMLExecutor setInput:data at:i]) return MLPERF_SUCCESS;
  return MLPERF_FAILURE;
}

// Return the number of outputs for the model.
int32_t mlperf_backend_get_output_count(mlperf_backend_ptr_t backend_ptr) {
  return [((CoreMLBackendData *)backend_ptr)->coreMLExecutor getOutputCount];
}

// Return the type of ith output.
mlperf_data_t mlperf_backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                             int32_t i) {
  enum mlperf_data_t::Type datatype = mlperf_data_t::Type::Float32;
  auto size =
      [((CoreMLBackendData *)backend_ptr)->coreMLExecutor getOutputSize];
  mlperf_data_t data = {.type = datatype, .size = size};
  return data;
}

// Get the data from ith output.
mlperf_status_t mlperf_backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                          uint32_t batchIndex, int32_t i,
                                          void **data) {
  LOG(INFO) << "mlperf_backend_get_output()";
  if ([((CoreMLBackendData *)backend_ptr)->coreMLExecutor getOutput:data at:i])
    return MLPERF_SUCCESS;
  return MLPERF_FAILURE;
}
