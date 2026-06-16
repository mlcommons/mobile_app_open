/* Copyright 2025 The MLPerf Authors. All Rights Reserved.

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

#ifndef TFLITE_LLM_PIPELINE_H_
#define TFLITE_LLM_PIPELINE_H_

#include <stdlib.h>

#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#if defined(_MSC_VER)
#include <malloc.h>
#endif

#include "absl/log/log.h"
#include "flutter/cpp/c/type.h"
#include "litert/cc/litert_compiled_model.h"
#include "litert/cc/litert_environment.h"
#include "litert/cc/litert_tensor_buffer.h"
#include "pipeline.h"

#define MINIMAL_CHECK(x)                                                   \
  if (!(x)) {                                                              \
    LOG(ERROR) << "Error at " << __FILE__ << ":" << __LINE__ << std::endl; \
    return MLPERF_FAILURE;                                                 \
  }
#define MINIMAL_CHECK_PTR(x)                                               \
  if (!(x)) {                                                              \
    LOG(ERROR) << "Error at " << __FILE__ << ":" << __LINE__ << std::endl; \
    return nullptr;                                                        \
  }
#define MINIMAL_CHECK_VOID(x)                                              \
  if (!(x)) {                                                              \
    LOG(ERROR) << "Error at " << __FILE__ << ":" << __LINE__ << std::endl; \
    return;                                                                \
  }

struct LLMBackendData {
  const char* name = "TFLite";
  const char* vendor = "Google";
  const char* accelerator = "CPU";

  std::unique_ptr<litert::CompiledModel> model;
  std::unique_ptr<litert::Environment> env;
  std::vector<std::pair<size_t, size_t>> prefill_sigs;  // (sig_idx, seq_size)
  std::vector<litert::TensorBuffer> decode_input_bufs;
  std::vector<litert::TensorBuffer> decode_output_bufs;
  std::vector<litert::TensorBuffer> prefill_input_bufs;
  std::vector<litert::TensorBuffer> prefill_output_bufs;
  std::unordered_map<std::string, size_t> decode_input_map;
  std::unordered_map<std::string, size_t> decode_output_map;
  std::unordered_map<std::string, size_t> prefill_input_map;
  std::unordered_map<std::string, size_t> prefill_output_map;
  size_t decode_sig_idx = 0;
  size_t current_prefill_sig_idx = SIZE_MAX;
  size_t prefill_tokens_idx = 0;
  size_t prefill_pos_idx = 0;
  size_t decode_tokens_idx = 0;
  size_t decode_pos_idx = 0;
  size_t logits_idx = 0;
  int num_kv_layers = 0;
  int kv_cache_max_size = 0;
  int kv_buf_float_count = 0;
  int prefill_seq_size = 0;
  std::vector<float> logits_scratch;
  int vocab_size = 0;

  std::vector<int> prompt_tokens;
  std::vector<int> output_tokens;
  uint16_t num_threads = 4;
  int max_output_tokens = 128;
  std::unordered_set<int> stop_token_ids{128001, 128008, 128009};

  LLMBackendData() {}

  ~LLMBackendData() {}

  LLMBackendData(const LLMBackendData&) = delete;
  LLMBackendData& operator=(const LLMBackendData&) = delete;
};

// A simple pipeline which runs a single model.
class LLMPipeline : public Pipeline {
 public:
  LLMPipeline() = default;

  ~LLMPipeline() override = default;

  void backend_delete(mlperf_backend_ptr_t backend_ptr) override;

  mlperf_backend_ptr_t backend_create(const char* model_path,
                                      mlperf_backend_configuration_t* configs,
                                      const char* native_lib_path) override;

  const char* backend_vendor_name(mlperf_backend_ptr_t backend_ptr) override;

  const char* backend_accelerator_name(
      mlperf_backend_ptr_t backend_ptr) override;

  const char* backend_name(mlperf_backend_ptr_t backend_ptr) override;

  mlperf_status_t backend_issue_first_token_query(
      mlperf_backend_ptr_t backend_ptr);

  mlperf_status_t backend_issue_query(mlperf_backend_ptr_t backend_ptr,
                                      ft_callback callback,
                                      void* context) override;

  mlperf_status_t backend_flush_queries(
      mlperf_backend_ptr_t backend_ptr) override;

  int32_t backend_get_input_count(mlperf_backend_ptr_t backend_ptr) override;

  mlperf_data_t backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                       int32_t i) override;

  mlperf_status_t backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                    int32_t batch_index, int32_t i,
                                    void* data) override;

  int32_t backend_get_output_count(mlperf_backend_ptr_t backend_ptr) override;

  mlperf_data_t backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                        int32_t i) override;

  mlperf_status_t backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                     uint32_t batchIndex, int32_t i,
                                     void** data) override;

  void backend_convert_inputs(mlperf_backend_ptr_t backend_ptr, int bytes,
                              int width, int height, uint8_t* data) override;

  void backend_convert_outputs(mlperf_backend_ptr_t backend_ptr, int bytes,
                               int width, int height, uint8_t* data) override;

  void* backend_get_buffer(size_t n) override;

  void backend_release_buffer(void* p) override;

 private:
  bool BuildCompiledModel(LLMBackendData* backend_ptr, const char* model_path);
  bool BuildDecodeBuffers(LLMBackendData* backend_ptr);
  bool BuildPrefillBuffers(LLMBackendData* backend_ptr, size_t prefill_sig_idx);
  size_t GetSuitablePrefillSignature(LLMBackendData* backend_ptr,
                                     size_t num_input_tokens) const;
  void TransferKV(LLMBackendData* backend_ptr);
  void UpdateDecodeKV(LLMBackendData* backend_ptr);
  void ResetKV(LLMBackendData* backend_ptr);
  void ResetPrefillKV(LLMBackendData* backend_ptr);
  int GreedySampler(LLMBackendData* backend_ptr);
};

#endif  // TFLITE_SINGLE_MODEL_PIPELINE_H_
