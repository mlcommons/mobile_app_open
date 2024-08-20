
#include "stable_diffusion_pipeline.h"

#include "flutter/cpp/c/backend_c.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

mlperf_backend_ptr_t StableDiffusionPipeline::backend_create(
    const char* model_path, mlperf_backend_configuration_t* configs,
    const char* native_lib_path) {
  return nullptr;
}

const char* StableDiffusionPipeline::backend_vendor_name(
    mlperf_backend_ptr_t backend_ptr) {
  return "";
}

const char* StableDiffusionPipeline::backend_accelerator_name(
    mlperf_backend_ptr_t backend_ptr) {
  return "";
}

const char* StableDiffusionPipeline::backend_name(
    mlperf_backend_ptr_t backend_ptr) {
  return "";
}

void StableDiffusionPipeline::backend_delete(mlperf_backend_ptr_t backend_ptr) {
}

mlperf_status_t StableDiffusionPipeline::backend_issue_query(
    mlperf_backend_ptr_t backend_ptr) {
  return MLPERF_FAILURE;
}

mlperf_status_t StableDiffusionPipeline::backend_flush_queries(
    mlperf_backend_ptr_t backend_ptr) {
  return MLPERF_FAILURE;
}

int32_t StableDiffusionPipeline::backend_get_input_count(
    mlperf_backend_ptr_t backend_ptr) {
  return 0;
}

mlperf_data_t StableDiffusionPipeline::backend_get_input_type(
    mlperf_backend_ptr_t backend_ptr, int32_t i) {
  mlperf_data_t result;
  result.type = mlperf_data_t::Float32;
  result.size = 0;
  return result;
}
mlperf_status_t StableDiffusionPipeline::backend_set_input(
    mlperf_backend_ptr_t backend_ptr, int32_t batchIndex, int32_t i,
    void* data) {
  return MLPERF_FAILURE;
}

int32_t StableDiffusionPipeline::backend_get_output_count(
    mlperf_backend_ptr_t backend_ptr) {
  return 0;
}

mlperf_data_t StableDiffusionPipeline::backend_get_output_type(
    mlperf_backend_ptr_t backend_ptr, int32_t i) {
  mlperf_data_t result;
  result.type = mlperf_data_t::Float32;
  result.size = 0;
  return result;
}

mlperf_status_t StableDiffusionPipeline::backend_get_output(
    mlperf_backend_ptr_t backend_ptr, uint32_t batchIndex, int32_t i,
    void** data) {
  return MLPERF_FAILURE;
}

void StableDiffusionPipeline::backend_convert_inputs(
    mlperf_backend_ptr_t backend_ptr, int bytes, int width, int height,
    uint8_t* data) {}

void* StableDiffusionPipeline::backend_get_buffer(size_t n) { return nullptr; }
void StableDiffusionPipeline::backend_release_buffer(void* p) {}

#ifdef __cplusplus
}
#endif  // __cplusplus
