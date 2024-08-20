#include "flutter/cpp/backends/sd/stable_diffusion_backend.h"
#include <random>
#include <valarray>
#include <iostream>

namespace mlperf {
namespace mobile {

// Utility functions
std::vector<float> get_normal(unsigned numbers, unsigned seed = 5, float mean = 0.0, float stddev = 1.0) {
    std::default_random_engine generator(seed);
    std::normal_distribution<float> distribution(mean, stddev);

    std::vector<float> d(numbers);
    for (auto& value : d)
        value = distribution(generator);

    return d;
}

std::vector<int> get_timesteps(int start, int end, int step) {
    std::vector<int> timesteps;
    for (int i = start; i < end; i += step)
        timesteps.push_back(i);
    return timesteps;
}

std::tuple<std::vector<double>, std::vector<double>> get_initial_alphas(const std::vector<int>& timesteps) {
    std::vector<double> alphas;
    std::vector<double> alphas_prev;

    for (const auto& t : timesteps) {
        alphas.push_back(_ALPHAS_CUMPROD[t]);
    }
    alphas_prev.push_back(1.0);
    alphas_prev.insert(alphas_prev.end(), alphas.begin(), alphas.end() - 1);

    return {alphas, alphas_prev};
}

// Constructor
StableDiffusionBackend::StableDiffusionBackend(const std::string& model_first_path,
                                               const std::string& model_second_path,
                                               const std::string& model_decoder_path,
                                               const std::string& backend_lib_path,
                                               const SettingList& settings,
                                               const std::string& native_lib_path) {
    first_model_ = std::make_unique<ExternalBackend>(model_first_path, backend_lib_path, settings, native_lib_path);
    second_model_ = std::make_unique<ExternalBackend>(model_second_path, backend_lib_path, settings, native_lib_path);
    decoder_ = std::make_unique<ExternalBackend>(model_decoder_path, backend_lib_path, settings, native_lib_path);
}

// Backend interface implementation
const std::string& StableDiffusionBackend::Name() const {
    static std::string name = "StableDiffusionBackend";
    return name;
}

const std::string& StableDiffusionBackend::Vendor() const {
    return first_model_->Vendor();
}

const std::string& StableDiffusionBackend::AcceleratorName() const {
    return first_model_->AcceleratorName();
}

void StableDiffusionBackend::IssueQuery() {
    float unconditional_guidance_scale = 7.5;
    auto noise = get_normal(64 * 64 * 4, 42);  // Example seed
    std::vector<void*> latent = std::vector<void*>(noise.begin(), noise.end());
    auto timesteps = get_timesteps(1, 1000, 1000 / 50);  // Example num_steps = 50
    auto [alphas, alphas_prev] = get_initial_alphas(timesteps);

    for (int i = timesteps.size() - 1; i >= 0; --i) {
        std::vector<void*> latent_prev = latent;
        auto t_emb = get_timestep_embedding(timesteps[i]);

        // Diffusion step 1 (unconditional)
        std::vector<void*> unconditional_output = diffusion_step(latent, t_emb, unconditional_text_);

        // Diffusion step 2 (conditional)
        std::vector<void*> conditional_output = diffusion_step(latent, t_emb, encoded_text_);

        // Mixing results
        std::valarray<float> l(reinterpret_cast<float*>(latent.data()), latent.size());
        std::valarray<float> l_prev(reinterpret_cast<float*>(latent_prev.data()), latent_prev.size());
        std::valarray<float> u(reinterpret_cast<float*>(unconditional_output.data()), unconditional_output.size());

        l = u + unconditional_guidance_scale * (l - u);

        auto a_t = alphas[i];
        auto a_prev = alphas_prev[i];
        auto prev_x0 = (l_prev - sqrtf(1.0f - a_t) * l) / sqrtf(a_t);
        l = (l * sqrtf(1.0f - a_prev) + sqrtf(a_prev) * prev_x0);
        latent.assign(reinterpret_cast<void**>(&l[0]), reinterpret_cast<void**>(&l[0]) + l.size());
    }

    final_output_ = decode(latent);
}

void StableDiffusionBackend::FlushQueries() {
    first_model_->FlushQueries();
    second_model_->FlushQueries();
    decoder_->FlushQueries();
}

void StableDiffusionBackend::SetInputs(const std::vector<void*>& inputs, int batchIndex) {
    encoded_text_ = inputs;  // Assuming the first set of inputs is encoded text
    unconditional_text_ = ...; // Assign unconditional tokens here
    first_model_->SetInputs(inputs, batchIndex);
}

std::vector<void*> StableDiffusionBackend::GetPredictedOutputs(int batchIndex) {
    return final_output_;
}

const DataFormat& StableDiffusionBackend::GetInputFormat() const {
    return first_model_->GetInputFormat();
}

const DataFormat& StableDiffusionBackend::GetOutputFormat() const {
    return decoder_->GetOutputFormat();
}

void StableDiffusionBackend::ConvertInputs(int bytes, int image_width, int image_height, uint8_t* data) {
    first_model_->ConvertInputs(bytes, image_width, image_height, data);
}

// Diffusion step implementation
std::vector<void*> StableDiffusionBackend::diffusion_step(std::vector<void*>& latent,
                                                          std::vector<void*>& t_emb,
                                                          std::vector<void*>& context) {
    // Create a vector of void* pointers for the inputs
    std::vector<void*> inputs;
    inputs.push_back(static_cast<void*>(latent.data()));
    inputs.push_back(static_cast<void*>(t_emb.data()));
    inputs.push_back(static_cast<void*>(context.data()));

    // Set inputs for the first model
    first_model_->SetInputs(inputs);
    
    // Issue the inference query for the first model
    first_model_->IssueQuery();
    
    // Get the output from the first model
    auto first_output = first_model_->GetPredictedOutputs();

    // Set inputs for the second model using the output of the first model
    second_model_->SetInputs(first_output);
    
    // Issue the inference query for the second model
    second_model_->IssueQuery();
    
    // Get and return the output from the second model
    return second_model_->GetPredictedOutputs();
}

// Decode step implementation
std::vector<void*> StableDiffusionBackend::decode(std::vector<void*>& latent) {
    decoder_->SetInputs(latent);
    return decoder_->GetPredictedOutputs();
}

}  // namespace mobile
}  // namespace mlperf