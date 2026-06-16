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

#include "llm_pipeline.h"

#include <cstdlib>
#include <cstring>
#include <fstream>

#if defined(MTK_TFLITE_NEURON_BACKEND) && defined(__ANDROID__)
#include <dlfcn.h>

#include "neuron/APUWareUtilsApi.h"
#endif

#include "absl/log/log.h"
#include "absl/types/span.h"
#include "flutter/cpp/c/type.h"
#include "flutter/cpp/utils.h"
#include "litert/cc/litert_compiled_model.h"
#include "litert/cc/litert_environment.h"
#include "litert/cc/litert_tensor_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

static bool backendExists = false;

static bool CopyKVBuffer(litert::TensorBuffer& src, litert::TensorBuffer& dst,
                         int float_count) {
  std::vector<float> tmp(float_count);
  if (!src.Read<float>(absl::MakeSpan(tmp))) return false;
  return !!dst.Write<float>(absl::MakeConstSpan(tmp));
}

static size_t lookup (const std::unordered_map<std::string, size_t>& map,
                 const std::string& name) {
  auto it = map.find(name);
  if (it == map.end()) return std::numeric_limits<size_t>::max();
  return it->second;
};

static std::unordered_map<std::string, size_t> make_index_map (const std::vector<std::string_view>& names) {
  std::unordered_map<std::string, size_t> map;
  for (size_t i = 0; i < names.size(); ++i)
    map[std::string(names[i])] = i;
  return map;
};

// Destroy the backend pointer and its data.
void LLMPipeline::backend_delete(mlperf_backend_ptr_t backend_ptr) {
  LLMBackendData* backend_data = (LLMBackendData*)backend_ptr;
  if (backend_data) delete backend_data;
  backendExists = false;
}

// Create a new backend and return the pointer to it.
mlperf_backend_ptr_t LLMPipeline::backend_create(
    const char* model_path, mlperf_backend_configuration_t* configs,
    const char* native_lib_path) {
  // Verify only one instance of the backend exists at any time
  if (backendExists) {
    LOG(ERROR) << "Only one backend instance should exist at a time";
    return nullptr;
  }

  LLMBackendData* backend_data = new LLMBackendData();

  if (!BuildCompiledModel(backend_data, model_path)) {
    LOG(ERROR) << "Failed to build CompiledModel from: " << model_path;
    backend_delete(backend_data);
    return nullptr;
  }
  if (!BuildDecodeBuffers(backend_data)) {
    LOG(ERROR) << "Failed to allocate decode buffers";
    backend_delete(backend_data);
    return nullptr;
  }

  LOG(ERROR) << "backend_create: model=" << (backend_data->model ? "ok" : "null")
             << " decode_input_bufs=" << backend_data->decode_input_bufs.size()
             << " num_kv_layers=" << backend_data->num_kv_layers;

  return backend_data;
}

// Vendor name who create this backend.
const char* LLMPipeline::backend_vendor_name(mlperf_backend_ptr_t backend_ptr) {
  LLMBackendData* backend_data = (LLMBackendData*)backend_ptr;
  return backend_data->vendor;
}

const char* LLMPipeline::backend_accelerator_name(
    mlperf_backend_ptr_t backend_ptr) {
  LLMBackendData* backend_data = (LLMBackendData*)backend_ptr;
  return backend_data->accelerator;
}

// Return the name of this backend.
const char* LLMPipeline::backend_name(mlperf_backend_ptr_t backend_ptr) {
  LLMBackendData* backend_data = (LLMBackendData*)backend_ptr;
  return backend_data->name;
}

