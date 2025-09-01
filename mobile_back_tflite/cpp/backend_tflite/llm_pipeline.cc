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

#include "tensorflow/core/platform/logging.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

static bool backendExists = false;

// Destroy the backend pointer and its data.
void LLMPipeline::backend_delete(mlperf_backend_ptr_t backend_ptr) {
  LLMBackendData *backend_data = (LLMBackendData *)backend_ptr;
  if (backend_data) delete backend_data;
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
  backend_data->model = tflite::FlatBufferModel::BuildFromFile(model_path).release();
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

  if (!backend_data->end_token.empty()) {
    backend_data->stop_token_id = backend_data->sp_processor->PieceToId((backend_data->end_token));
  }

  return backend_data;
}

// Vendor name who create this backend.
const char *LLMPipeline::backend_vendor_name(mlperf_backend_ptr_t backend_ptr) {
  LLMBackendData *backend_data = (LLMBackendData *)backend_ptr;
  return backend_data->vendor;
}

const char *LLMPipeline::backend_accelerator_name(mlperf_backend_ptr_t backend_ptr) {
  LLMBackendData *backend_data = (LLMBackendData *)backend_ptr;
  return backend_data->accelerator;
}

// Return the name of this backend.
const char *LLMPipeline::backend_name(mlperf_backend_ptr_t backend_ptr) {
  LLMBackendData *backend_data = (LLMBackendData *)backend_ptr;
  return backend_data->name;
}

// TODO chunked prefill support
// Run the prefill inference and at least 1 output token producing decode inference.
// This function exclusively handles the input tokens.
mlperf_status_t LLMPipeline::backend_issue_first_token_query(mlperf_backend_ptr_t backend_ptr) {
  LLMBackendData *backend_data = (LLMBackendData *)backend_ptr;

  // Get Input Tensors for each of the runners.
  // Shape: [Batch, Seq], Dtype: int32
  TfLiteTensor* prefill_input = backend_data->prefill_runner->input_tensor("tokens");
  // Shape: [Seq], Dtype: int32
  TfLiteTensor* prefill_input_pos = backend_data->prefill_runner->input_tensor("input_pos");
  // Shape: [Batch, Seq], Dtype: int32
  TfLiteTensor* decode_input = backend_data->decode_runner->input_tensor("tokens");
  // Shape: [Seq], Dtype: int32
  TfLiteTensor* decode_input_pos = backend_data->decode_runner->input_tensor("input_pos");
  // shape: [Batch, kv_cache_max, num_query_groups, head_dim]
  TfLiteTensor* kv_cache_k_0 = backend_data->decode_runner->input_tensor("kv_cache_k_0");

  int max_seq_size = prefill_input->dims->data[1];
  int kv_cache_max_size = kv_cache_k_0->dims->data[1];
  int prefill_seq_size = std::min(static_cast<int>(backend_data->prompt_tokens.size()), max_seq_size);
  bool prefill_overflow = static_cast<int>(backend_data->prompt_tokens.size()) > max_seq_size;
  int overflow_size = prefill_overflow ? static_cast<int>(backend_data->prompt_tokens.size()) - max_seq_size : 0;
  int prefill_amount = prefill_overflow ? prefill_seq_size : (prefill_seq_size - 1);
  int decode_tokens = prefill_overflow ? overflow_size - 1 : 1;

  std::memset(prefill_input->data.i32, 0, prefill_input->bytes);
  std::memset(prefill_input_pos->data.i32, 0, prefill_input_pos->bytes);
  // If the prefill can fit the entire input, leave one token for decode, otherwise prefill as much of the input as possible.
  for (int i = 0; i < prefill_amount; ++i) {
    prefill_input->data.i32[i] = backend_data->prompt_tokens[i];
    prefill_input_pos->data.i32[i] = i;
  }

  MINIMAL_CHECK(backend_data->prefill_runner->Invoke() == kTfLiteOk);

  // Run decode once if input fits inside prefill, otherwise decode the rest of the input one by one
  int next_token = backend_data->prompt_tokens[prefill_amount];
  int next_position = prefill_amount;
  for (int i = 0; i < decode_tokens; ++i) {
    decode_input->data.i32[0] = next_token;
    decode_input_pos->data.i32[0] = next_position;
    MINIMAL_CHECK(backend_data->decode_runner->Invoke() == kTfLiteOk);
    next_token = backend_data->prompt_tokens[++next_position];
  }

  return MLPERF_SUCCESS;
}

