#include <string.h>

#include "flutter/cpp/c/backend_c.h"
#include "flutter/cpp/c/type.h"

// Return the name of the backend
const char *mlperf_backend_vendor_name(mlperf_backend_ptr_t backend_ptr) {
    return "Apple";
}

// Return the name of the accelerator.
const char *mlperf_backend_accelerator_name(mlperf_backend_ptr_t backend_ptr) {
    return "Neural Engine";
}

// Return the name of this backend.
const char *mlperf_backend_name(mlperf_backend_ptr_t backend_ptr) {
    (void) backend_ptr;
    return "CoreML";
}

// Should return true if current hardware is supported.
bool mlperf_backend_matches_hardware(const char **not_allowed_message,
                                     const char **settings,
                                     const mlperf_device_info_t *device_info) {
    (void) settings;
    (void) device_info;
    *not_allowed_message = "This is a stub backend for CoreML";
    return true;
}

// Create a new backend and return the pointer to it.
mlperf_backend_ptr_t mlperf_backend_create(
        const char *model_path, mlperf_backend_configuration_t *configs,
        const char *native_lib_path) {
    return nullptr;
}


// Destroy the backend pointer and its data.
void mlperf_backend_delete(mlperf_backend_ptr_t backend_ptr) {
    (void) backend_ptr;
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
    return -1;
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
    return -1;
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
