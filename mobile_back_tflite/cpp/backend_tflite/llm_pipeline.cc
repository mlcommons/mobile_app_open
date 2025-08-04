/* Copyright 2020-2021 The MLPerf Authors. All Rights Reserved.

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
#include "llm_pipeline.h"

#include <cstdlib>
#include <cstring>
#include <fstream>

#if defined(MTK_TFLITE_NEURON_BACKEND) && defined(__ANDROID__)
#include <dlfcn.h>

#include "neuron/APUWareUtilsApi.h"
#endif

#include "flutter/cpp/c/type.h"
#include "flutter/cpp/utils.h"
#include "tensorflow/lite/c/common.h"
#if __ANDROID__
#include <sys/system_properties.h>

#include "tensorflow/lite/delegates/gpu/delegate.h"
#include "tensorflow/lite/delegates/nnapi/nnapi_delegate.h"
#endif
#include "tensorflow/core/platform/logging.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

static bool backendExists = false;

// Destroy the backend pointer and its data.
void LLMPipeline::backend_delete(mlperf_backend_ptr_t backend_ptr) {
  LLMBackendData *backend_data = (LLMBackendData *)backend_ptr;
  if (backend_data) {
    TfLiteModelDelete(backend_data->model);
    TfLiteSignatureRunnerDelete(backend_data->prefill_runner);
    TfLiteSignatureRunnerDelete(backend_data->decode_runner);
    TfLiteInterpreterDelete(backend_data->interpreter);
    delete backend_data->sp_processor;
    delete backend_data;
  }
  backendExists = false;
}

// Create a new backend and return the pointer to it.
// TODO add eos and bos tokens as config parameters
mlperf_backend_ptr_t LLMPipeline::backend_create(const char *model_path, mlperf_backend_configuration_t *configs, const char *native_lib_path) {
  // Verify only one instance of the backend exists at any time
  if (backendExists) {
    LOG(ERROR) << "Only one backend instance should exist at a time";
    return nullptr;
  }

  LLMBackendData *backend_data = new LLMBackendData();

  // sentencePiece Processor Path
  std::string sppp = mlperf::mobile::GetConfigValue(configs, "sentencepiece_processor_path", std::string(""));

  // Load the model.
  backend_data->model = TfLiteModelCreateFromFile(model_path);
  if (!backend_data->model) {
    LOG(ERROR) << "Failed to load model: " << model_path;
    backend_delete(backend_data);
    return nullptr;
  }

  backend_data->interpreter = BuildInterpreter(backend_data->model, backend_data->threads);
  if (!backend_data->interpreter) {
    LOG(ERROR) << "Failed to load interpreter";
    backend_delete(backend_data);
    return nullptr;
  }

  backend_data->kv_cache = BuildKVCache(backend_data->interpreter);
  //TODO kv_cache check

  backend_data->decode_runner = GetDecodeRunner(backend_data->interpreter, backend_data->kv_cache);

  backend_data->sp_processor = LoadSentencePieceProcessor(sppp);
  if (!backend_data->sp_processor) {
    LOG(ERROR) << "Failed to load sentencepiece processor: " << sppp;
        backend_delete(backend_data);
    return nullptr;
  }

  return backend_data;
}

// Vendor name who create this backend.
const char *LLMPipeline::backend_vendor_name(mlperf_backend_ptr_t backend_ptr) {
  LLMBackendData *backend_data = (LLMBackendData *)backend_ptr;
  return backend_data->vendor;
}

// TODO: Return the name of the accelerator.
const char *LLMPipeline::backend_accelerator_name(mlperf_backend_ptr_t backend_ptr) {
  LLMBackendData *backend_data = (LLMBackendData *)backend_ptr;
  return backend_data->accelerator;
}

// Return the name of this backend.
const char *LLMPipeline::backend_name(mlperf_backend_ptr_t backend_ptr) {
  LLMBackendData *backend_data = (LLMBackendData *)backend_ptr;
  return backend_data->name;
}

// Run the inference for a sample.
mlperf_status_t LLMPipeline::backend_issue_query(mlperf_backend_ptr_t backend_ptr) {
  LLMBackendData *backend_data = (LLMBackendData *)backend_ptr;

  // Get Input Tensors for each of the runners.
  // Shape: [Batch, Seq], Dtype: int32
  TfLiteTensor* prefill_input = TfLiteSignatureRunnerGetInputTensor(backend_data->prefill_runner, "tokens");
  // Shape: [Seq], Dtype: int32
  TfLiteTensor* prefill_input_pos = TfLiteSignatureRunnerGetInputTensor(backend_data->prefill_runner, "input_pos");
  // Shape: [Batch, Seq], Dtype: int32
  TfLiteTensor* decode_input = TfLiteSignatureRunnerGetInputTensor(backend_data->decode_runner, "tokens");
  // Shape: [Seq], Dtype: int32
  TfLiteTensor* decode_input_pos = TfLiteSignatureRunnerGetInputTensor(backend_data->decode_runner, "input_pos");
  // shape: [Batch, kv_cache_max, num_query_groups, head_dim]
  TfLiteTensor* kv_cache_k_0 = TfLiteSignatureRunnerGetInputTensor(backend_data->decode_runner, "kv_cache_k_0");

  int max_seq_size = prefill_input->dims->data[1];
  int kv_cache_max_size = kv_cache_k_0->dims->data[1];
  int prefill_seq_size = std::min(static_cast<int>(backend_data->prompt_tokens.size()), max_seq_size);

  std::memset(prefill_input->data.i32, 0, prefill_input->bytes);
  std::memset(prefill_input_pos->data.i32, 0, prefill_input_pos->bytes);
  for (int i = 0; i < prefill_seq_size - 1; ++i) {
    prefill_input->data.i32[i] = backend_data->prompt_tokens[i];
    prefill_input_pos->data.i32[i] = i;
  }

  MINIMAL_CHECK(TfLiteSignatureRunnerInvoke(backend_data->prefill_runner) == kTfLiteOk);

  int decode_steps = kv_cache_max_size - prefill_seq_size;
  MINIMAL_CHECK(decode_steps > 0);

  std::vector<int> output_tokens;
  output_tokens.reserve(decode_steps);
  int next_token = backend_data->prompt_tokens[prefill_seq_size - 1];
  int next_position = prefill_seq_size - 1;
  for (int i = 0; i < decode_steps; ++i) {
    decode_input->data.i32[0] = next_token;
    decode_input_pos->data.i32[0] = next_position;
    MINIMAL_CHECK(TfLiteSignatureRunnerInvoke(backend_data->decode_runner) == kTfLiteOk);
    next_token = GreedySampler(TfLiteSignatureRunnerGetOutputTensor(backend_data->decode_runner, "logits"));
    output_tokens.push_back(next_token);
    next_position += 1;
    if (next_token == backend_data->stop_token_id) break;
  }

  MINIMAL_CHECK(backend_data->sp_processor->Decode(output_tokens, &backend_data->output).ok());

  return MLPERF_SUCCESS;
}

// Flush the staged queries immediately.
mlperf_status_t LLMPipeline::backend_flush_queries(mlperf_backend_ptr_t backend_ptr) {
  return MLPERF_SUCCESS;
}

// Return the number of inputs of the model.
int32_t LLMPipeline::backend_get_input_count(mlperf_backend_ptr_t backend_ptr) {
  return 2;
}

// Return the type of the ith input.
mlperf_data_t LLMPipeline::backend_get_input_type(mlperf_backend_ptr_t backend_ptr, int32_t i) {
  return mlperf_data_t{mlperf_data_t::Int32, 0};
}

// Set the data for ith input.
mlperf_status_t LLMPipeline::backend_set_input(mlperf_backend_ptr_t backend_ptr, int32_t batch_index, int32_t i, void *data) {
  LLMBackendData *backend_data = (LLMBackendData *)backend_ptr;

  std::string prompt = std::string(static_cast<char*>(data));
  MINIMAL_CHECK(backend_data->sp_processor->Encode(prompt, &backend_data->prompt_tokens).ok()); //TODO

  if (!backend_data->start_token.empty()) {
    backend_data->prompt_tokens.insert(backend_data->prompt_tokens.begin(), backend_data->sp_processor->PieceToId((backend_data->start_token)));
  }

  // NOTE block below can be moved safely to backend_create
  if (!backend_data->end_token.empty()) {
    backend_data->stop_token_id = backend_data->sp_processor->PieceToId((backend_data->end_token));
  }
  // ---

  uint16_t effective_prefill_token_size = backend_data->prompt_tokens.size() - 1; //assuming max tokens is <16k

  backend_data->prefill_runner = GetPrefillRunner(backend_data->interpreter, effective_prefill_token_size, backend_data->kv_cache);


  return MLPERF_SUCCESS;
}

// Return the number of outputs for the model.
int32_t LLMPipeline::backend_get_output_count(mlperf_backend_ptr_t backend_ptr) {
  return 1;
}

// Return the type of ith output.
mlperf_data_t LLMPipeline::backend_get_output_type(mlperf_backend_ptr_t backend_ptr, int32_t i) {
  return mlperf_data_t{mlperf_data_t::Float32, 0};
}

// Get the data from ith output.
mlperf_status_t LLMPipeline::backend_get_output(mlperf_backend_ptr_t backend_ptr, uint32_t batch_index, int32_t i, void **data) {
  LLMBackendData *backend_data = (LLMBackendData *)backend_ptr;

  if (i == 0) {
    *data = backend_data->output.data();
    return MLPERF_SUCCESS;
  }

  return MLPERF_FAILURE;
}

void LLMPipeline::backend_convert_inputs(mlperf_backend_ptr_t backend_ptr, int bytes, int width, int height, uint8_t *data) {}

void LLMPipeline::backend_convert_outputs(mlperf_backend_ptr_t backend_ptr, int bytes, int width, int height, uint8_t *data) {}

void *LLMPipeline::backend_get_buffer(size_t n) {
  return ::operator new(n);
}

void LLMPipeline::backend_release_buffer(void *p) {
  ::operator delete(p);
}

TfLiteInterpreter *LLMPipeline::BuildInterpreter(TfLiteModel *model, int num_threads) {
  TfLiteInterpreterOptions* options = TfLiteInterpreterOptionsCreate();
  TfLiteInterpreterOptionsSetNumThreads(options, num_threads);
  // NOTE: We need to manually register optimized OPs for KV-cache and
  // Scaled Dot Product Attention (SDPA).
  TfLiteInterpreterOptionsAddCustomOp(options, "GEN_AI_GENERATE", GetGenAIGenerateOp(), 1, 1);
  TfLiteInterpreter* interpreter = TfLiteInterpreterCreate(model, options);

  MINIMAL_CHECK_PTR(interpreter != nullptr);

  return interpreter;
}

kv_cache_t LLMPipeline::BuildKVCache(TfLiteInterpreter *interpreter) {
  TfLiteSignatureRunner *runner = TfLiteInterpreterGetSignatureRunner(interpreter, "decode");
  // TODO
  if (runner == nullptr) {
    return {};
  }
  // The two arguments excluded are `tokens` and `input_pos`.
  // TODO more arguments might need to be excluded
  size_t num_layers = (TfLiteSignatureRunnerGetInputCount(runner) - 2) / 2;
  if (num_layers == 0) {
    return {};
  }

  kv_cache_t kv_cache;
  for (int i = 0; i < num_layers; ++i) {
    std::string k_cache_name = "kv_cache_k_" + std::to_string(i);
    std::string v_cache_name = "kv_cache_v_" + std::to_string(i);
    // We are assuming K and V tensors are of the same shape.
    TfLiteTensor* tensor = TfLiteSignatureRunnerGetInputTensor(runner, k_cache_name.c_str());
    size_t count = tensor->bytes / sizeof(float);
    kv_cache.emplace(k_cache_name,
                     std::vector<float, AlignedAllocator<float>>(count, 0.0));
    kv_cache.emplace(v_cache_name,
                     std::vector<float, AlignedAllocator<float>>(count, 0.0));
  }

  return kv_cache;
}

void LLMPipeline::PrepareRunner(TfLiteSignatureRunner* runner, kv_cache_t& kv_cache) {
  for (auto& [name, cache] : kv_cache) {
    TfLiteCustomAllocation allocation = {
                                         .data = static_cast<void*>(cache.data()),
                                         .bytes = cache.size() * sizeof(float)};
    // Both input and output tensors are set to the same buffer. Not all
    // delegates support this in-place update. For those cases, we need to do
    // a ping-pong buffer and update the pointers between inference calls.
    MINIMAL_CHECK_VOID(TfLiteSignatureRunnerSetInputCustomAllocation(runner, name.c_str(), &allocation));
    MINIMAL_CHECK_VOID(TfLiteSignatureRunnerSetInputCustomAllocation(runner, name.c_str(), &allocation));
  }
  MINIMAL_CHECK_VOID(TfLiteSignatureRunnerAllocateTensors(runner));
}

TfLiteSignatureRunner *LLMPipeline::GetPrefillRunner(TfLiteInterpreter* interpreter, std::size_t num_input_tokens, kv_cache_t& kv_cache) {
  // Find the prefill signature length that best matches the input token size.
  TfLiteSignatureRunner* runner = nullptr;
  //int best_seq_size = -1;
  size_t delta = std::numeric_limits<size_t>::max();
  for (int32_t i = 0; i < TfLiteInterpreterGetSignatureCount(interpreter); i++) {
    std::string key (TfLiteInterpreterGetSignatureKey(interpreter, i));
    if (key.find("prefill") == std::string::npos) continue;
    TfLiteTensor* input_pos = TfLiteSignatureRunnerGetInputTensor(TfLiteInterpreterGetSignatureRunner(interpreter, key.c_str()), "input_pos");
    // The expected shape for input position is [Seq].
    size_t seq_size = input_pos->dims->data[0];
    if (num_input_tokens <= seq_size && seq_size - num_input_tokens < delta) {
      runner = TfLiteInterpreterGetSignatureRunner(interpreter, key.c_str());
      //best_seq_size = seq_size;
      delta = seq_size - num_input_tokens;
    }
  }
  MINIMAL_CHECK_PTR(runner != nullptr);
  PrepareRunner(runner, kv_cache);
  return runner;
}

TfLiteSignatureRunner *LLMPipeline::GetDecodeRunner(TfLiteInterpreter* interpreter, kv_cache_t& kv_cache) {
  TfLiteSignatureRunner* runner = TfLiteInterpreterGetSignatureRunner(interpreter, "decode");
  MINIMAL_CHECK_PTR(runner != nullptr);
  PrepareRunner(runner, kv_cache);
  return runner;
}

sentencepiece::SentencePieceProcessor *LLMPipeline::LoadSentencePieceProcessor(std::string path) {
  std::ifstream input(path, std::ios::binary);
  std::string serialized_proto = std::string(
      std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
  auto processor = new sentencepiece::SentencePieceProcessor();
  MINIMAL_CHECK_PTR(processor->LoadFromSerializedProto(serialized_proto).ok());
  return processor;
}

// A basic greedy sampler (equivalent to argmax).
int LLMPipeline::GreedySampler(const TfLiteTensor* logits) {
  float max_value = -std::numeric_limits<float>::infinity();
  int max_index = 0;
  // logits shape: [Batch, Seq, Vocab], Dtype: float
  for (int i = 0; i < logits->dims->data[2]; ++i) {
    if (logits->data.f[i] > max_value) {
      max_value = logits->data.f[i];
      max_index = i;
    }
  }
  return max_index;
}

TfLiteRegistration* LLMPipeline::GetGenAIGenerateOp() {
  static tflite::MutableOpResolver resolver;

  tflite::ops::custom::GenAIOpsRegisterer(&resolver);

  const TfLiteRegistration* reg = resolver.FindOp("GEN_AI_GENERATE", /*version=*/1);

  if (!reg) {
    LOG(ERROR) << "Could not find GEN_AI_GENERATE op." << std::endl;
    return nullptr;
  }

  static TfLiteRegistration reg_copy = *reg;
  return &reg_copy;
}

