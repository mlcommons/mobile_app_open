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

#include <map>
#include <string>
#include <vector>

#include "flutter/cpp/c/type.h"
#include "pipeline.h"
#include "src/sentencepiece_processor.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/interpreter_builder.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model_builder.h"
#include "tensorflow/lite/signature_runner.h"

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

// TF Lite requires all buffers (including external buffers used for KV cache
// here) be `tflite::kDefaultTensorAlignment` aligned. To ensure that, we use
// this custom allocator. Please use with caution as different platforms may
// have different alignment requirements.
template <typename T>
class AlignedAllocator {
 public:
  using value_type = T;

  T *allocate(std::size_t n) {
    void *ptr;
    std::size_t size = n * sizeof(T);
    // NOTE this part of the code from seems to be redundant
    // std::size_t padding = tflite::kDefaultTensorAlignment -
    //                      (size % tflite::kDefaultTensorAlignment);
    // size += padding;
    int ret = posix_memalign(&ptr, tflite::kDefaultTensorAlignment, size);
    if (ret != 0) {
      return nullptr;
    }
    return static_cast<T *>(ptr);
  };

  void deallocate(T *ptr, std::size_t n) { free(ptr); }
};

using kv_cache_t =
    std::map<std::string, std::vector<float, AlignedAllocator<float>>>;

// A simple container for pointers to the tensors used during inference.
// The pointers here should not be managed or deleted by this struct.
struct LLMTensors {
  bool get_tensors(tflite::SignatureRunner *prefill_runner,
                   tflite::SignatureRunner *decode_runner) {
    prefill_input_ = prefill_runner->input_tensor("tokens");
    prefill_input_pos_ = prefill_runner->input_tensor("input_pos");
    decode_input_ = decode_runner->input_tensor("tokens");
    decode_input_pos_ = decode_runner->input_tensor("input_pos");
    logits_output_ = decode_runner->output_tensor("logits");
    kv_cache_k_0_ = decode_runner->input_tensor("kv_cache_k_0");

    // Making sure none of the tensors are nullptr.
    return prefill_input_ && prefill_input_pos_ && decode_input_ &&
           decode_input_pos_ && logits_output_ && kv_cache_k_0_;
  }

  LLMTensors() {}

  LLMTensors(const LLMTensors &) = delete;
  LLMTensors &operator=(const LLMTensors &) = delete;

  TfLiteTensor *prefill_input() const { return prefill_input_; }
  TfLiteTensor *prefill_input_pos() const { return prefill_input_pos_; }
  TfLiteTensor *decode_input() const { return decode_input_; }
  TfLiteTensor *decode_input_pos() const { return decode_input_pos_; }
  const TfLiteTensor *logits_output() const { return logits_output_; }
  TfLiteTensor *kv_cache_k_0() const { return kv_cache_k_0_; }

 private:
  // Shape: [Batch, Seq], Dtype: int32
  TfLiteTensor *prefill_input_;
  // Shape: [Seq], Dtype: int32
  TfLiteTensor *prefill_input_pos_;
  // Shape: [Batch, Seq], Dtype: int32
  TfLiteTensor *decode_input_;
  // Shape: [Seq], Dtype: int32
  TfLiteTensor *decode_input_pos_;
  // Shape: [Seq], Dtype: float32
  const TfLiteTensor *logits_output_;
  // shape: [Batch, kv_cache_max, num_query_groups, head_dim]
  TfLiteTensor *kv_cache_k_0_;
};

struct LLMBackendData {
  const char *name = "TFLite";
  const char *vendor = "Google";
  const char *accelerator = "CPU";
  tflite::FlatBufferModel *model{nullptr};
  sentencepiece::SentencePieceProcessor *sp_processor{nullptr};
  // TfLiteInterpreterOptions *options{}; TODO use this to allow different
  // delegates other than CPU?
  tflite::Interpreter *interpreter{};
  tflite::SignatureRunner *prefill_runner{nullptr};
  tflite::SignatureRunner *decode_runner{nullptr};
  LLMTensors tensors;
  kv_cache_t kv_cache;
  std::vector<int> prompt_tokens;
  std::vector<int> output_tokens;
  std::string output;
  uint8_t threads = 30;
  int max_output_tokens = 2;
  std::string start_token = "<bos>";
  std::string end_token = "<eos>";
  int stop_token_id = -1;

  LLMBackendData() {}

  ~LLMBackendData() {
    // Runners are owned by interpreter and therefore don't need to be deleted
    delete sp_processor;
    delete interpreter;
    delete model;
  }

  LLMBackendData(const LLMBackendData &) = delete;
  LLMBackendData &operator=(const LLMBackendData &) = delete;
};

// A simple pipeline which runs a single model.
class LLMPipeline : public Pipeline {
 public:
  LLMPipeline() = default;

  ~LLMPipeline() override = default;

  void backend_delete(mlperf_backend_ptr_t backend_ptr) override;

  mlperf_backend_ptr_t backend_create(const char *model_path,
                                      mlperf_backend_configuration_t *configs,
                                      const char *native_lib_path) override;

  const char *backend_vendor_name(mlperf_backend_ptr_t backend_ptr) override;

  const char *backend_accelerator_name(
      mlperf_backend_ptr_t backend_ptr) override;

  const char *backend_name(mlperf_backend_ptr_t backend_ptr) override;

  mlperf_status_t backend_issue_first_token_query(
      mlperf_backend_ptr_t backend_ptr) override;

  mlperf_status_t backend_issue_query(
      mlperf_backend_ptr_t backend_ptr) override;

  mlperf_status_t backend_flush_queries(
      mlperf_backend_ptr_t backend_ptr) override;

  int32_t backend_get_input_count(mlperf_backend_ptr_t backend_ptr) override;

  mlperf_data_t backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                       int32_t i) override;

  mlperf_status_t backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                    int32_t batch_index, int32_t i,
                                    void *data) override;

  int32_t backend_get_output_count(mlperf_backend_ptr_t backend_ptr) override;

  mlperf_data_t backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                        int32_t i) override;

  mlperf_status_t backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                     uint32_t batchIndex, int32_t i,
                                     void **data) override;

  void backend_convert_inputs(mlperf_backend_ptr_t backend_ptr, int bytes,
                              int width, int height, uint8_t *data) override;

  void backend_convert_outputs(mlperf_backend_ptr_t backend_ptr, int bytes,
                               int width, int height, uint8_t *data) override;

  void *backend_get_buffer(size_t n) override;

  void backend_release_buffer(void *p) override;

 private:
  tflite::Interpreter *BuildInterpreter(tflite::FlatBufferModel *model,
                                        int num_threads);
  kv_cache_t BuildKVCache(tflite::Interpreter *interpreter);
  void PrepareRunner(tflite::SignatureRunner *runner, kv_cache_t &kv_cache);
  tflite::SignatureRunner *GetPrefillRunner(tflite::Interpreter *interpreter,
                                            std::size_t num_input_tokens,
                                            kv_cache_t &kv_cache);
  tflite::SignatureRunner *GetDecodeRunner(tflite::Interpreter *interpreter,
                                           kv_cache_t &kv_cache);
  sentencepiece::SentencePieceProcessor *LoadSentencePieceProcessor(
      std::string path);
  int GreedySampler(const TfLiteTensor *logits);
};

#endif  // TFLITE_SINGLE_MODEL_PIPELINE_H_
