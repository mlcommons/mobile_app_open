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
#include "stable_diffusion_invoker.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <random>
#include <tuple>
#include <valarray>
#include <vector>

#include "absl/log/log.h"
#include "embedding_utils.h"
#include "litert/cc/litert_tensor_buffer.h"
#include "sd_utils.h"

namespace {

// All three Stable Diffusion models export exactly one signature.
constexpr size_t kSignatureIndex = 0;

// Latent resolution of the 512x512 UNet: 64 * 64 * 4.
constexpr unsigned kLatentElements = 64 * 64 * 4;

// Classifier-free guidance scale of the reference implementation.
constexpr float kUnconditionalGuidanceScale = 7.5f;

std::vector<float> get_normal(unsigned numbers, unsigned seed = 5,
                              float mean = 0.0, float stddev = 1.0) {
  std::default_random_engine generator(seed);
  std::normal_distribution<float> distribution(mean, stddev);

  std::vector<float> d;
  for (unsigned i = 0; i < numbers; i++) d.push_back(distribution(generator));

  return d;
}

// Writes a host vector into a tensor buffer. TensorBuffer::Write only fails
// when the source is *larger* than the tensor, so the exact-size check has
// to happen here: a short write would silently leave the tail of the tensor
// holding the previous invocation's data.
template <typename T>
bool WriteExact(litert::TensorBuffer &buffer, const std::vector<T> &data,
                const char *what) {
  auto packed = buffer.PackedSize();
  if (!packed) {
    LOG(ERROR) << "PackedSize failed for " << what << ": "
               << packed.Error().Message();
    return false;
  }
  if (*packed != data.size() * sizeof(T)) {
    LOG(ERROR) << "Cannot write " << what << ": the tensor holds " << *packed
               << " bytes, got " << data.size() * sizeof(T);
    return false;
  }
  auto written =
      buffer.Write<T>(litert::Span<const T>(data.data(), data.size()));
  if (!written) {
    LOG(ERROR) << "Failed to write " << what << ": "
               << written.Error().Message();
    return false;
  }
  return true;
}

// Reads a float tensor into *out, sized from the buffer's packed size. The
// model's own shapes are no help here: they are dynamic in dim 0, and
// RankedTensorType::Bytes()/NumElements() fail on dynamic dimensions.
bool ReadFloats(litert::TensorBuffer &buffer, std::vector<float> *out,
                const char *what) {
  auto packed = buffer.PackedSize();
  if (!packed) {
    LOG(ERROR) << "PackedSize failed for " << what << ": "
               << packed.Error().Message();
    return false;
  }
  if (*packed == 0 || *packed % sizeof(float) != 0) {
    LOG(ERROR) << "Unusable size for " << what << ": " << *packed << " bytes";
    return false;
  }
  out->resize(*packed / sizeof(float));
  auto read = buffer.Read<float>(litert::Span<float>(out->data(), out->size()));
  if (!read) {
    LOG(ERROR) << "Failed to read " << what << ": " << read.Error().Message();
    return false;
  }
  return true;
}

// Run is synchronous; the buffers were created once at backend_create time.
bool RunModel(SDModel &model, const char *what) {
  auto run =
      model.compiled->Run(kSignatureIndex, model.input_bufs, model.output_bufs);
  if (!run) {
    LOG(ERROR) << "Failed to run the " << what << ": " << run.Error().Message();
    return false;
  }
  return true;
}

}  // namespace

StableDiffusionInvoker::StableDiffusionInvoker(SDBackendData *backend_data)
    : backend_data_(backend_data) {}

bool StableDiffusionInvoker::invoke(std::vector<float> *image) {
  // No-ops unless the platform releases the transient models between queries
  // (see SDBackendData). Rebuilding is cheap next to a query.
  if (backend_data_->ensure_transient_models &&
      !backend_data_->ensure_transient_models()) {
    LOG(ERROR) << "Failed to rebuild the text encoder and diffusion model";
    return false;
  }

  LOG(INFO) << "Prompt encoding started";
  std::vector<float> encoded_text;
  if (!encode_prompt(backend_data_->input_prompt_tokens, &encoded_text)) {
    return false;
  }
  std::vector<float> unconditional_encoded_text;
  if (!encode_prompt(backend_data_->unconditional_tokens,
                     &unconditional_encoded_text)) {
    return false;
  }

  LOG(INFO) << "Diffusion process started";
  std::vector<float> latent;
  if (!diffusion_process(encoded_text, unconditional_encoded_text,
                         backend_data_->num_steps, backend_data_->seed,
                         &latent)) {
    return false;
  }

  // Decoding is the memory peak and needs neither of these, so let the
  // platform reclaim them first if it asked to.
  if (backend_data_->release_transient_models) {
    backend_data_->release_transient_models();
  }

  LOG(INFO) << "Image decoding started";
  return decode_image(latent, image);
}

