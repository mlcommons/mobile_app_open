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
#include "stable_diffusion_pipeline.h"

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "absl/log/log.h"
#include "embedding_utils.h"
#include "litert/cc/litert_options.h"
#include "litert_env.h"
#include "stable_diffusion_invoker.h"

namespace {

// All three Stable Diffusion models export exactly one signature.
constexpr size_t kSignatureIndex = 0;

// CLIP prompt length, and the size of the decoded RGB image.
constexpr int kTokenCount = 77;
constexpr int kImageElements = 512 * 512 * 3;

// Start-of-text / end-of-text ids of the CLIP BPE vocabulary. The
// unconditional prompt is a start token followed by padding.
constexpr int32_t kStartOfTextToken = 49406;
constexpr int32_t kEndOfTextToken = 49407;

// Every tensor this pipeline binds is int32 or float32, and none is
// quantized at the graph boundary (all quantization parameters are (0, 0)),
// so element size is uniform.
constexpr size_t kElementBytes = 4;

// A tensor bound by name, with the concrete shape it is resized to. Only the
// batch dimension is dynamic in the exported models.
struct TensorSpec {
  const char *name;
  std::vector<int> dims;
};

#if defined(__APPLE__)
// Free a compiled model and everything allocated from it. The buffers were
// created from the compiled model, so they have to go first -- the same
// ordering constraint the SDModel declaration order encodes.
//
// Guarded because its only caller is: an unguarded definition would be an
// unused function in an anonymous namespace on every other platform.
void ReleaseModel(SDModel *model) {
  model->output_bufs.clear();
  model->input_bufs.clear();
  model->compiled.reset();
}
#endif

bool backendExists = false;

// The //flutter/cpp:utils config readers are deliberately not linked into
// this backend (they drag in a second copy of the TF Lite C API), so the two
// cases this pipeline needs are inlined here.
int GetConfigInt(mlperf_backend_configuration_t *configs, const char *key,
                 int default_value) {
  for (int i = 0; i < configs->count; ++i) {
    if (strcmp(configs->keys[i], key) != 0) continue;
    const char *value_str = configs->values[i];
    char *endptr = nullptr;
    errno = 0;
    long value = strtol(value_str, &endptr, 10);
    if (errno == ERANGE || value < INT_MIN || value > INT_MAX ||
        endptr == value_str || *endptr != '\0') {
      LOG(ERROR) << "Invalid int value for " << key << ": " << value_str;
      return default_value;
    }
    return static_cast<int>(value);
  }
  return default_value;
}

std::string GetConfigString(mlperf_backend_configuration_t *configs,
                            const char *key, const std::string &default_value) {
  for (int i = 0; i < configs->count; ++i) {
    if (strcmp(configs->keys[i], key) == 0) {
      return std::string(configs->values[i]);
    }
  }
  return default_value;
}

size_t ElementCount(const std::vector<int> &dims) {
  size_t count = 1;
  for (int dim : dims) count *= static_cast<size_t>(dim);
  return count;
}

std::unordered_map<std::string, size_t> MakeIndexMap(
    const std::vector<litert::StringView> &names) {
  std::unordered_map<std::string, size_t> map;
  for (size_t i = 0; i < names.size(); ++i) map[std::string(names[i])] = i;
  return map;
}

std::string JoinNames(const std::vector<litert::StringView> &names) {
  std::string joined;
  for (const auto &name : names) {
    if (!joined.empty()) joined += ", ";
    joined += std::string(name);
  }
  return joined;
}

// Binds one role to its signature index by exact tensor name. There is
// deliberately no positional fallback: the signature keys are ordered
// alphabetically and do not match the positional tensor order, so a wrong
// binding would produce a silently wrong image instead of a crash.
bool LookupTensor(const std::unordered_map<std::string, size_t> &index_map,
                  const std::vector<litert::StringView> &names,
                  const std::string &model_path, const char *tensor_name,
                  size_t *index) {
  auto it = index_map.find(tensor_name);
  if (it == index_map.end()) {
    LOG(ERROR) << "Model " << model_path << " has no tensor named '"
               << tensor_name
               << "'; its signature exposes: " << JoinNames(names);
    return false;
  }
  *index = it->second;
  return true;
}

// A buffer whose size does not match the shape this pipeline resized it to
// means the model file is not the export this code was written for.
bool CheckPackedSize(const litert::TensorBuffer &buffer, const TensorSpec &spec,
                     const std::string &model_path) {
  auto packed = buffer.PackedSize();
  if (!packed) {
    LOG(ERROR) << "PackedSize failed for '" << spec.name << "' of "
               << model_path << ": " << packed.Error().Message();
    return false;
  }
  const size_t expected = ElementCount(spec.dims) * kElementBytes;
  if (*packed != expected) {
    LOG(ERROR) << "Tensor '" << spec.name << "' of " << model_path << " holds "
               << *packed << " bytes, expected " << expected;
    return false;
  }
  return true;
}

// Compiles one model on the CPU accelerator, binds every tensor by name,
// pins the batch dimension and creates the buffers that all invocations
// reuse. On success `*input_indices` holds the signature index of each spec
// in `input_specs`, in the same order.
bool BuildModel(litert::Environment &env, const std::string &model_path,
                int num_threads, const std::vector<TensorSpec> &input_specs,
                const TensorSpec &output_spec, SDModel *model,
                std::vector<size_t> *input_indices) {
  auto options = litert::Options::Create();
  if (!options) {
    LOG(ERROR) << "Options::Create failed for " << model_path;
    return false;
  }
  options->SetHardwareAccelerators(litert::HwAccelerators::kCpu);
  if (num_threads > 0) {
    auto cpu_options = options->GetCpuOptions();
    if (cpu_options) {
      cpu_options->SetNumThreads(num_threads);
    } else {
      LOG(WARNING) << "GetCpuOptions failed; using the default thread count";
    }
  }

  auto compiled = litert::CompiledModel::Create(env, model_path, *options);
  if (!compiled) {
    LOG(ERROR) << "CompiledModel::Create failed for " << model_path << ": "
               << compiled.Error().Message();
    return false;
  }
  model->compiled =
      std::make_unique<litert::CompiledModel>(std::move(*compiled));

  auto input_names = model->compiled->GetSignatureInputNames(kSignatureIndex);
  auto output_names = model->compiled->GetSignatureOutputNames(kSignatureIndex);
  if (!input_names || !output_names) {
    LOG(ERROR) << "Failed to read the signature tensor names of " << model_path;
    return false;
  }
  const auto input_map = MakeIndexMap(*input_names);
  const auto output_map = MakeIndexMap(*output_names);

  input_indices->clear();
  for (const auto &spec : input_specs) {
    size_t index = 0;
    if (!LookupTensor(input_map, *input_names, model_path, spec.name, &index)) {
      return false;
    }
    input_indices->push_back(index);
  }
  if (!LookupTensor(output_map, *output_names, model_path, output_spec.name,
                    &model->output_idx)) {
    return false;
  }

  // Pin the shapes before the buffers are created: every input exports the
  // batch dimension as dynamic (-1) and has no size until it is resized.
  for (size_t i = 0; i < input_specs.size(); ++i) {
    const std::vector<int> &dims = input_specs[i].dims;
    auto resized = model->compiled->ResizeInputTensorNonStrict(
        kSignatureIndex, (*input_indices)[i],
        litert::Span<const int>(dims.data(), dims.size()));
    if (!resized) {
      LOG(ERROR) << "Failed to resize input '" << input_specs[i].name << "' of "
                 << model_path << ": " << resized.Error().Message();
      return false;
    }
  }

  // Created once and reused by every invocation; the denoising loop must not
  // reallocate them per step.
  auto input_bufs = model->compiled->CreateInputBuffers(kSignatureIndex);
  auto output_bufs = model->compiled->CreateOutputBuffers(kSignatureIndex);
  if (!input_bufs || !output_bufs) {
    LOG(ERROR) << "Failed to create the tensor buffers of " << model_path;
    return false;
  }
  model->input_bufs = std::move(*input_bufs);
  model->output_bufs = std::move(*output_bufs);

  for (size_t i = 0; i < input_specs.size(); ++i) {
    if (!CheckPackedSize(model->input_bufs[(*input_indices)[i]], input_specs[i],
                         model_path)) {
      return false;
    }
  }
  return CheckPackedSize(model->output_bufs[model->output_idx], output_spec,
                         model_path);
}

}  // namespace