// Run the prefill inference and at least 1 output token producing decode
// inference. This function exclusively handles the input tokens.
mlperf_status_t LLMPipeline::backend_issue_first_token_query(
    mlperf_backend_ptr_t backend_ptr) {
  LLMBackendData* backend_data = (LLMBackendData*)backend_ptr;

  //ResetPrefillKV(backend_data);

  int max_seq_size = backend_data->prefill_seq_size;

  int kv_cache_max_size = backend_data->kv_cache_max_size;
  int prefill_seq_size = std::min(
      static_cast<int>(backend_data->prompt_tokens.size()), max_seq_size);
  bool prefill_overflow =
      static_cast<int>(backend_data->prompt_tokens.size()) > max_seq_size;
  int overflow_size =
      prefill_overflow
          ? static_cast<int>(backend_data->prompt_tokens.size()) - max_seq_size
          : 0;
  int prefill_amount =
      prefill_overflow ? prefill_seq_size : (prefill_seq_size - 1);
  int decode_tokens = prefill_overflow ? overflow_size - 1 : 1;

  std::vector<int32_t> tokens_data(max_seq_size, 128009);
  std::vector<int32_t> pos_data(max_seq_size);
  for (int i = 0; i < prefill_amount; ++i)
    tokens_data[i] = backend_data->prompt_tokens[i];
  for (int i = 0; i < max_seq_size; ++i) pos_data[i] = i;
  backend_data->prefill_input_bufs[backend_data->prefill_tokens_idx].Write<int32_t>(
      absl::MakeConstSpan(tokens_data));
  backend_data->prefill_input_bufs[backend_data->prefill_pos_idx].Write<int32_t>(
      absl::MakeConstSpan(pos_data));
  if (backend_data->kv_buf_float_count > 0) {
    ResetPrefillKV(backend_data);
  }

  MINIMAL_CHECK(backend_data->model->Run(
      backend_data->current_prefill_sig_idx,
      absl::MakeSpan(backend_data->prefill_input_bufs),
      absl::MakeSpan(backend_data->prefill_output_bufs)));

  // Swap the prefill KV buffers with the decode ones
  TransferKV(backend_data);

  // Run decode once if input fits inside prefill, otherwise decode the rest of
  // the input one by one
  int next_token = backend_data->prompt_tokens[prefill_amount];
  int next_position = prefill_amount;
  for (int i = 0; i < decode_tokens; ++i) {
    std::vector<int32_t> tok{next_token}, pos{next_position};
    backend_data->decode_input_bufs[backend_data->decode_tokens_idx].Write<int32_t>(absl::MakeConstSpan(tok));
    backend_data->decode_input_bufs[backend_data->decode_pos_idx].Write<int32_t>(absl::MakeConstSpan(pos));
    MINIMAL_CHECK(backend_data->model->Run(
        backend_data->decode_sig_idx,
        absl::MakeSpan(backend_data->decode_input_bufs),
        absl::MakeSpan(backend_data->decode_output_bufs)));
    next_token = backend_data->prompt_tokens[++next_position];
    UpdateDecodeKV(backend_data);
  }

  return MLPERF_SUCCESS;
}

// Run the output token producing decode inference.
// This function exclusively takes output tokens to produce more output tokens.
mlperf_status_t LLMPipeline::backend_issue_query(
    mlperf_backend_ptr_t backend_ptr, ft_callback callback, void* context) {
  LLMBackendData* backend_data = (LLMBackendData*)backend_ptr;

  auto check_stop_id = [backend_data](int id) {
    for (int stop_token_id : backend_data->stop_token_ids) {
      if (id == stop_token_id) return true;
    }
    return false;
  };

  backend_issue_first_token_query(backend_ptr);
  callback(context);

  int kv_cache_max_size = backend_data->kv_cache_max_size;
  size_t input_size = backend_data->prompt_tokens.size();

  // Use a manual number for maximum tokens to generate as long as it's not
  // larger than the KV cache can handle. take away 1 from max_output_tokens
  // because backend_issue_first_token_query always generates the first output
  // token.
  int decode_steps = std::min(backend_data->max_output_tokens - 1,
                              kv_cache_max_size - static_cast<int>(input_size));
  MINIMAL_CHECK(decode_steps > 0);

  backend_data->output_tokens.reserve(decode_steps);
  int next_token = GreedySampler(backend_data);
  if (check_stop_id(next_token)) return MLPERF_SUCCESS;
  backend_data->output_tokens.push_back(next_token);
  int next_position = input_size;
  for (int i = 0; i < decode_steps; ++i) {
    std::vector<int32_t> tok{next_token}, pos{next_position};
    backend_data->decode_input_bufs[backend_data->decode_tokens_idx].Write<int32_t>(absl::MakeConstSpan(tok));
    backend_data->decode_input_bufs[backend_data->decode_pos_idx].Write<int32_t>(absl::MakeConstSpan(pos));
    MINIMAL_CHECK(backend_data->model->Run(
        backend_data->decode_sig_idx,
        absl::MakeSpan(backend_data->decode_input_bufs),
        absl::MakeSpan(backend_data->decode_output_bufs)));
    UpdateDecodeKV(backend_data);
    next_token = GreedySampler(backend_data);
    backend_data->output_tokens.push_back(next_token);
    next_position += 1;
    if (check_stop_id(next_token)) break;
  }

  return MLPERF_SUCCESS;
}

