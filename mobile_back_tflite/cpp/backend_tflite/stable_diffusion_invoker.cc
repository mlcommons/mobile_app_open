#include "stable_diffusion_invoker.h"

#include <iostream>
#include <random>
#include <valarray>

#include "sd_utils.h"
#include "stable_diffusion_pipeline.h"
#include "tensorflow/lite/c/c_api.h"
#include "tensorflow/lite/c/common.h"

std::vector<float> get_normal(unsigned numbers, unsigned seed = 5,
                              float mean = 0.0, float stddev = 1.0) {
  std::default_random_engine generator(seed);
  std::normal_distribution<float> distribution(mean, stddev);

  std::vector<float> d;
  for (unsigned i = 0; i < numbers; i++) d.push_back(distribution(generator));

  return d;
}

StableDiffusionInvoker::StableDiffusionInvoker(SDBackendData* backend_data)
    : backend_data_(backend_data) {}

std::vector<float> StableDiffusionInvoker::invoke() {
  std::cout << "Prompt encoding started" << std::endl;
  auto encoded_text = encode_prompt(backend_data_->input_prompt_tokens);
  auto unconditional_encoded_text =
      encode_prompt(backend_data_->unconditional_tokens);
  std::cout << "Diffusion process started" << std::endl;
  auto latent =
      diffusion_process(encoded_text, unconditional_encoded_text,
                        backend_data_->num_steps, backend_data_->seed);
  std::cout << "Image decoding started" << std::endl;
  return decode_image(latent);
}

std::vector<float> StableDiffusionInvoker::encode_prompt(
    const std::vector<int>& data) {
  return run_inference(backend_data_->text_encoder_interpreter, data);
}

std::vector<float> StableDiffusionInvoker::diffusion_step(
    const std::vector<float>& latent, const std::vector<float>& t_emb,
    const std::vector<float>& context) {
  // Prepare the first model's inputs

  auto first_input_details =
      TfLiteInterpreterGetInputTensor(backend_data_->first_interpreter, 0);
  auto second_input_details =
      TfLiteInterpreterGetInputTensor(backend_data_->first_interpreter, 1);
  auto third_input_details =
      TfLiteInterpreterGetInputTensor(backend_data_->first_interpreter, 2);

  std::copy(context.begin(), context.end(),
            reinterpret_cast<float*>(TfLiteTensorData(first_input_details)));
  std::copy(t_emb.begin(), t_emb.end(),
            reinterpret_cast<float*>(TfLiteTensorData(second_input_details)));
  std::copy(latent.begin(), latent.end(),
            reinterpret_cast<float*>(TfLiteTensorData(third_input_details)));

  // Invoke the first model
  if (TfLiteInterpreterInvoke(backend_data_->first_interpreter) != kTfLiteOk) {
    std::cerr << "Failed to invoke the first diffusion model!" << std::endl;
    exit(-1);
  }

  // Output names from the first model and corresponding input names for the
  // second model
  std::vector<std::string> output_names = {
      "Identity_6",  "Identity_4", "Identity",    "input_1",    "Identity_12",
      "Identity_11", "Identity_3", "Identity_10", "Identity_9", "Identity_5",
      "Identity_8",  "Identity_7", "Identity_2"};

  std::vector<std::string> input_names = {
      "args_0",    "args_0_1",  "args_0_2", "args_0_4", "args_0_3",
      "args_0_5",  "args_0_6",  "args_0_7", "args_0_8", "args_0_9",
      "args_0_10", "args_0_11", "args_0_12"};

  // Copy outputs of the first model to the inputs of the second model based on
  // names
  for (size_t i = 0; i < input_names.size(); ++i) {
    int input_index = get_tensor_index_by_name(
        backend_data_->second_interpreter, input_names[i], true);
    int output_index = get_tensor_index_by_name(
        backend_data_->first_interpreter, output_names[i], false);

    if (input_index == -1 || output_index == -1) {
      std::cerr << "Failed to find matching input or output tensor by name!"
                << std::endl;
      exit(-1);
    }

    auto first_model_output_details = TfLiteInterpreterGetOutputTensor(
        backend_data_->first_interpreter, output_index);

    float* output_data =
        reinterpret_cast<float*>(TfLiteTensorData(first_model_output_details));
    int output_size =
        TfLiteTensorByteSize(first_model_output_details) / sizeof(float);

    float* input_data = reinterpret_cast<float*>(
        TfLiteTensorData(TfLiteInterpreterGetInputTensor(
            backend_data_->second_interpreter, input_index)));

    std::copy(output_data, output_data + output_size, input_data);
  }

  // Invoke the second model
  if (TfLiteInterpreterInvoke(backend_data_->second_interpreter) != kTfLiteOk) {
    std::cerr << "Failed to invoke the second diffusion model!" << std::endl;
    exit(-1);
  }

  float* output = reinterpret_cast<float*>(TfLiteTensorData(
      TfLiteInterpreterGetOutputTensor(backend_data_->second_interpreter, 0)));
  int output_size = TfLiteTensorByteSize(TfLiteInterpreterGetOutputTensor(
                        backend_data_->second_interpreter, 0)) /
                    sizeof(float);
  return std::vector<float>(output, output + output_size);
}