// Create a new backend and return the pointer to it.
mlperf_backend_ptr_t StableDiffusionPipeline::backend_create(
    const char *model_path, mlperf_backend_configuration_t *configs,
    const char *native_lib_path) {
  // model_path is a directory for this benchmark: it ships three models plus
  // the timestep embedding table, and the filenames come from the settings.

  // Verify only one instance of the backend exists at any time
  if (backendExists) {
    LOG(ERROR) << "Only one backend instance should exist at a time";
    return nullptr;
  }

  auto *backend_data = new SDBackendData();
  backendExists = true;

  backend_data->seed = GetConfigInt(configs, "stable_diffusion_seed", 0);
  if (backend_data->seed == 0) {
    LOG(ERROR) << "Cannot get stable_diffusion_seed";
    backend_delete(backend_data);
    return nullptr;
  }
  backend_data->num_steps =
      GetConfigInt(configs, "stable_diffusion_num_steps", 0);
  if (backend_data->num_steps <= 0) {
    LOG(ERROR) << "Cannot get stable_diffusion_num_steps";
    backend_delete(backend_data);
    return nullptr;
  }
  const int num_threads = GetConfigInt(configs, "num_threads", 4);

  const std::string text_encoder_name =
      GetConfigString(configs, "text_encoder_filename", "");
  const std::string diffusion_model_name =
      GetConfigString(configs, "diffusion_model_filename", "");
  const std::string decoder_name =
      GetConfigString(configs, "decoder_filename", "");
  const std::string timestep_embeddings_name =
      GetConfigString(configs, "timestep_embeddings_filename", "");
  if (text_encoder_name.empty() || diffusion_model_name.empty() ||
      decoder_name.empty() || timestep_embeddings_name.empty()) {
    LOG(ERROR) << "Missing a Stable Diffusion filename in the settings";
    backend_delete(backend_data);
    return nullptr;
  }

  const std::string dir = std::string(model_path) + "/";
  const std::string text_encoder_path = dir + text_encoder_name;
  const std::string diffusion_model_path = dir + diffusion_model_name;
  const std::string decoder_path = dir + decoder_name;
  const std::string timestep_embeddings_path = dir + timestep_embeddings_name;

  // The shipped exports are dynamic_int8 (text encoder, diffusion) and
  // dynamic_fp16 (decoder), aimed at CPU/XNNPACK; the settings only offer a
  // CPU delegate for this benchmark.
  if (configs->delegate_selected != nullptr &&
      strcmp(configs->delegate_selected, "CPU") != 0) {
    LOG(WARNING) << "Ignoring delegate_selected=" << configs->delegate_selected
                 << "; the Stable Diffusion pipeline runs on the CPU";
  }

#if defined(__APPLE__)
  // This is the most memory-hungry pipeline in the backend and, in a CI sweep,
  // it starts after five other benchmarks have each built and torn down a
  // backend. Hand back whatever the allocator is still holding from those
  // before compiling about 1 GiB of models on top of it.
  litert_apple::ReturnFreeMemoryToOS();
  LITERT_LOG_MEM("sd: before compiling models");
#endif

  // One environment shared by all three compiled models, as the LiteRT
  // header recommends. Built through the shared factory so the Apple runtime
  // library directory is set the same way as in the other pipelines; this
  // pipeline is CPU-only today, but an environment that cannot find the
  // accelerators is a trap for whoever adds a GPU choice here.
  auto env = CreateLiteRtEnvironment();
  if (!env) {
    LOG(ERROR) << "Environment::Create failed";
    backend_delete(backend_data);
    return nullptr;
  }
  backend_data->env = std::make_unique<litert::Environment>(std::move(*env));

  // Signature input keys are alphabetical, so these lists are in role order,
  // not tensor order: the text encoder's signature reads (positions,
  // tokens), and the diffusion model's reads (context, latent,
  // timestep_embedding).
  const std::vector<TensorSpec> encoder_inputs = {
      {"tokens", {1, kTokenCount}},
      {"positions", {1, kTokenCount}},
  };
  const TensorSpec encoder_output = {"layer_normalization_72",
                                     {1, kTokenCount, 768}};
  const std::vector<TensorSpec> diffusion_inputs = {
      {"latent", {1, 64, 64, 4}},
      {"context", {1, kTokenCount, 768}},
      {"timestep_embedding", {1, 1280}},
  };
  const TensorSpec diffusion_output = {"padded_conv2d_83", {1, 64, 64, 4}};
  const std::vector<TensorSpec> decoder_inputs = {
      {"input_1", {1, 64, 64, 4}},
  };
  const TensorSpec decoder_output = {"padded_conv2d_37", {1, 512, 512, 3}};

  std::vector<size_t> indices;
  if (!BuildModel(*backend_data->env, text_encoder_path, num_threads,
                  encoder_inputs, encoder_output, &backend_data->text_encoder,
                  &indices)) {
    backend_delete(backend_data);
    return nullptr;
  }
  backend_data->encoder_tokens_idx = indices[0];
  backend_data->encoder_positions_idx = indices[1];

  if (!BuildModel(*backend_data->env, diffusion_model_path, num_threads,
                  diffusion_inputs, diffusion_output, &backend_data->diffusion,
                  &indices)) {
    backend_delete(backend_data);
    return nullptr;
  }
  backend_data->diffusion_latent_idx = indices[0];
  backend_data->diffusion_context_idx = indices[1];
  backend_data->diffusion_timestep_idx = indices[2];

  if (!BuildModel(*backend_data->env, decoder_path, num_threads, decoder_inputs,
                  decoder_output, &backend_data->decoder, &indices)) {
    backend_delete(backend_data);
    return nullptr;
  }
  backend_data->decoder_latent_idx = indices[0];

  if (!EmbeddingManager::getInstance().load_timestep_embeddings(
          timestep_embeddings_path)) {
    LOG(ERROR) << "Failed to load timestep embeddings from "
               << timestep_embeddings_path;
    backend_delete(backend_data);
    return nullptr;
  }

  // The unconditional prompt is constant: a start token, then padding.
  backend_data->unconditional_tokens.assign(kTokenCount, kEndOfTextToken);
  backend_data->unconditional_tokens[0] = kStartOfTextToken;
  backend_data->input_prompt_tokens.assign(kTokenCount, 0);

  LITERT_LOG_MEM("sd: all three models compiled");

#if defined(__APPLE__)
  // See the comment on these members in the header: the decode step is the
  // memory peak and iOS kills the process past its high-watermark limit, so
  // the encoder and the diffusion model do not stay resident across it.
  backend_data->set_phase =
      [backend_data, text_encoder_path, diffusion_model_path, decoder_path,
       num_threads, encoder_inputs, encoder_output, diffusion_inputs,
       diffusion_output, decoder_inputs, decoder_output](SDPhase phase) {
        // Release first, then build: the point is to never hold both phases'
        // working sets at once. Destroying a model is not enough on its own --
        // the pages stay on libmalloc's free list and keep counting against the
        // limit until they are handed back, which is what makes the release
        // visible to EXC_RESOURCE.
        std::vector<size_t> idx;
        if (phase == SDPhase::kEncodeAndDiffuse) {
          ReleaseModel(&backend_data->decoder);
          litert_apple::ReturnFreeMemoryToOS();
          if (backend_data->text_encoder.compiled == nullptr) {
            if (!BuildModel(*backend_data->env, text_encoder_path, num_threads,
                            encoder_inputs, encoder_output,
                            &backend_data->text_encoder, &idx)) {
              return false;
            }
            backend_data->encoder_tokens_idx = idx[0];
            backend_data->encoder_positions_idx = idx[1];
          }
          if (backend_data->diffusion.compiled == nullptr) {
            if (!BuildModel(*backend_data->env, diffusion_model_path,
                            num_threads, diffusion_inputs, diffusion_output,
                            &backend_data->diffusion, &idx)) {
              return false;
            }
            backend_data->diffusion_latent_idx = idx[0];
            backend_data->diffusion_context_idx = idx[1];
            backend_data->diffusion_timestep_idx = idx[2];
          }
          return true;
        }

        ReleaseModel(&backend_data->text_encoder);
        ReleaseModel(&backend_data->diffusion);
        litert_apple::ReturnFreeMemoryToOS();
        if (backend_data->decoder.compiled == nullptr) {
          if (!BuildModel(*backend_data->env, decoder_path, num_threads,
                          decoder_inputs, decoder_output,
                          &backend_data->decoder, &idx)) {
            return false;
          }
          backend_data->decoder_latent_idx = idx[0];
        }
        return true;
      };
#endif

  return backend_data;
}