// Flush the staged queries immediately.
mlperf_status_t LLMPipeline::backend_flush_queries(
    mlperf_backend_ptr_t backend_ptr) {
  return MLPERF_SUCCESS;
}

// Return the number of inputs of the model.
// 2 inputs need to be provided manually, the tokens themselves. and a token
// count limit. The other inputs are handled by the pipeline
int32_t LLMPipeline::backend_get_input_count(mlperf_backend_ptr_t backend_ptr) {
  return 2;
}

// Return the type of the ith input.
// All inputs are of the type [int32]
mlperf_data_t LLMPipeline::backend_get_input_type(
    mlperf_backend_ptr_t backend_ptr, int32_t i) {
  return mlperf_data_t{mlperf_data_t::Int32, 0};
}

// Set the data for ith input.
// 0: list of input tokens.
// 1: output token count limit.
mlperf_status_t LLMPipeline::backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                               int32_t batch_index, int32_t i,
                                               void* data) {
  LLMBackendData* backend_data = (LLMBackendData*)backend_ptr;
  // Reset the tokens and kv caches from potential previous runs.
  backend_data->output_tokens.clear();

  if (i == 1) {
    backend_data->max_output_tokens = *(reinterpret_cast<int*>(data));
    return MLPERF_SUCCESS;
  }

  backend_data->prompt_tokens = *(reinterpret_cast<std::vector<int>*>(data));

  ResetKV(backend_data);
  size_t effective_prefill_token_size = backend_data->prompt_tokens.size() - 1;
  size_t sig_idx =
      GetSuitablePrefillSignature(backend_data, effective_prefill_token_size);
  if (!BuildPrefillBuffers(backend_data, sig_idx)) return MLPERF_FAILURE;
  if ((int)(effective_prefill_token_size + 1) >
      backend_data->kv_cache_max_size) {
    LOG(ERROR) << "Input size ("
               << std::to_string(effective_prefill_token_size + 1)
               << ") exceeds KV cache limit ("
               << std::to_string(backend_data->kv_cache_max_size) << ")."
               << std::endl;
    return MLPERF_FAILURE;
  }

  return MLPERF_SUCCESS;
}

// Return the number of outputs for the model.
int32_t LLMPipeline::backend_get_output_count(
    mlperf_backend_ptr_t backend_ptr) {
  return 1;  // 0 is the output tokens
}

// Return the type of ith output.
mlperf_data_t LLMPipeline::backend_get_output_type(
    mlperf_backend_ptr_t backend_ptr, int32_t i) {
  return mlperf_data_t{mlperf_data_t::Float32, 0};
}

// Get the data from ith output.
mlperf_status_t LLMPipeline::backend_get_output(
    mlperf_backend_ptr_t backend_ptr, uint32_t batch_index, int32_t i,
    void** data) {
  LLMBackendData* backend_data = (LLMBackendData*)backend_ptr;

  if (i != 0) return MLPERF_FAILURE;

  *data = reinterpret_cast<void*>(&backend_data->output_tokens);
  return MLPERF_SUCCESS;
}

void LLMPipeline::backend_convert_inputs(mlperf_backend_ptr_t backend_ptr,
                                         int bytes, int width, int height,
                                         uint8_t* data) {}

void LLMPipeline::backend_convert_outputs(mlperf_backend_ptr_t backend_ptr,
                                          int bytes, int width, int height,
                                          uint8_t* data) {}

void* LLMPipeline::backend_get_buffer(size_t n) { return ::operator new(n); }

void LLMPipeline::backend_release_buffer(void* p) { ::operator delete(p); }

// TODO for private methods, provide only necessary data

