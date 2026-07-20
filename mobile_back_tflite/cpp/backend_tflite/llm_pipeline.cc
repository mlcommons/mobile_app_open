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

#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>

#if defined(MTK_TFLITE_NEURON_BACKEND) && defined(__ANDROID__)
#include <dlfcn.h>

#include "neuron/APUWareUtilsApi.h"
#endif

#include "absl/log/log.h"
#include "absl/types/span.h"
#include "flutter/cpp/c/type.h"
#include "flutter/cpp/utils.h"
#include "litert/cc/litert_compiled_model.h"
#include "litert/cc/litert_element_type.h"
#include "litert/cc/litert_environment.h"
#include "litert/cc/litert_options.h"
#include "litert/cc/litert_tensor_buffer.h"
#include "litert/cc/options/litert_gpu_options.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

static bool backendExists = false;

// Fill value for masked attention positions. Must be finite, not -inf: the
// WebGPU softmax overflows on -inf. Matches LiteRT-LM's -0.7 * FLT_MAX for
// float32. Allowed positions are filled with 0.
static constexpr float kMaskedValue = -0.7f * std::numeric_limits<float>::max();

static std::unordered_map<std::string, size_t> make_index_map(
    const std::vector<std::string_view>& names) {
  std::unordered_map<std::string, size_t> map;
  for (size_t i = 0; i < names.size(); ++i) map[std::string(names[i])] = i;
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

  if (!BuildCompiledModel(*backend_data, model_path)) {
    LOG(ERROR) << "Failed to build CompiledModel from: " << model_path;
    backend_delete(backend_data);
    return nullptr;
  }
  if (!BuildDecodeBuffers(*backend_data)) {
    LOG(ERROR) << "Failed to allocate decode buffers";
    backend_delete(backend_data);
    return nullptr;
  }

  backendExists = true;
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

  int max_seq_size = backend_data->prefill_seq_size;
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

  backend_data->prefill_input_bufs[backend_data->prefill_tokens_idx]
      .Write<int32_t>(absl::MakeConstSpan(tokens_data));
  backend_data->prefill_input_bufs[backend_data->prefill_pos_idx]
      .Write<int32_t>(absl::MakeConstSpan(pos_data));
  ResetPrefillKV(backend_data->num_kv_layers, backend_data->kv_buf_float_count,
                 backend_data->prefill_input_map,
                 backend_data->prefill_input_bufs);
  if (backend_data->has_mask_input) {
    WritePrefillMask(
        *backend_data->model, backend_data->current_prefill_sig_idx,
        backend_data->prefill_mask_idx, backend_data->mask_is_bool,
        backend_data->prefill_input_bufs[backend_data->prefill_mask_idx]);
  }

  MINIMAL_CHECK(backend_data->model->Run(
      backend_data->current_prefill_sig_idx,
      absl::MakeSpan(backend_data->prefill_input_bufs),
      absl::MakeSpan(backend_data->prefill_output_bufs)));

  // Move the prefill KV state into the decode buffers.
  TransferKV(backend_data->num_kv_layers, backend_data->prefill_output_map,
             backend_data->decode_input_map, backend_data->prefill_output_bufs,
             backend_data->decode_input_bufs);

  // Decode once if the input fit inside prefill, otherwise decode the
  // remaining input tokens one by one.
  int next_token = backend_data->prompt_tokens[prefill_amount];
  int next_position = prefill_amount;
  for (int i = 0; i < decode_tokens; ++i) {
    std::vector<int32_t> tok{next_token}, pos{next_position};
    backend_data->decode_input_bufs[backend_data->decode_tokens_idx]
        .Write<int32_t>(absl::MakeConstSpan(tok));
    backend_data->decode_input_bufs[backend_data->decode_pos_idx]
        .Write<int32_t>(absl::MakeConstSpan(pos));
    if (backend_data->has_mask_input) {
      WriteDecodeMask(
          *backend_data->model, backend_data->decode_sig_idx,
          backend_data->decode_mask_idx, backend_data->mask_is_bool,
          backend_data->decode_input_bufs[backend_data->decode_mask_idx],
          next_position);
    }
    MINIMAL_CHECK(backend_data->model->Run(
        backend_data->decode_sig_idx,
        absl::MakeSpan(backend_data->decode_input_bufs),
        absl::MakeSpan(backend_data->decode_output_bufs)));
    next_token = backend_data->prompt_tokens[++next_position];
    UpdateDecodeKV(backend_data->num_kv_layers, backend_data->decode_input_map,
                   backend_data->decode_output_map,
                   backend_data->decode_input_bufs,
                   backend_data->decode_output_bufs);
  }

  return MLPERF_SUCCESS;
}