// Vendor name who create this backend.
const char *StableDiffusionPipeline::backend_vendor_name(
    mlperf_backend_ptr_t backend_ptr) {
  auto *backend_data = static_cast<SDBackendData *>(backend_ptr);
  return backend_data->vendor;
}

// Return the name of the accelerator.
const char *StableDiffusionPipeline::backend_accelerator_name(
    mlperf_backend_ptr_t backend_ptr) {
  auto *backend_data = static_cast<SDBackendData *>(backend_ptr);
  return backend_data->accelerator;
}

// Return the name of this backend.
const char *StableDiffusionPipeline::backend_name(
    mlperf_backend_ptr_t backend_ptr) {
  auto *backend_data = static_cast<SDBackendData *>(backend_ptr);
  return backend_data->name;
}

// Destroy the backend pointer and its data.
void StableDiffusionPipeline::backend_delete(mlperf_backend_ptr_t backend_ptr) {
  // ~SDBackendData tears the LiteRT objects down in reverse declaration
  // order, which gives buffers, then compiled models, then the shared
  // environment. Safe on a partially built backend: every member is RAII.
  delete static_cast<SDBackendData *>(backend_ptr);
  backendExists = false;
  LITERT_LOG_MEM("sd: backend deleted (before reclaim)");
#if defined(__APPLE__)
  // The next benchmark allocates into whatever this leaves behind, and this
  // pipeline is the largest consumer in the backend. Destroying the models
  // only returns the pages to the allocator's free list, where they still
  // count against the limit, so hand them back to the OS here too.
  litert_apple::ReturnFreeMemoryToOS();
  LITERT_LOG_MEM("sd: backend deleted (after reclaim)");
#endif
}