bool LLMPipeline::BuildCompiledModel(LLMBackendData* backend_ptr,
                                     const char* model_path) {
  auto env = litert::Environment::Create({});
  if (!env) {
    LOG(ERROR) << "Environment::Create failed";
    return false;
  }
  backend_ptr->env = std::make_unique<litert::Environment>(std::move(*env));

  auto options = litert::Options::Create();
  options->SetHardwareAccelerators(litert::HwAccelerators::kGpu | litert::HwAccelerators::kCpu);

  auto model =
      litert::CompiledModel::Create(*backend_ptr->env, model_path, *options);
  if (!model) {
    LOG(ERROR) << "CompiledModel::Create failed";
    return false;
  }
  backend_ptr->model =
      std::make_unique<litert::CompiledModel>(std::move(*model));

  auto decode = backend_ptr->model->GetSignatureIndex("decode");
  if (!decode) {
    LOG(ERROR) << "No 'decode' signature";
    return false;
  }
  backend_ptr->decode_sig_idx = *decode;

  static const int kSizes[] = {32, 64, 128, 256, 512, 1024, 2048, 4096};
  for (int seq : kSizes) {
    auto sig_idx =
        backend_ptr->model->GetSignatureIndex("prefill_" + std::to_string(seq));
    if (!sig_idx) continue;
    // auto reqs = backend_ptr->model->GetInputBufferRequirements(*sig_idx, 1);
    // if (!reqs) continue;
    // auto buffer_size = reqs->BufferSize();
    // int actual = static_cast<int>(*buffer_size / sizeof(int32_t));
    backend_ptr->prefill_sigs.push_back(
        {*sig_idx, seq});
  }
  if (backend_ptr->prefill_sigs.empty()) {
    LOG(ERROR) << "No prefill signatures found";
    return false;
  }
  std::sort(backend_ptr->prefill_sigs.begin(), backend_ptr->prefill_sigs.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });
  return true;
}

bool LLMPipeline::BuildDecodeBuffers(LLMBackendData* backend_ptr) {
  auto input_bufs =
      backend_ptr->model->CreateInputBuffers(backend_ptr->decode_sig_idx);
  if (!input_bufs) {
    LOG(ERROR) << "CreateInputBuffers (decode) failed";
    return false;
  }
  backend_ptr->decode_input_bufs = std::move(*input_bufs);

  const auto input_names_exp = backend_ptr->model->GetSignatureInputNames(backend_ptr->decode_sig_idx);
  if (!input_names_exp) {
    LOG(ERROR) << "Couldn't get input names";
    return false;
  }
  const auto& input_names = *input_names_exp;
  auto it = absl::c_find(input_names, "tokens");
  if (it != input_names.end())
    backend_ptr->decode_tokens_idx = std::distance(input_names.begin(), it);
  it = absl::c_find(input_names, "input_pos");
  if (it != input_names.end())
    backend_ptr->decode_pos_idx = std::distance(input_names.begin(), it);

  auto output_bufs =
      backend_ptr->model->CreateOutputBuffers(backend_ptr->decode_sig_idx);
  if (!output_bufs) {
    LOG(ERROR) << "CreateOutputBuffers (decode) failed";
    return false;
  }
  backend_ptr->decode_output_bufs = std::move(*output_bufs);

  const auto output_names_exp = backend_ptr->model->GetSignatureOutputNames(backend_ptr->decode_sig_idx);
  if (!output_names_exp) {
    LOG(ERROR) << "Couldn't get output names";
    return false;
  }
  const auto& output_names = *output_names_exp;
  // it = absl::c_find(output_names, "logits");
  // if (it != output_names.end())
  //   backend_ptr->logits_idx = std::distance(output_names.begin(), it);

  backend_ptr->decode_input_map  = make_index_map(input_names);
  backend_ptr->decode_output_map = make_index_map(output_names);

  // tokens, pos
  backend_ptr->decode_tokens_idx = backend_ptr->decode_input_map.at("tokens");
  backend_ptr->decode_pos_idx    = backend_ptr->decode_input_map.at("input_pos");
  backend_ptr->logits_idx        = backend_ptr->decode_output_map.at("logits");

  // There are 2 KV tensors per layer + tokens and pos tensors.
  // TODO search for "kv_k" instead of using hard coded numbers.
  backend_ptr->num_kv_layers = (backend_ptr->decode_output_map.size() - 1) / 2;
      //(static_cast<int>(backend_ptr->decode_input_bufs.size()) - 2) / 2;

  if (backend_ptr->num_kv_layers > 0) {
    // info on the very first KV_K tensor
    // TODO get index by name instead of hard coding 2
    auto kv_metadata = backend_ptr->model->GetInputBufferRequirements(
        backend_ptr->decode_sig_idx, "kv_cache_k_0");
    if (!kv_metadata) {
      LOG(ERROR) << "GetInputBufferRequirements (KV) failed";
      return false;
    }

    auto kv_type = backend_ptr->model->GetInputTensorType(backend_ptr->decode_sig_idx, "kv_cache_k_0");
    if (!kv_type) {
      LOG(ERROR) << "RankedTensorType failed";
      return false;
    }
    backend_ptr->kv_cache_max_size = (*kv_type).Layout().Dimensions()[1];

    auto buffer_size = kv_metadata->BufferSize();
    backend_ptr->kv_buf_float_count =
        static_cast<int>(*buffer_size / sizeof(float));
  }

  auto logits_metadata = backend_ptr->model->GetOutputBufferRequirements(
      backend_ptr->decode_sig_idx, "logits");
  if (!logits_metadata) {
    LOG(ERROR) << "GetOutputBufferRequirements (logits) failed";
    return false;
  }

  auto buffer_size = logits_metadata->BufferSize();
  backend_ptr->vocab_size =
      static_cast<int>(*buffer_size / sizeof(float));
  backend_ptr->logits_scratch.resize(backend_ptr->vocab_size);
  return true;
}