// Helper function to get tensor index by name
int StableDiffusionInvoker::get_tensor_index_by_name(
    TfLiteInterpreter* interpreter, const std::string& name, bool is_input) {
  int tensor_count = is_input
                         ? TfLiteInterpreterGetInputTensorCount(interpreter)
                         : TfLiteInterpreterGetOutputTensorCount(interpreter);

  for (int i = 0; i < tensor_count; ++i) {
    const char* tensor_name =
        is_input
            ? TfLiteTensorName(TfLiteInterpreterGetInputTensor(interpreter, i))
            : TfLiteTensorName(
                  TfLiteInterpreterGetOutputTensor(interpreter, i));

    if (tensor_name == name) {
      return i;
    }
  }
  return -1;
}

std::vector<float> StableDiffusionInvoker::diffusion_process(
    const std::vector<float>& encoded_text,
    const std::vector<float>& unconditional_encoded_text, int num_steps,
    int seed) {
  float unconditional_guidance_scale = 7.5f;
  auto noise = get_normal(64 * 64 * 4, seed);
  auto latent = noise;

  auto timesteps = get_timesteps(1, 1000, 1000 / num_steps);
  auto alphas_tuple = get_initial_alphas(timesteps);
  auto alphas = std::get<0>(alphas_tuple);
  auto alphas_prev = std::get<1>(alphas_tuple);

  for (int i = timesteps.size() - 1; i >= 0; --i) {
    std::cout << "Step " << timesteps.size() - 1 - i << "\n";

    auto latent_prev = latent;
    auto t_emb = get_timestep_embedding(timesteps[i]);

    auto unconditional_latent =
        diffusion_step(latent, t_emb, unconditional_encoded_text);
    latent = diffusion_step(latent, t_emb, encoded_text);

    std::valarray<float> l(latent.data(), latent.size());
    std::valarray<float> l_prev(latent_prev.data(), latent_prev.size());
    std::valarray<float> u(unconditional_latent.data(),
                           unconditional_latent.size());

    l = u + unconditional_guidance_scale * (l - u);

    auto a_t = alphas[i];
    auto a_prev = alphas_prev[i];

    auto prev_x0 = (l_prev - sqrtf(1.0f - a_t) * l) / sqrtf(a_t);
    l = (l * sqrtf(1.0f - a_prev) + sqrtf(a_prev) * prev_x0);
    latent.assign(std::begin(l), std::end(l));
  }

  return latent;
}

std::vector<float> StableDiffusionInvoker::decode_image(
    const std::vector<float>& latent) {
  return run_inference(backend_data_->decoder_interpreter, latent);
}

std::vector<float> StableDiffusionInvoker::run_inference(
    TfLiteInterpreter* interpreter, const std::vector<int>& encoded) {
  // Determine the size of the encoded input
  int encoded_size = encoded.size();

  // Generate position IDs corresponding to the length of the encoded tokens
  std::vector<int> pos_ids(encoded_size);
  for (int i = 0; i < encoded_size; ++i) {
    pos_ids[i] = i;  // Position ID corresponds to the index
  }

  // Access the input tensors
  void* pos_ids_input_data =
      TfLiteTensorData(TfLiteInterpreterGetInputTensor(interpreter, 0));
  void* encoded_input_data =
      TfLiteTensorData(TfLiteInterpreterGetInputTensor(interpreter, 1));

  // Copy data to input tensors (type cast required for correct copy operation)
  std::memcpy(pos_ids_input_data, pos_ids.data(), pos_ids.size() * sizeof(int));
  std::memcpy(encoded_input_data, encoded.data(), encoded.size() * sizeof(int));

  // Invoke the model
  if (TfLiteInterpreterInvoke(interpreter) != kTfLiteOk) {
    std::cerr << "Failed to invoke tflite!" << std::endl;
    exit(-1);
  }

  // Access the output tensor
  const TfLiteTensor* output_tensor =
      TfLiteInterpreterGetOutputTensor(interpreter, 0);
  void* output_data = TfLiteTensorData(output_tensor);

  // Calculate the number of elements in the output tensor
  int output_size = TfLiteTensorByteSize(output_tensor) / sizeof(float);

  // Cast output_data back to the correct type and return as a vector of floats
  float* output = static_cast<float*>(output_data);
  return std::vector<float>(output, output + output_size);
}

std::vector<float> StableDiffusionInvoker::run_inference(
    TfLiteInterpreter* interpreter, const std::vector<float>& input) {
  std::copy(input.begin(), input.end(),
            reinterpret_cast<float*>(TfLiteTensorData(
                TfLiteInterpreterGetInputTensor(interpreter, 0))));

  if (TfLiteInterpreterInvoke(interpreter) != kTfLiteOk) {
    std::cerr << "Failed to invoke the model!" << std::endl;
    exit(-1);
  }

  float* output = reinterpret_cast<float*>(
      TfLiteTensorData(TfLiteInterpreterGetOutputTensor(interpreter, 0)));
  int output_size =
      TfLiteTensorByteSize(TfLiteInterpreterGetOutputTensor(interpreter, 0)) /
      sizeof(float);
  return std::vector<float>(output, output + output_size);
}

std::vector<float> StableDiffusionInvoker::get_timestep_embedding(
    int timestep, int dim, float max_period) {
  int half = dim / 2;
  std::vector<float> freqs(half);
  for (int i = 0; i < half; ++i) {
    freqs[i] = std::exp(-std::log(max_period) * i / half);
  }

  std::vector<float> args(half);
  for (int i = 0; i < half; ++i) {
    args[i] = timestep * freqs[i];
  }

  std::vector<float> embedding(2 * half);
  for (int i = 0; i < half; ++i) {
    embedding[i] = std::cos(args[i]);
    embedding[half + i] = std::sin(args[i]);
  }

  return embedding;
}