// Run the inference for a sample.
mlperf_status_t StableDiffusionPipeline::backend_issue_query(
    mlperf_backend_ptr_t backend_ptr, ft_callback callback, void *context) {
  auto *backend_data = static_cast<SDBackendData *>(backend_ptr);
  StableDiffusionInvoker invoker(backend_data);
  if (!invoker.invoke(&backend_data->output)) {
    LOG(ERROR) << "Stable Diffusion inference failed";
    return MLPERF_FAILURE;
  }
  return MLPERF_SUCCESS;
}

// Flush the staged queries immediately.
mlperf_status_t StableDiffusionPipeline::backend_flush_queries(
    mlperf_backend_ptr_t backend_ptr) {
  return MLPERF_SUCCESS;
}

// Return the number of inputs of the model.
int32_t StableDiffusionPipeline::backend_get_input_count(
    mlperf_backend_ptr_t backend_ptr) {
  return 1;
}

// Return the type of the ith input: the text encoder's "tokens" tensor.
mlperf_data_t StableDiffusionPipeline::backend_get_input_type(
    mlperf_backend_ptr_t backend_ptr, int32_t i) {
  mlperf_data_t result;
  result.type = mlperf_data_t::Int32;
  result.size = kTokenCount;
  if (i != 0) {
    LOG(ERROR) << "Unsupported input index: " << i;
    result.size = 0;
  }
  return result;
}