bool LLMPipeline::BuildPrefillBuffers(LLMBackendData* backend_ptr,
                                      size_t prefill_sig_idx) {
  if (backend_ptr->current_prefill_sig_idx == prefill_sig_idx) return true;

  auto input_bufs = backend_ptr->model->CreateInputBuffers(prefill_sig_idx);
  if (!input_bufs) {
    LOG(ERROR) << "CreateInputBuffers (prefill) failed";
    return false;
  }
  backend_ptr->prefill_input_bufs = std::move(*input_bufs);

  // get input indices
  const auto input_names_exp = backend_ptr->model->GetSignatureInputNames(prefill_sig_idx);
  if (!input_names_exp) {
    LOG(ERROR) << "Couldn't get input names";
    return false;
  }
  const auto& input_names = *input_names_exp;

  const auto output_names_exp = backend_ptr->model->GetSignatureOutputNames(prefill_sig_idx);
  if (!output_names_exp) {
    LOG(ERROR) << "Couldn't get input names";
    return false;
  }
  const auto& output_names = *output_names_exp;

  auto it = absl::c_find(input_names, "tokens");
  if (it != input_names.end())
    backend_ptr->prefill_tokens_idx = std::distance(input_names.begin(), it);
  it = absl::c_find(input_names, "input_pos");
  if (it != input_names.end())
    backend_ptr->prefill_pos_idx = std::distance(input_names.begin(), it);

  auto output_bufs = backend_ptr->model->CreateOutputBuffers(prefill_sig_idx);
  if (!output_bufs) {
    LOG(ERROR) << "CreateOutputBuffers (prefill) failed";
    return false;
  }
  backend_ptr->prefill_output_bufs = std::move(*output_bufs);

  backend_ptr->prefill_input_map  = make_index_map(input_names);
  backend_ptr->prefill_output_map = make_index_map(output_names);

  backend_ptr->prefill_tokens_idx = backend_ptr->prefill_input_map.at("tokens");
  backend_ptr->prefill_pos_idx    = backend_ptr->prefill_input_map.at("input_pos");

  auto reqs =
      backend_ptr->model->GetInputBufferRequirements(prefill_sig_idx, "tokens");
  if (reqs){
    auto buffer_size = reqs->BufferSize();
    backend_ptr->prefill_seq_size =
        static_cast<int>(*buffer_size / sizeof(int32_t));
  }

  backend_ptr->current_prefill_sig_idx = prefill_sig_idx;
  return true;
}