// Decode output tokens until a stop token or the token/KV limit is hit.
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

  // Cap generation by max_output_tokens and by remaining KV cache room.
  // Subtract 1 because backend_issue_first_token_query already produced the
  // first output token.
  int decode_steps = std::min(backend_data->max_output_tokens - 1,
                              kv_cache_max_size - static_cast<int>(input_size));
  MINIMAL_CHECK(decode_steps > 0);

  backend_data->output_tokens.reserve(decode_steps);
  int next_token =
      GreedySampler(backend_data->decode_output_bufs[backend_data->logits_idx],
                    backend_data->vocab_size);
  if (check_stop_id(next_token)) return MLPERF_SUCCESS;
  backend_data->output_tokens.push_back(next_token);
  int next_position = input_size;
  for (int i = 0; i < decode_steps; ++i) {
    std::vector<int32_t> tok{next_token}, pos{next_position};
    backend_data->decode_input_bufs[backend_data->decode_tokens_idx]
        .Write<int32_t>(absl::MakeConstSpan(tok));
    backend_data->decode_input_bufs[backend_data->decode_pos_idx]
        .Write<int32_t>(absl::MakeConstSpan(pos));
    if (backend_data->has_mask_input) {
      WriteDecodeMask(
          *backend_data->model, backend_data->decode_sig_idx,
          backend_data->decode_mask_idx, backend_data->mask_is_bool,
          backend_data->decode_input_bufs[backend_data->decode_mask_idx],
          next_position);
    }
    MINIMAL_CHECK(backend_data->model->Run(
        backend_data->decode_sig_idx,
        absl::MakeSpan(backend_data->decode_input_bufs),
        absl::MakeSpan(backend_data->decode_output_bufs)));
    UpdateDecodeKV(backend_data->num_kv_layers, backend_data->decode_input_map,
                   backend_data->decode_output_map,
                   backend_data->decode_input_bufs,
                   backend_data->decode_output_bufs);
    next_token = GreedySampler(
        backend_data->decode_output_bufs[backend_data->logits_idx],
        backend_data->vocab_size);
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

  ResetKV(backend_data->num_kv_layers, backend_data->kv_buf_float_count,
          backend_data->decode_input_map, backend_data->decode_input_bufs);
  size_t effective_prefill_token_size = backend_data->prompt_tokens.size() - 1;
  size_t sig_idx = GetSuitablePrefillSignature(backend_data->prefill_sigs,
                                               effective_prefill_token_size);
  if (!BuildPrefillBuffers(*backend_data, sig_idx)) return MLPERF_FAILURE;
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

bool LLMPipeline::BuildCompiledModel(LLMBackendData& data,
                                     const char* model_path) {
  auto env = litert::Environment::Create({});
  if (!env) {
    LOG(ERROR) << "Environment::Create failed";
    return false;
  }
  data.env = std::make_unique<litert::Environment>(std::move(*env));

  auto options = litert::Options::Create();
  if (!options) {
    LOG(ERROR) << "Options::Create failed";
    return false;
  }
  options->SetHardwareAccelerators(litert::HwAccelerators::kGpu |
                                   litert::HwAccelerators::kCpu);

  // GPU compile options mirroring LiteRT-LM's CreateCompilationOptions
  // (llm_executor_settings_utils.cc). With empty options the prebuilt WebGPU
  // delegate takes default paths that crash during kernel init on this graph.
  auto gpu_options = options->GetGpuOptions();
  if (gpu_options) {
    gpu_options->EnableInfiniteFloatCapping(true);
    gpu_options->SetPrecision(litert::GpuOptions::Precision::kFp16);
    gpu_options->SetBufferStorageType(
        litert::GpuOptions::BufferStorageType::kBuffer);
    gpu_options->SetPreferTextureWeights(false);
    // Keep KV caches external so the input/output swap isn't mangled by BHWC
    // conversion.
    gpu_options->EnableExternalTensorsMode(false);
    gpu_options->AddExternalTensorPattern("kv_cache_");
    // Prefill and decode must each land in a single delegate partition.
    gpu_options->SetHintFullyDelegatedToSingleDelegate(true);
    gpu_options->SetMadviseOriginalSharedTensors(false);
    gpu_options->SetConvertWeightsOnGpu(false);
    gpu_options->EnableConstantTensorSharing(false);
    gpu_options->EnableAllowSrcQuantizedFcConvOps(true);
    // KV cache is swapped, so GPU bindings repeat every 2 steps.
    gpu_options->SetNumStepsOfCommandBufferPreparations(2);
    gpu_options->SetNumThreadsToUpload(2);
    gpu_options->SetNumThreadsToCompile(1);
#if defined(LITERT_USE_WEBGPU_ACCELERATOR)
    gpu_options->SetBackend(litert::GpuOptions::Backend::kWebGpu);
#endif  // defined(LITERT_USE_WEBGPU_ACCELERATOR)
  } else {
    LOG(ERROR) << "GetGpuOptions failed; GPU compile may crash";
  }

  std::string transformer_path = model_path;
  auto model =
      litert::CompiledModel::Create(*data.env, transformer_path, *options);
  if (!model) {
    LOG(ERROR) << "CompiledModel::Create failed: " << transformer_path;
    return false;
  }
  data.model = std::make_unique<litert::CompiledModel>(std::move(*model));

  auto decode = data.model->GetSignatureIndex("decode");
  if (!decode) {
    LOG(ERROR) << "No 'decode' signature";
    return false;
  }
  data.decode_sig_idx = *decode;

  static const int kSizes[] = {32, 64, 128, 256, 512, 1024, 2048, 4096};
  for (int seq : kSizes) {
    auto sig_idx =
        data.model->GetSignatureIndex("prefill_" + std::to_string(seq));
    if (!sig_idx) continue;
    data.prefill_sigs.push_back({*sig_idx, seq});
  }
  if (data.prefill_sigs.empty()) {
    LOG(ERROR) << "No prefill signatures found";
    return false;
  }
  std::sort(data.prefill_sigs.begin(), data.prefill_sigs.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });

  return true;
}