// Set the data for ith input.
mlperf_status_t StableDiffusionPipeline::backend_set_input(
    mlperf_backend_ptr_t backend_ptr, int32_t batchIndex, int32_t i,
    void *data) {
  auto *backend_data = static_cast<SDBackendData *>(backend_ptr);
  if (i != 0 || data == nullptr) {
    LOG(ERROR) << "Unsupported input index: " << i;
    return MLPERF_FAILURE;
  }

  // The dataset hands over a zero-terminated token array sized for the
  // encoder input; stop at the terminator but never scan past the tensor.
  const int *tokens = static_cast<const int *>(data);
  int token_count = 0;
  while (token_count < kTokenCount && tokens[token_count] != 0) ++token_count;

  // Rewrite the whole tensor length so no tail from the previous sample
  // survives into this run.
  backend_data->input_prompt_tokens.assign(kTokenCount, 0);
  std::copy(tokens, tokens + token_count,
            backend_data->input_prompt_tokens.begin());
  return MLPERF_SUCCESS;
}

// Return the number of outputs for the model.
int32_t StableDiffusionPipeline::backend_get_output_count(
    mlperf_backend_ptr_t backend_ptr) {
  return 1;
}

// Return the type of ith output: the decoder's RGB image.
mlperf_data_t StableDiffusionPipeline::backend_get_output_type(
    mlperf_backend_ptr_t backend_ptr, int32_t i) {
  mlperf_data_t result;
  result.type = mlperf_data_t::Float32;
  result.size = kImageElements;
  if (i != 0) {
    LOG(ERROR) << "Unsupported output index: " << i;
    result.size = 0;
  }
  return result;
}