// Run the output token producing decode inference.
// This function exclusively takes output tokens to produce more output tokens.
mlperf_status_t LLMPipeline::backend_issue_query(mlperf_backend_ptr_t backend_ptr) {
  LLMBackendData *backend_data = (LLMBackendData *)backend_ptr;

  // Get Input Tensors for each of the runners.
  // Shape: [Batch, Seq], Dtype: int32
  TfLiteTensor* prefill_input = backend_data->prefill_runner->input_tensor("tokens");
  // Shape: [Seq], Dtype: int32
  TfLiteTensor* prefill_input_pos = backend_data->prefill_runner->input_tensor("input_pos");
  // Shape: [Batch, Seq], Dtype: int32
  TfLiteTensor* decode_input = backend_data->decode_runner->input_tensor("tokens");
  // Shape: [Seq], Dtype: int32
  TfLiteTensor* decode_input_pos = backend_data->decode_runner->input_tensor("input_pos");
  // shape: [Batch, kv_cache_max, num_query_groups, head_dim]
  TfLiteTensor* kv_cache_k_0 = backend_data->decode_runner->input_tensor("kv_cache_k_0");

  int kv_cache_max_size = kv_cache_k_0->dims->data[1];
  size_t input_size = backend_data->prompt_tokens.size();

  // Use a manual number for maximum tokens to generate as long as it's not larger than the KV cache can handle
  int decode_steps = std::min(backend_data->max_output_tokens, kv_cache_max_size - static_cast<int>(input_size));
  MINIMAL_CHECK(decode_steps > 0);

  backend_data->output_tokens.reserve(decode_steps);
  int next_token = GreedySampler(backend_data->decode_runner->output_tensor("logits"));
  if (next_token == backend_data->stop_token_id) return MLPERF_SUCCESS;
  backend_data->output_tokens.push_back(next_token);
  int next_position = input_size;
  for (int i = 0; i < decode_steps; ++i) {
    decode_input->data.i32[0] = next_token;
    decode_input_pos->data.i32[0] = next_position;
    MINIMAL_CHECK(backend_data->decode_runner->Invoke() == kTfLiteOk);
    next_token = GreedySampler(backend_data->decode_runner->output_tensor("logits"));
    if (next_token == backend_data->stop_token_id) break;
    backend_data->output_tokens.push_back(next_token);
    next_position += 1;
  }

  return MLPERF_SUCCESS;
}

// Flush the staged queries immediately.
mlperf_status_t LLMPipeline::backend_flush_queries(mlperf_backend_ptr_t backend_ptr) {
  LLMBackendData *backend_data = (LLMBackendData *)backend_ptr;

  backend_data->prompt_tokens.clear();
  backend_data->output_tokens.clear();

  for (auto& [_, vec]: backend_data->kv_cache) {
    std::fill(vec.begin(), vec.end(), 0.0f);
  }

  return MLPERF_SUCCESS;
}

// Return the number of inputs of the model.
// Only 1 input needs to be provided, which is the tokens themselves, the other inputs are handled by the pipeline
int32_t LLMPipeline::backend_get_input_count(mlperf_backend_ptr_t backend_ptr) {
  return 1;
}

// Return the type of the ith input.
mlperf_data_t LLMPipeline::backend_get_input_type(mlperf_backend_ptr_t backend_ptr, int32_t i) {
  return mlperf_data_t{mlperf_data_t::Int32, 0};
}

// Set the data for ith input.
mlperf_status_t LLMPipeline::backend_set_input(mlperf_backend_ptr_t backend_ptr, int32_t batch_index, int32_t i, void *data) {
  LLMBackendData *backend_data = (LLMBackendData *)backend_ptr;

  std::string prompt = std::string(static_cast<char*>(data));


  LOG(INFO) << "Input: " << prompt << std::endl;
  MINIMAL_CHECK(backend_data->sp_processor->Encode(prompt, &backend_data->prompt_tokens).ok());

  if (!backend_data->start_token.empty()) {
    backend_data->prompt_tokens.insert(backend_data->prompt_tokens.begin(), backend_data->sp_processor->PieceToId((backend_data->start_token)));
  }

  uint16_t effective_prefill_token_size = backend_data->prompt_tokens.size() - 1; //assuming max tokens is <16k

  backend_data->prefill_runner = GetPrefillRunner(backend_data->interpreter, effective_prefill_token_size, backend_data->kv_cache);


  return MLPERF_SUCCESS;
}

// Return the number of outputs for the model.
int32_t LLMPipeline::backend_get_output_count(mlperf_backend_ptr_t backend_ptr) {
  return 2; // 0 is the output string, 1 is the output tokens
}

// Return the type of ith output.
mlperf_data_t LLMPipeline::backend_get_output_type(mlperf_backend_ptr_t backend_ptr, int32_t i) {
  return mlperf_data_t{mlperf_data_t::Float32, 0};
}

// Get the data from ith output.
mlperf_status_t LLMPipeline::backend_get_output(mlperf_backend_ptr_t backend_ptr, uint32_t batch_index, int32_t i, void **data) {
  LLMBackendData *backend_data = (LLMBackendData *)backend_ptr;

  if (i == 0){
    MINIMAL_CHECK(backend_data->sp_processor->Decode(backend_data->output_tokens, &backend_data->output).ok());
    LOG(INFO) << "Output: " << backend_data->output << std::endl;

    *data = backend_data->output.data();
  }
  else if (i == 1) {
    *data = &backend_data->output_tokens;
  }
  else return MLPERF_FAILURE;
  return MLPERF_SUCCESS;
}

void LLMPipeline::backend_convert_inputs(mlperf_backend_ptr_t backend_ptr, int bytes, int width, int height, uint8_t *data) {}