bool LLMPipeline::BuildDecodeBuffers(LLMBackendData& data) {
  auto input_bufs = data.model->CreateInputBuffers(data.decode_sig_idx);
  if (!input_bufs) {
    LOG(ERROR) << "CreateInputBuffers (decode) failed";
    return false;
  }
  data.decode_input_bufs = std::move(*input_bufs);

  const auto input_names_exp =
      data.model->GetSignatureInputNames(data.decode_sig_idx);
  if (!input_names_exp) {
    LOG(ERROR) << "Couldn't get input names";
    return false;
  }
  const auto& input_names = *input_names_exp;

  auto output_bufs = data.model->CreateOutputBuffers(data.decode_sig_idx);
  if (!output_bufs) {
    LOG(ERROR) << "CreateOutputBuffers (decode) failed";
    return false;
  }
  data.decode_output_bufs = std::move(*output_bufs);

  const auto output_names_exp =
      data.model->GetSignatureOutputNames(data.decode_sig_idx);
  if (!output_names_exp) {
    LOG(ERROR) << "Couldn't get output names";
    return false;
  }
  const auto& output_names = *output_names_exp;

  data.decode_input_map = make_index_map(input_names);
  data.decode_output_map = make_index_map(output_names);

  data.decode_tokens_idx = data.decode_input_map.at("tokens");
  data.decode_pos_idx = data.decode_input_map.at("input_pos");
  data.logits_idx = data.decode_output_map.at("logits");

  // Optional attention-mask input (model exported with a mask input).
  for (const char* mname : {"mask", "attn_mask"}) {
    auto it = data.decode_input_map.find(mname);
    if (it == data.decode_input_map.end()) continue;
    data.has_mask_input = true;
    data.decode_mask_idx = it->second;
    auto mtype = data.model->GetInputTensorType(data.decode_sig_idx, mname);
    if (mtype)
      data.mask_is_bool = (*mtype).ElementType() == litert::ElementType::Bool;
    break;
  }

  // 2 KV tensors per layer + the logits output.
  // TODO: count "kv_cache_k_*" outputs instead of assuming the layout.
  data.num_kv_layers = (data.decode_output_map.size() - 1) / 2;

  if (data.num_kv_layers > 0) {
    auto kv_metadata = data.model->GetInputBufferRequirements(
        data.decode_sig_idx, "kv_cache_k_0");
    if (!kv_metadata) {
      LOG(ERROR) << "GetInputBufferRequirements (KV) failed";
      return false;
    }

    auto kv_type =
        data.model->GetInputTensorType(data.decode_sig_idx, "kv_cache_k_0");
    if (!kv_type) {
      LOG(ERROR) << "RankedTensorType failed";
      return false;
    }
    // [1, n_kv_heads, kv_len, head_dim]
    data.kv_cache_max_size = (*kv_type).Layout().Dimensions()[2];

    auto buffer_size = kv_metadata->BufferSize();
    data.kv_buf_float_count = static_cast<int>(*buffer_size / sizeof(float));
  }

  auto logits_metadata =
      data.model->GetOutputBufferRequirements(data.decode_sig_idx, "logits");
  if (!logits_metadata) {
    LOG(ERROR) << "GetOutputBufferRequirements (logits) failed";
    return false;
  }

  auto buffer_size = logits_metadata->BufferSize();
  data.vocab_size = static_cast<int>(*buffer_size / sizeof(float));
  data.logits_scratch.resize(data.vocab_size);
  return true;
}