// Get the data from ith output.
mlperf_status_t StableDiffusionPipeline::backend_get_output(
    mlperf_backend_ptr_t backend_ptr, uint32_t batchIndex, int32_t i,
    void **data) {
  auto *backend_data = static_cast<SDBackendData *>(backend_ptr);
  if (i != 0) {
    LOG(ERROR) << "Unsupported output index: " << i;
    return MLPERF_FAILURE;
  }
  if (backend_data->output.size() != static_cast<size_t>(kImageElements)) {
    LOG(ERROR) << "Decoded image holds " << backend_data->output.size()
               << " floats, expected " << kImageElements;
    return MLPERF_FAILURE;
  }
  // Points into the member staging vector, so it stays valid after this
  // call returns.
  *data = backend_data->output.data();
  return MLPERF_SUCCESS;
}

void StableDiffusionPipeline::backend_convert_inputs(
    mlperf_backend_ptr_t backend_ptr, int bytes, int width, int height,
    uint8_t *data) {}

void StableDiffusionPipeline::backend_convert_outputs(
    mlperf_backend_ptr_t backend_ptr, int bytes, int width, int height,
    uint8_t *data) {}

void *StableDiffusionPipeline::backend_get_buffer(size_t n) {
  return ::operator new(n);
}

void StableDiffusionPipeline::backend_release_buffer(void *p) {
  ::operator delete(p);
}