bool LLMPipeline::TfLiteSignatureRunnerSetInputCustomAllocation(TfLiteSignatureRunner* runner, const char* input_name, const TfLiteCustomAllocation* allocation) {
  if (!runner || !input_name || !allocation) return false;
  auto* cpp_runner = reinterpret_cast<tflite::SignatureRunner*>(runner);
  return cpp_runner
             ->SetCustomAllocationForInputTensor(input_name, *allocation) ==
         kTfLiteOk;
}

bool LLMPipeline::TfLiteSignatureRunnerSetOutputCustomAllocation(TfLiteSignatureRunner* runner, const char* output_name, const TfLiteCustomAllocation* allocation) {
  if (!runner || !output_name || !allocation) return false;
  auto* cpp_runner = reinterpret_cast<tflite::SignatureRunner*>(runner);
  return cpp_runner
             ->SetCustomAllocationForOutputTensor(output_name, *allocation) ==
         kTfLiteOk;
}

bool LLMPipeline::TfLiteSignatureRunnerAllocateTensors(TfLiteSignatureRunner* runner) {
  if (!runner) return false;
  return reinterpret_cast<tflite::SignatureRunner*>(runner)->AllocateTensors() == kTfLiteOk;
}

#ifdef __cplusplus
};
#endif  // __cplusplus
