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
