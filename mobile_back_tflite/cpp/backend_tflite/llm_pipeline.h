/* Copyright 2024 The MLPerf Authors. All Rights Reserved.
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
#include "tensorflow/lite/c/c_api.h"
#include "tensorflow/lite/c/c_api_experimental.h"
#include "tensorflow/lite/experimental/genai/genai_ops.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/interpreter_builder.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model_builder.h"
#include "tensorflow/lite/signature_runner.h"
#include "tensorflow/lite/util.h"
#include "src/sentencepiece_processor.h"

#include "tensorflow/core/platform/logging.h"

#define MINIMAL_CHECK(x)                                                     \
if (!(x)) {                                                                  \
      LOG(ERROR) << "Error at " << __FILE__ << ":" << __LINE__ << std::endl; \
      return MLPERF_FAILURE;                                                 \
}
#define MINIMAL_CHECK_PTR(x)                                                     \
if (!(x)) {                                                                  \
      LOG(ERROR) << "Error at " << __FILE__ << ":" << __LINE__ << std::endl; \
      return nullptr;                                                 \
}
#define MINIMAL_CHECK_VOID(x)                                                     \
if (!(x)) {                                                                  \
      LOG(ERROR) << "Error at " << __FILE__ << ":" << __LINE__ << std::endl; \
      return;                                                 \
}

// TF Lite requires all buffers (including external buffers used for KV cache
// here) be `tflite::kDefaultTensorAlignment` aligned. To ensure that, we use
// this custom allocator. Please use with caution as different platforms may
// have different alignment requirements.
template <typename T>
class AlignedAllocator {
 public:
  using value_type = T;

  T* allocate(std::size_t n) {
    void* ptr;
    std::size_t size = n * sizeof(T);
    std::size_t padding = tflite::kDefaultTensorAlignment -
                          (size % tflite::kDefaultTensorAlignment);
    size += padding;
    int ret = posix_memalign(&ptr, tflite::kDefaultTensorAlignment, size);
    if (ret != 0) {
      return nullptr;
    }
    return static_cast<T*>(ptr);
  };

  void deallocate(T* ptr, std::size_t n) { free(ptr); }
};

using kv_cache_t = std::map<std::string, std::vector<float, AlignedAllocator<float>>>;

struct LLMBackendData {
  const char *name = "TFLite";
  const char *vendor = "Google";
  const char *accelerator = "CPU";
  TfLiteModel *model{nullptr};
  sentencepiece::SentencePieceProcessor *sp_processor{nullptr};
  //TfLiteInterpreterOptions *options{}; TODO use this to allow different delegates other than CPU?
  TfLiteInterpreter *interpreter{};
  TfLiteSignatureRunner *prefill_runner{nullptr};
  TfLiteSignatureRunner *decode_runner{nullptr};
  kv_cache_t kv_cache;
  //std::string input_prompt;
  std::vector<int> prompt_tokens;
  uint8_t threads = 1;
  std::string start_token = "<bos>";
  std::string end_token = "<eos>";
  int stop_token_id = -1;
  std::string output;

//  uint32_t real_batch_size = 1;
//std::unique_ptr<Threadpool> executer;
//  int32_t original_tensor_size = 0;
//#ifdef MTK_TFLITE_NEURON_BACKEND
//  neuron_backend_ptr_t neuronBackendData{nullptr};
//#endif
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
  TfLiteInterpreter *BuildInterpreter(TfLiteModel *model, int num_threads);
  kv_cache_t BuildKVCache(TfLiteInterpreter *interpreter);
  void PrepareRunner(TfLiteSignatureRunner *runner, kv_cache_t &kv_cache);
  TfLiteSignatureRunner *GetPrefillRunner(TfLiteInterpreter *interpreter, std::size_t num_input_tokens, kv_cache_t &kv_cache);
  TfLiteSignatureRunner *GetDecodeRunner(TfLiteInterpreter *interpreter, kv_cache_t &kv_cache);
  sentencepiece::SentencePieceProcessor *LoadSentencePieceProcessor(std::string path);
  int GreedySampler(const TfLiteTensor *logits);
  TfLiteRegistration* GetGenAIGenerateOp();
  bool TfLiteSignatureRunnerSetInputCustomAllocation(struct TfLiteSignatureRunner* runner, const char* input_name, const struct TfLiteCustomAllocation* allocation);
  bool TfLiteSignatureRunnerSetOutputCustomAllocation(struct TfLiteSignatureRunner* runner, const char* output_name, const struct TfLiteCustomAllocation* allocation);
  bool TfLiteSignatureRunnerAllocateTensors(TfLiteSignatureRunner* runner);

};

#endif  // TFLITE_SINGLE_MODEL_PIPELINE_H_