bool LLMPipeline::BuildPrefillBuffers(LLMBackendData& data,
                                      size_t prefill_sig_idx) {
  if (data.current_prefill_sig_idx == prefill_sig_idx) return true;

  auto input_bufs = data.model->CreateInputBuffers(prefill_sig_idx);
  if (!input_bufs) {
    LOG(ERROR) << "CreateInputBuffers (prefill) failed";
    return false;
  }
  data.prefill_input_bufs = std::move(*input_bufs);

  const auto input_names_exp =
      data.model->GetSignatureInputNames(prefill_sig_idx);
  if (!input_names_exp) {
    LOG(ERROR) << "Couldn't get input names";
    return false;
  }
  const auto& input_names = *input_names_exp;

  const auto output_names_exp =
      data.model->GetSignatureOutputNames(prefill_sig_idx);
  if (!output_names_exp) {
    LOG(ERROR) << "Couldn't get output names";
    return false;
  }
  const auto& output_names = *output_names_exp;

  auto output_bufs = data.model->CreateOutputBuffers(prefill_sig_idx);
  if (!output_bufs) {
    LOG(ERROR) << "CreateOutputBuffers (prefill) failed";
    return false;
  }
  data.prefill_output_bufs = std::move(*output_bufs);

  data.prefill_input_map = make_index_map(input_names);
  data.prefill_output_map = make_index_map(output_names);

  data.prefill_tokens_idx = data.prefill_input_map.at("tokens");
  data.prefill_pos_idx = data.prefill_input_map.at("input_pos");

  // Optional attention-mask input (model exported with a mask input).
  for (const char* mname : {"mask", "attn_mask"}) {
    auto it = data.prefill_input_map.find(mname);
    if (it == data.prefill_input_map.end()) continue;
    data.has_mask_input = true;
    data.prefill_mask_idx = it->second;
    break;
  }

  auto reqs = data.model->GetInputBufferRequirements(prefill_sig_idx, "tokens");
  if (reqs) {
    auto buffer_size = reqs->BufferSize();
    data.prefill_seq_size = static_cast<int>(*buffer_size / sizeof(int32_t));
  }

  data.current_prefill_sig_idx = prefill_sig_idx;
  return true;
}

