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

#ifndef LITERT_STABLE_DIFFUSION_INVOKER_H_
#define LITERT_STABLE_DIFFUSION_INVOKER_H_

#include <cstdint>
#include <vector>

#include "stable_diffusion_pipeline.h"

// Runs the encode / denoise / decode sequence for a single prompt over the
// compiled models held by SDBackendData. Cheap to construct: it owns no
// LiteRT state, the buffers all live in the backend data.
class StableDiffusionInvoker {
 public:
  explicit StableDiffusionInvoker(SDBackendData *backend_data);

  // Runs the full pipeline for the currently set prompt and writes the
  // decoded 512x512x3 image into *image. Returns false on any failure; the
  // caller reports that as MLPERF_FAILURE.
  bool invoke(std::vector<float> *image);

 private:
  // Runs the text encoder over a 77-token prompt.
  bool encode_prompt(const std::vector<int32_t> &tokens,
                     std::vector<float> *context);

  // Runs the diffusion model once, predicting the noise in `latent`.
  bool diffusion_step(const std::vector<float> &latent,
                      const std::vector<float> &t_emb,
                      const std::vector<float> &context,
                      std::vector<float> *noise);

  // Classifier-free guided DDIM sampling loop.
  bool diffusion_process(const std::vector<float> &encoded_text,
                         const std::vector<float> &unconditional_encoded_text,
                         int num_steps, int seed, std::vector<float> *latent);

  // Runs the decoder, turning the final latent into an RGB image.
  bool decode_image(const std::vector<float> &latent,
                    std::vector<float> *image);

  // Not owned; outlives the invoker.
  SDBackendData *backend_data_;
};

#endif  // LITERT_STABLE_DIFFUSION_INVOKER_H_