void LLMPipeline::backend_convert_outputs(mlperf_backend_ptr_t backend_ptr, int bytes, int width, int height, uint8_t *data) {}

void *LLMPipeline::backend_get_buffer(size_t n) {
  return ::operator new(n);
}

void LLMPipeline::backend_release_buffer(void *p) {
  ::operator delete(p);
}

tflite::Interpreter *LLMPipeline::BuildInterpreter(tflite::FlatBufferModel *model, int num_threads) {
  tflite::ops::builtin::BuiltinOpResolver resolver;
  // NOTE: We need to manually register optimized OPs for KV-cache and
  // Scaled Dot Product Attention (SDPA).
  tflite::ops::custom::GenAIOpsRegisterer(&resolver);
  tflite::InterpreterBuilder builder(*model, resolver);
  MINIMAL_CHECK_PTR(builder.SetNumThreads(num_threads) == kTfLiteOk);
  std::unique_ptr<tflite::Interpreter> interpreter;
  builder(&interpreter);

  MINIMAL_CHECK_PTR(interpreter != nullptr);

  return interpreter.release();
}

kv_cache_t LLMPipeline::BuildKVCache(tflite::Interpreter* interpreter) {
  tflite::SignatureRunner* runner = interpreter->GetSignatureRunner("decode");
  if (runner == nullptr) {
    return {};
  }
  // The two arguments excluded are `tokens` and `input_pos`.
  // TODO more arguments might need to be excluded
  size_t num_layers = (runner->input_size() - 2) / 2;
  if (num_layers == 0) {
    return {};
  }

  kv_cache_t kv_cache;
  for (int i = 0; i < num_layers; ++i) {
    std::string k_cache_name = "kv_cache_k_" + std::to_string(i);
    std::string v_cache_name = "kv_cache_v_" + std::to_string(i);
    // We are assuming K and V tensors are of the same shape.
    TfLiteTensor* tensor = runner->input_tensor(k_cache_name.c_str());
    size_t count = tensor->bytes / sizeof(float);
    kv_cache.emplace(k_cache_name,
                     std::vector<float, AlignedAllocator<float>>(count, 0.0f));
    kv_cache.emplace(v_cache_name,
                     std::vector<float, AlignedAllocator<float>>(count, 0.0f));
  }

  return kv_cache;
}

void LLMPipeline::PrepareRunner(tflite::SignatureRunner* runner, kv_cache_t& kv_cache) {
  for (auto& [name, cache] : kv_cache) {
    TfLiteCustomAllocation allocation = {.data = static_cast<void*>(cache.data()), .bytes = cache.size() * sizeof(float)};
    // Both input and output tensors are set to the same buffer. Not all
    // delegates support this in-place update. For those cases, we need to do
    // a ping-pong buffer and update the pointers between inference calls.
    MINIMAL_CHECK_VOID(runner->SetCustomAllocationForInputTensor(name.c_str(), allocation) == kTfLiteOk);
    MINIMAL_CHECK_VOID(runner->SetCustomAllocationForOutputTensor(name.c_str(), allocation) == kTfLiteOk);
  }
  MINIMAL_CHECK_VOID(runner->AllocateTensors() == kTfLiteOk);
}

tflite::SignatureRunner *LLMPipeline::GetPrefillRunner(tflite::Interpreter* interpreter, std::size_t num_input_tokens, kv_cache_t& kv_cache) {
  // Find the prefill signature length that best matches the input token size.
  tflite::SignatureRunner* runner = nullptr;
  //int best_seq_size = -1;
  size_t delta = std::numeric_limits<size_t>::max();
  size_t max_prefill_size = 0;
  std::string max_prefill_key = std::string("");
  for (const std::string* key : interpreter->signature_keys()) {
    if (key->find("prefill") == std::string::npos) continue;
    TfLiteTensor* input_pos = interpreter->GetSignatureRunner(key->c_str())->input_tensor("input_pos");
    // The expected shape for input position is [Seq].
    size_t seq_size = input_pos->dims->data[0];
    //TODO this could be else maybe?
    if (seq_size > max_prefill_size) {
      max_prefill_size = seq_size;
      max_prefill_key = std::string(key->c_str());
    }
    if (num_input_tokens <= seq_size && seq_size - num_input_tokens < delta) {
      runner = interpreter->GetSignatureRunner(key->c_str());
      //best_seq_size = seq_size;
      delta = seq_size - num_input_tokens;
    }
  }
  //fallback to maximum possible size if a runner is not found (most likely because the seq_size is larger than max_prefill_size)
  if (!runner && max_prefill_key != "") runner = interpreter->GetSignatureRunner(max_prefill_key.c_str());
  MINIMAL_CHECK_PTR(runner != nullptr);
  PrepareRunner(runner, kv_cache);
  return runner;
}

tflite::SignatureRunner *LLMPipeline::GetDecodeRunner(tflite::Interpreter* interpreter, kv_cache_t& kv_cache) {
  tflite::SignatureRunner* runner = interpreter->GetSignatureRunner("decode");
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

#ifdef __cplusplus
};
#endif  // __cplusplus