size_t LLMPipeline::GetSuitablePrefillSignature(
    const std::vector<std::pair<size_t, size_t>>& prefill_sigs,
    size_t num_input_tokens) const {
  size_t best = prefill_sigs.back().first;
  size_t delta = std::numeric_limits<size_t>::max();
  for (const auto& [sig_idx, seq_size] : prefill_sigs) {
    if (seq_size >= num_input_tokens && seq_size - num_input_tokens < delta) {
      delta = seq_size - num_input_tokens;
      best = sig_idx;
    }
  }
  return best;
}

void LLMPipeline::TransferKV(
    int num_layers,
    const std::unordered_map<std::string, size_t>& prefill_output_map,
    const std::unordered_map<std::string, size_t>& decode_input_map,
    std::vector<litert::TensorBuffer>& prefill_output_bufs,
    std::vector<litert::TensorBuffer>& decode_input_bufs) {
  for (int i = 0; i < num_layers; ++i) {
    for (const char* prefix : {"kv_cache_k_", "kv_cache_v_"}) {
      std::string name = std::string(prefix) + std::to_string(i);
      std::swap(prefill_output_bufs[prefill_output_map.at(name)],
                decode_input_bufs[decode_input_map.at(name)]);
    }
  }
}

void LLMPipeline::UpdateDecodeKV(
    int num_layers,
    const std::unordered_map<std::string, size_t>& decode_input_map,
    const std::unordered_map<std::string, size_t>& decode_output_map,
    std::vector<litert::TensorBuffer>& decode_input_bufs,
    std::vector<litert::TensorBuffer>& decode_output_bufs) {
  for (int i = 0; i < num_layers; ++i) {
    for (const char* prefix : {"kv_cache_k_", "kv_cache_v_"}) {
      std::string name = std::string(prefix) + std::to_string(i);
      std::swap(decode_input_bufs[decode_input_map.at(name)],
                decode_output_bufs[decode_output_map.at(name)]);
    }
  }
}

void LLMPipeline::ResetKV(
    int num_layers, int float_count,
    const std::unordered_map<std::string, size_t>& decode_input_map,
    std::vector<litert::TensorBuffer>& decode_input_bufs) {
  if (float_count == 0) return;
  std::vector<float> zeros(float_count, 0.0f);
  for (int i = 0; i < num_layers; ++i) {
    for (const char* prefix : {"kv_cache_k_", "kv_cache_v_"}) {
      std::string name = std::string(prefix) + std::to_string(i);
      decode_input_bufs[decode_input_map.at(name)].Write<float>(
          absl::MakeConstSpan(zeros));
    }
  }
}