size_t LLMPipeline::GetSuitablePrefillSignature(LLMBackendData* backend_ptr,
                                                size_t num_input_tokens) const {
  size_t best = backend_ptr->prefill_sigs.back().first;
  size_t delta = std::numeric_limits<size_t>::max();
  for (const auto& [sig_idx, seq_size] : backend_ptr->prefill_sigs) {
    if (seq_size >= num_input_tokens && seq_size - num_input_tokens < delta) {
      delta = seq_size - num_input_tokens;
      best = sig_idx;
    }
  }
  return best;
}

void LLMPipeline::TransferKV(LLMBackendData* backend_ptr) {
  for (int i = 0; i < backend_ptr->num_kv_layers; ++i) {
    for (const char* prefix : {"kv_cache_k_", "kv_cache_v_"}) {
      std::string name = std::string(prefix) + std::to_string(i);
      size_t prefill_out_idx = backend_ptr->prefill_output_map.at(name);
      size_t decode_in_idx   = backend_ptr->decode_input_map.at(name);
      std::swap(backend_ptr->prefill_output_bufs[prefill_out_idx],
                backend_ptr->decode_input_bufs[decode_in_idx]);
    }
  }
}

void LLMPipeline::ResetKV(LLMBackendData* backend_ptr) {
  if (backend_ptr->kv_buf_float_count == 0) return;
  std::vector<float> zeros(backend_ptr->kv_buf_float_count, 0.0f);
  for (int i = 0; i < backend_ptr->num_kv_layers; ++i) {
    for (const char* prefix : {"kv_cache_k_", "kv_cache_v_"}) {
      std::string name = std::string(prefix) + std::to_string(i);
      backend_ptr->decode_input_bufs[backend_ptr->decode_input_map.at(name)]
          .Write<float>(absl::MakeConstSpan(zeros));
    }
  }
}

void LLMPipeline::ResetPrefillKV(LLMBackendData* backend_ptr) {
  if (backend_ptr->kv_buf_float_count == 0) return;
  std::vector<float> zeros(backend_ptr->kv_buf_float_count, 0.0f);
  for (int i = 0; i < backend_ptr->num_kv_layers; ++i) {
    for (const char* prefix : {"kv_cache_k_", "kv_cache_v_"}) {
      std::string name = std::string(prefix) + std::to_string(i);
      backend_ptr->prefill_input_bufs[backend_ptr->prefill_input_map.at(name)]
          .Write<float>(absl::MakeConstSpan(zeros));
    }
  }
}

void LLMPipeline::UpdateDecodeKV(LLMBackendData* backend_ptr) {
  for (int i = 0; i < backend_ptr->num_kv_layers; ++i) {
    for (const char* prefix : {"kv_cache_k_", "kv_cache_v_"}) {
      std::string name = std::string(prefix) + std::to_string(i);
      std::swap(
          backend_ptr->decode_input_bufs[backend_ptr->decode_input_map.at(name)],
          backend_ptr->decode_output_bufs[backend_ptr->decode_output_map.at(name)]);
    }
  }
}

// A basic greedy sampler (equivalent to argmax).
int LLMPipeline::GreedySampler(LLMBackendData* backend_ptr) {
  std::vector<float> tmp(backend_ptr->vocab_size);
  auto span = absl::MakeSpan(tmp);
  auto ok = backend_ptr->decode_output_bufs[backend_ptr->logits_idx].Read<float>(span);
  auto meta = backend_ptr->model->GetOutputBufferRequirements(
      backend_ptr->decode_sig_idx, backend_ptr->logits_idx);
  if (meta)
    LOG(ERROR) << "logits buffer size=" << meta->BufferSize()
               << " expected=" << backend_ptr->vocab_size * sizeof(float);
  if (!ok) {
    LOG(ERROR) << "Failed to read logits" << " " << ok.Error().Message();
    return 0;
  }
  float max_value = -std::numeric_limits<float>::infinity();
  int max_index = 5;
  for (int i = 0; i < backend_ptr->vocab_size; ++i) {
    if (tmp[i] > max_value) {
      max_value = tmp[i];
      max_index = i;
    }
  }
  return max_index;
}

#ifdef __cplusplus
};
#endif  // __cplusplus
