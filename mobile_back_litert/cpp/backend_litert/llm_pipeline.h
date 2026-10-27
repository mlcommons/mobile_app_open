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

#ifndef LITERT_LLM_PIPELINE_H_
#define LITERT_LLM_PIPELINE_H_

#include <stdlib.h>

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
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
  const char* name = "LiteRT";
  const char* vendor = "Google";
  const char* accelerator = "CPU";

  // LiteRT requires the environment to outlive the compiled model built in it
  // ("the provided environment must outlive the compiled model and any
  // executions running on it" -- litert_compiled_model.h). The destructor
  // below is what enforces that here: it clears the buffers, then the model,
  // then the environment, and a destructor body runs before its members are
  // destroyed. Declaration order is kept consistent with it anyway -- and with
  // the ordering SDBackendData documents -- so the two cannot drift if that
  // destructor is ever simplified away.
  std::unique_ptr<litert::Environment> env;
  std::unique_ptr<litert::CompiledModel> model;
  std::vector<std::pair<size_t, size_t>> prefill_sigs;  // (sig_idx, seq_size)

  std::unique_ptr<litert::CompiledModel> embedder;
  std::unique_ptr<litert::CompiledModel> per_layer_embedder;
  std::vector<litert::TensorBuffer> emb_decode_in, emb_decode_out;
  std::vector<litert::TensorBuffer> emb_prefill_in, emb_prefill_out;
  std::vector<litert::TensorBuffer> ple_decode_in, ple_decode_out;
  std::vector<litert::TensorBuffer> ple_prefill_in, ple_prefill_out;
  size_t emb_decode_sig_idx = 0;
  size_t emb_prefill_sig_idx = 0;
  size_t ple_decode_sig_idx = 0;
  size_t ple_prefill_sig_idx = 0;
  bool external_embedder = false;
  bool has_per_layer_embedder = false;
  size_t decode_embeddings_idx = 0;
  size_t decode_ple_idx = 0;
  size_t prefill_embeddings_idx = 0;
  size_t prefill_ple_idx = 0;
  size_t emb_decode_floats = 0, emb_prefill_floats = 0;
  size_t ple_decode_floats = 0, ple_prefill_floats = 0;

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
  size_t prefill_mask_idx = 0;
  size_t decode_mask_idx = 0;
  bool has_mask_input = false;  // model exported with a mask input
  bool mask_is_bool = false;    // mask element type: bool vs float32

  int num_kv_layers = 0;
  int kv_cache_max_size = 0;
  std::vector<int> kv_k_float_counts;
  std::vector<int> kv_v_float_counts;
  int prefill_seq_size = 0;
  int vocab_size = 0;

  std::vector<int> prompt_tokens;
  std::vector<int> output_tokens;
  uint16_t num_threads = 4;
  int max_output_tokens = 128;
  int pad_token_id = 128009;
  std::unordered_set<int> stop_token_ids{128001, 128008, 128009};

  LLMBackendData() {}

  ~LLMBackendData() {
    decode_input_bufs.clear();
    decode_output_bufs.clear();
    prefill_input_bufs.clear();
    prefill_output_bufs.clear();
    emb_decode_in.clear();
    emb_decode_out.clear();
    emb_prefill_in.clear();
    emb_prefill_out.clear();
    ple_decode_in.clear();
    ple_decode_out.clear();
    ple_prefill_in.clear();
    ple_prefill_out.clear();
    embedder.reset();
    per_layer_embedder.reset();
    model.reset();
    env.reset();
  }

  LLMBackendData(const LLMBackendData&) = delete;
  LLMBackendData& operator=(const LLMBackendData&) = delete;
};

// Pipeline for autoregressive LLM inference (prefill + decode).
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
  bool BuildCompiledModel(LLMBackendData& data, const char* model_path,
                          bool use_gpu);
  bool BuildDecodeBuffers(LLMBackendData& data);
  bool BuildPrefillBuffers(LLMBackendData& data, size_t prefill_sig_idx);
  bool BuildEmbedders(LLMBackendData& data, const std::string& embedder_path,
                      const std::string& per_layer_embedder_path);
  bool BuildEmbedderPrefillBuffers(LLMBackendData& data, int seq_size);
  bool RunEmbedders(LLMBackendData& data, const std::vector<int32_t>& tokens,
                    bool prefill);

  // Pick the prefill signature to run the prompt through. Buckets larger than
  // max_useful_seq_size are skipped when a smaller one exists; pass SIZE_MAX
  // to consider every bucket.
  size_t GetSuitablePrefillSignature(
      const std::vector<std::pair<size_t, size_t>>& prefill_sigs,
      size_t num_input_tokens, size_t max_useful_seq_size) const;
  // Move each layer's KV from the prefill outputs into the decode inputs.
  void TransferKV(
      int num_layers,
      const std::unordered_map<std::string, size_t>& prefill_output_map,
      const std::unordered_map<std::string, size_t>& decode_input_map,
      std::vector<litert::TensorBuffer>& prefill_output_bufs,
      std::vector<litert::TensorBuffer>& decode_input_bufs);
  // Swap each layer's KV between the decode inputs and outputs (one step).
  void UpdateDecodeKV(
      int num_layers,
      const std::unordered_map<std::string, size_t>& decode_input_map,
      const std::unordered_map<std::string, size_t>& decode_output_map,
      std::vector<litert::TensorBuffer>& decode_input_bufs,
      std::vector<litert::TensorBuffer>& decode_output_bufs);
  void ResetKV(int num_layers, const std::vector<int>& k_float_counts,
               const std::vector<int>& v_float_counts,
               const std::unordered_map<std::string, size_t>& input_map,
               std::vector<litert::TensorBuffer>& input_bufs);
  void WritePrefillMask(litert::CompiledModel& model, size_t sig_idx,
                        size_t mask_idx, bool mask_is_bool,
                        litert::TensorBuffer& buf);
  void WriteDecodeMask(litert::CompiledModel& model, size_t sig_idx,
                       size_t mask_idx, bool mask_is_bool,
                       litert::TensorBuffer& buf, int position);
  int GreedySampler(litert::TensorBuffer& logits_buf, int vocab_size);
};

#endif  // LITERT_LLM_PIPELINE_H_