void LLMPipeline::ResetPrefillKV(
    int num_layers, int float_count,
    const std::unordered_map<std::string, size_t>& prefill_input_map,
    std::vector<litert::TensorBuffer>& prefill_input_bufs) {
  if (float_count == 0) return;
  std::vector<float> zeros(float_count, 0.0f);
  for (int i = 0; i < num_layers; ++i) {
    for (const char* prefix : {"kv_cache_k_", "kv_cache_v_"}) {
      std::string name = std::string(prefix) + std::to_string(i);
      prefill_input_bufs[prefill_input_map.at(name)].Write<float>(
          absl::MakeConstSpan(zeros));
    }
  }
}

// Fill a causal prefill attention mask. Row s (position s) attends to key
// positions k in [0, s]; everything else is masked. Shape [B,1,S,K] with K the
// KV cache length.
void LLMPipeline::WritePrefillMask(litert::CompiledModel& model, size_t sig_idx,
                                   size_t mask_idx, bool mask_is_bool,
                                   litert::TensorBuffer& buf) {
  auto type = model.GetInputTensorType(sig_idx, mask_idx);
  if (!type) {
    LOG(ERROR) << "GetInputTensorType (prefill mask) failed";
    return;
  }
  const auto dims = (*type).Layout().Dimensions();
  if (dims.size() < 2) {
    LOG(ERROR) << "Unexpected prefill mask rank: " << dims.size();
    return;
  }
  const int S = static_cast<int>(dims[dims.size() - 2]);
  const int K = static_cast<int>(dims[dims.size() - 1]);

  if (mask_is_bool) {
    std::vector<uint8_t> mask(static_cast<size_t>(S) * K, 0);  // masked = false
    for (int s = 0; s < S; ++s)
      for (int k = 0; k <= s && k < K; ++k) mask[s * K + k] = 1;  // allowed
    buf.Write<uint8_t>(absl::MakeConstSpan(mask));
  } else {
    std::vector<float> mask(static_cast<size_t>(S) * K, kMaskedValue);
    for (int s = 0; s < S; ++s)
      for (int k = 0; k <= s && k < K; ++k) mask[s * K + k] = 0.0f;  // allowed
    buf.Write<float>(absl::MakeConstSpan(mask));
  }
}

// Fill a single-step decode attention mask for the given absolute position.
// Shape [B,1,1,K]: attends to key positions k in [0, position].
void LLMPipeline::WriteDecodeMask(litert::CompiledModel& model, size_t sig_idx,
                                  size_t mask_idx, bool mask_is_bool,
                                  litert::TensorBuffer& buf, int position) {
  auto type = model.GetInputTensorType(sig_idx, mask_idx);
  if (!type) {
    LOG(ERROR) << "GetInputTensorType (decode mask) failed";
    return;
  }
  const auto dims = (*type).Layout().Dimensions();
  const int K = static_cast<int>(dims[dims.size() - 1]);

  if (mask_is_bool) {
    std::vector<uint8_t> mask(K, 0);
    for (int k = 0; k <= position && k < K; ++k) mask[k] = 1;
    buf.Write<uint8_t>(absl::MakeConstSpan(mask));
  } else {
    std::vector<float> mask(K, kMaskedValue);
    for (int k = 0; k <= position && k < K; ++k) mask[k] = 0.0f;
    buf.Write<float>(absl::MakeConstSpan(mask));
  }
}

// Greedy sampler (argmax over the logits buffer).
int LLMPipeline::GreedySampler(litert::TensorBuffer& logits_buf,
                               int vocab_size) {
  std::vector<float> logits(vocab_size);
  auto ok = logits_buf.Read<float>(absl::MakeSpan(logits));
  if (!ok) {
    LOG(ERROR) << "Failed to read logits: " << ok.Error().Message();
    return 0;
  }
  float max_value = -std::numeric_limits<float>::infinity();
  int max_index = 0;
  for (int i = 0; i < vocab_size; ++i) {
    if (logits[i] > max_value) {
      max_value = logits[i];
      max_index = i;
    }
  }
  return max_index;
}

#ifdef __cplusplus
};
#endif  // __cplusplus
