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

#ifndef LITERT_STABLE_DIFFUSION_PIPELINE_H_
#define LITERT_STABLE_DIFFUSION_PIPELINE_H_

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "flutter/cpp/c/type.h"
#include "litert/cc/litert_compiled_model.h"
#include "litert/cc/litert_environment.h"
#include "litert/cc/litert_tensor_buffer.h"
#include "pipeline.h"

// One stage of the Stable Diffusion pipeline: a compiled model plus the
// tensor buffers every invocation reuses.
//
// Declaration order matters: members are destroyed in reverse order, and the
// buffers must go before the compiled model they were created from.
struct SDModel {
  std::unique_ptr<litert::CompiledModel> compiled;
  std::vector<litert::TensorBuffer> input_bufs;
  std::vector<litert::TensorBuffer> output_bufs;

  // Signature index of the model's single output, resolved by name.
  size_t output_idx = 0;
};

// The three stages of a query, which on Apple are kept apart in memory. Each
// needs exactly one of the three models, and the values that pass between them
// -- the encoded prompts, then the latent -- are small host vectors, so no two
// models ever have to be resident at once.
enum class SDPhase {
  kEncode,
  kDiffuse,
  kDecode,
};

struct SDBackendData {
  const char *name = "LiteRT";
  const char *vendor = "Google";
  const char *accelerator = "CPU";

  // Declaration order matters here too: the three compiled models (and the
  // buffers inside them) must be destroyed before the environment they
  // share, so the environment is declared first and destroyed last.
  std::unique_ptr<litert::Environment> env;
  SDModel text_encoder;
  SDModel diffusion;
  SDModel decoder;

  // Signature input indices, resolved by name at create time. The signature
  // input keys are ordered alphabetically, which does not match the
  // positional tensor order, so none of these may be assumed.
  size_t encoder_tokens_idx = 0;
  size_t encoder_positions_idx = 0;
  size_t diffusion_latent_idx = 0;
  size_t diffusion_context_idx = 0;
  size_t diffusion_timestep_idx = 0;
  size_t decoder_latent_idx = 0;

  std::vector<int32_t> input_prompt_tokens;
  std::vector<int32_t> unconditional_tokens;

  int num_steps = 20;
  int seed = 633994880;

  // Host staging for the decoded image: backend_get_output hands out a
  // pointer into it, so it has to stay valid after the call returns.
  std::vector<float> output;

  // Apple CPU only. Left null on the GPU, and everywhere off Apple, where
  // every model stays resident.
  //
  // iOS kills a process that crosses a per-process limit (measured at 3376 MB
  // on an 8 GB device) and the kill cannot be caught. Releasing a model and
  // rebuilding it later is only worth doing when the release actually returns
  // the memory, which is true of the CPU path and NOT of the GPU one:
  // ReleaseModel drops the LiteRT objects and ReturnFreeMemoryToOS empties
  // libmalloc's free list, but the delegate's weights sit in Metal buffers
  // that libmalloc never owned. Measured on an iPhone 16 Pro, in MiB still
  // available before the limit:
  //
  //   before compiling models                      2885
  //   all three models compiled                    1517   (the three cost 1368)
  //   query start, after releasing two of them     1586   (only 69 recovered)
  //   diffusion model rebuilt                        72   -> killed
  //
  // So on the GPU the phases are not used at all: the three models are
  // compiled once and left alone. They fit together with room for the working
  // set, and not rebuilding them each query also takes a 20-step image from
  // 45.5 s to 18.8 s on the macOS host.
  //
  // The CPU path keeps the phases. There the weights are ordinary allocations
  // that the release does return, and the three models plus a phase's working
  // set do not fit together.
  std::function<bool(SDPhase)> set_phase;
};

// A pipeline for Stable Diffusion.
class StableDiffusionPipeline : public Pipeline {
 public:
  StableDiffusionPipeline() = default;

  ~StableDiffusionPipeline() override = default;

  void backend_delete(mlperf_backend_ptr_t backend_ptr) override;

  mlperf_backend_ptr_t backend_create(const char *model_path,
                                      mlperf_backend_configuration_t *configs,
                                      const char *native_lib_path) override;

  const char *backend_vendor_name(mlperf_backend_ptr_t backend_ptr) override;

  const char *backend_accelerator_name(
      mlperf_backend_ptr_t backend_ptr) override;

  const char *backend_name(mlperf_backend_ptr_t backend_ptr) override;

  mlperf_status_t backend_issue_query(mlperf_backend_ptr_t backend_ptr,
                                      ft_callback callback,
                                      void *context) override;

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
};

#endif  // LITERT_STABLE_DIFFUSION_PIPELINE_H_
