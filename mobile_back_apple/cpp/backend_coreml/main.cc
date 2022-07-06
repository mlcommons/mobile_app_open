#include <cstring>

#include "coreml_settings.h"
#include "coreml_settings.h"

#include "flutter/cpp/c/backend_c.h"
#include "flutter/cpp/c/type.h"

#import <CoreML/CoreML.h>

struct CoreMLBackendData {
  const char *name = "Core ML";
  const char *vendor = "Apple";
  const char *accelerator = "Neural Engine";
  MLModel *model{nullptr};
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
  *settings = coreml_settings.c_str();
  *not_allowed_message = "This is a stub backend for Core ML";
  return true;
}

// Create a new backend and return the pointer to it.
mlperf_backend_ptr_t mlperf_backend_create(
    const char *model_path, mlperf_backend_configuration_t *configs,
    const char *native_lib_path) {

  // Verify only one instance of the backend exists at any time
  if (backendExists) {
    NSLog(@"Only one backend instance should exist at a time");
    return nullptr;
  }

  CoreMLBackendData *backend_data = new CoreMLBackendData();

  backendExists = true;

  // Load the model.
  NSURL *modelURL = [NSURL URLWithString: [NSString stringWithCString: model_path encoding: NSUTF8StringEncoding]];
  NSURL *compiledModelURL = [MLModel compileModelAtURL: modelURL error: nil];
  MLModel *mlmodel = [MLModel modelWithContentsOfURL: compiledModelURL error: nil];
  backend_data->model = mlmodel;

  return backend_data;
}

// Destroy the backend pointer and its data.
void mlperf_backend_delete(mlperf_backend_ptr_t backend_ptr) {
  (void)backend_ptr;
}

// Run the inference for a sample.
mlperf_status_t mlperf_backend_issue_query(mlperf_backend_ptr_t backend_ptr) {
  return MLPERF_FAILURE;
}

// Flush the staged queries immediately.
mlperf_status_t mlperf_backend_flush_queries(mlperf_backend_ptr_t backend_ptr) {
  return MLPERF_FAILURE;
}

// Return the number of inputs of the model.
int32_t mlperf_backend_get_input_count(mlperf_backend_ptr_t backend_ptr) {
  NSLog(@"input %d", [[[((CoreMLBackendData *) backend_ptr)->model modelDescription] inputDescriptionsByName] count]);
  return [[[((CoreMLBackendData *) backend_ptr)->model modelDescription] inputDescriptionsByName] count];
}

// Return the type of the ith input.
mlperf_data_t mlperf_backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                            int32_t i) {
  enum mlperf_data_t::Type datatype = mlperf_data_t::Type::Float32;
  mlperf_data_t data = {.type = datatype, .size = 0};
  return data;
}

// Set the data for ith input.
mlperf_status_t mlperf_backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                         int32_t batchIndex, int32_t i,
                                         void *data) {
  return MLPERF_FAILURE;
}

// Return the number of outputs for the model.
int32_t mlperf_backend_get_output_count(mlperf_backend_ptr_t backend_ptr) {
  NSLog(@"output count %d", [[[((CoreMLBackendData *) backend_ptr)->model modelDescription] outputDescriptionsByName] count]);
  return [[[((CoreMLBackendData *) backend_ptr)->model modelDescription] outputDescriptionsByName] count];
}

// Return the type of ith output.
mlperf_data_t mlperf_backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                             int32_t i) {
  enum mlperf_data_t::Type datatype = mlperf_data_t::Type::Float32;
  mlperf_data_t data = {.type = datatype, .size = 0};
  return data;
}

// Get the data from ith output.
mlperf_status_t mlperf_backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                          uint32_t batchIndex, int32_t i,
                                          void **data) {
  return MLPERF_FAILURE;
}