bool StableDiffusionInvoker::encode_prompt(const std::vector<int32_t> &tokens,
                                           std::vector<float> *context) {
  SDModel &model = backend_data_->text_encoder;

  // Position ids run 0..76 alongside the token ids.
  std::vector<int32_t> positions(tokens.size());
  for (size_t i = 0; i < positions.size(); ++i) {
    positions[i] = static_cast<int32_t>(i);
  }

  if (!WriteExact(model.input_bufs[backend_data_->encoder_tokens_idx], tokens,
                  "text encoder tokens") ||
      !WriteExact(model.input_bufs[backend_data_->encoder_positions_idx],
                  positions, "text encoder positions")) {
    return false;
  }
  if (!RunModel(model, "text encoder")) return false;
  return ReadFloats(model.output_bufs[model.output_idx], context,
                    "text encoder output");
}

bool StableDiffusionInvoker::diffusion_step(const std::vector<float> &latent,
                                            const std::vector<float> &t_emb,
                                            const std::vector<float> &context,
                                            std::vector<float> *noise) {
  SDModel &model = backend_data_->diffusion;

  if (!WriteExact(model.input_bufs[backend_data_->diffusion_latent_idx], latent,
                  "diffusion latent") ||
      !WriteExact(model.input_bufs[backend_data_->diffusion_context_idx],
                  context, "diffusion context") ||
      !WriteExact(model.input_bufs[backend_data_->diffusion_timestep_idx],
                  t_emb, "diffusion timestep embedding")) {
    return false;
  }
  if (!RunModel(model, "diffusion model")) return false;
  return ReadFloats(model.output_bufs[model.output_idx], noise,
                    "diffusion model output");
}

bool StableDiffusionInvoker::diffusion_process(
    const std::vector<float> &encoded_text,
    const std::vector<float> &unconditional_encoded_text, int num_steps,
    int seed, std::vector<float> *out_latent) {
  auto latent = get_normal(kLatentElements, seed);

  // Get pre-calculated timesteps and embeddings
  auto &embedding_manager = EmbeddingManager::getInstance();
  auto timesteps = embedding_manager.get_timesteps(num_steps);
  if (timesteps.empty()) {
    LOG(ERROR) << "Failed to get timesteps for " << num_steps << " steps";
    return false;
  }

  auto alphas_tuple = get_initial_alphas(timesteps);
  const auto &alphas = std::get<0>(alphas_tuple);
  const auto &alphas_prev = std::get<1>(alphas_tuple);
  // get_initial_alphas returns empty schedules when a timestep falls outside
  // the cumulative alpha table.
  if (alphas.size() != timesteps.size() ||
      alphas_prev.size() != timesteps.size()) {
    LOG(ERROR) << "Failed to build the alpha schedule for " << num_steps
               << " steps";
    return false;
  }

  for (int i = static_cast<int>(timesteps.size()) - 1; i >= 0; --i) {
    LOG(INFO) << "Step " << timesteps.size() - 1 - i;

    auto t_emb = embedding_manager.get_timestep_embedding(i, num_steps);
    if (t_emb.empty()) {
      LOG(ERROR) << "Failed to get the timestep embedding for step " << i;
      return false;
    }

    std::vector<float> unconditional_latent;
    std::vector<float> conditional_latent;
    if (!diffusion_step(latent, t_emb, unconditional_encoded_text,
                        &unconditional_latent) ||
        !diffusion_step(latent, t_emb, encoded_text, &conditional_latent)) {
      return false;
    }

    std::valarray<float> l(conditional_latent.data(),
                           conditional_latent.size());
    // latent still holds this step's input; the DDIM update below replaces
    // it with the next one.
    std::valarray<float> l_prev(latent.data(), latent.size());
    std::valarray<float> u(unconditional_latent.data(),
                           unconditional_latent.size());

    l = u + kUnconditionalGuidanceScale * (l - u);

    auto a_t = alphas[i];
    auto a_prev = alphas_prev[i];

    auto prev_x0 = (l_prev - sqrtf(1.0f - a_t) * l) / sqrtf(a_t);
    l = (l * sqrtf(1.0f - a_prev) + sqrtf(a_prev) * prev_x0);
    latent.assign(std::begin(l), std::end(l));
  }

  LOG(INFO) << "Diffusion process completed!";
  *out_latent = std::move(latent);
  return true;
}

bool StableDiffusionInvoker::decode_image(const std::vector<float> &latent,
                                          std::vector<float> *image) {
  SDModel &model = backend_data_->decoder;

  if (!WriteExact(model.input_bufs[backend_data_->decoder_latent_idx], latent,
                  "decoder latent")) {
    return false;
  }
  if (!RunModel(model, "decoder")) return false;
  return ReadFloats(model.output_bufs[model.output_idx], image,
                    "decoder output");
}
