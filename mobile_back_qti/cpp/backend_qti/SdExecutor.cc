/* Copyright (c) 2020-2025 Qualcomm Innovation Center, Inc. All rights reserved.

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

#include "SdExecutor.h"

#define xverstr(a) verstr(a)
#define verstr(a) #a

#ifndef SNPE_VERSION_STRING
#define SNPE_VERSION_STRING "default"
#endif

std::vector<float> get_normal(unsigned numbers, unsigned seed = 5,
                              float mean = 0.0, float stddev = 1.0) {
  std::default_random_engine generator(seed);
  std::normal_distribution<float> distribution(mean, stddev);

  std::vector<float> d;
  for (unsigned i = 0; i < numbers; i++) d.push_back(distribution(generator));

  return d;
}

void SdExecutor::create(const char* model_path, const char* native_model_path) {
#ifdef STABLEDIFFUSION_FLAG
  bool use_mmap = false;  // we don't want to use cached
  uint64_t context_bin_mmap_read_budget = 100000;
  native_lib_path = std::string(native_model_path);
  data_folder_path = std::string(model_path);

  // Initialize input and output formats
  inputFormat_.clear();
  outputFormat_.clear();

  mlperf_data_t input;
  input.type = mlperf_data_t::Int32;
  input.size = 77 * 1;  // tokenized inputs 77 numbers
  inputFormat_.push_back(input);

  mlperf_data_t output;
  output.type = mlperf_data_t::Uint8;
  output.size = 512 * 512 * 3;
  outputFormat_.push_back(output);

  sd_pipeline = new QnnApiHelpers();

  if (0 != sd_pipeline->Init(data_folder_path, native_lib_path, 768, 77, 1.0,
                             512, 512, 3.0, use_mmap,
                             context_bin_mmap_read_budget)) {
    LOG(FATAL) << "Initialization Failure";
  }

  LOG(INFO) << "SD pipeline Initialized successfully";
#else
  LOG(ERROR) << "Stable Diffusion support not compiled in";
#endif
}

mlperf_status_t SdExecutor::set_input(int32_t batchIndex, int32_t inputIndex,
                                      void* data) {
#ifdef STABLEDIFFUSION_FLAG
  // preprocess input
  int32_t* input_prompt_ids = (int32_t*)data;
  std::vector<float> noise = get_normal(64 * 64 * 4, seed);
  if (sd_pipeline->PreProcessInput(input_prompt_ids, noise, num_steps,
                                   guidance_scale)) {
    return MLPERF_SUCCESS;
  } else {
    LOG(ERROR) << "PreProcessInput failed";
    return MLPERF_FAILURE;
  }
#else
  LOG(ERROR) << "Stable Diffusion support not compiled in";
  return MLPERF_FAILURE;
#endif
}

mlperf_status_t SdExecutor::get_output(uint32_t batchIndex, int32_t outputIndex,
                                       void** data) {
#ifdef STABLEDIFFUSION_FLAG
  if (outputIndex >= static_cast<int>(outputFormat_.size())) {
    LOG(ERROR) << "Invalid output index: " << outputIndex;
    return MLPERF_FAILURE;
  }

  JniHelpers::InferenceReturn inferenceReturn;
  if (!sd_pipeline->PostProcessOutput(false, false, inferenceReturn)) {
    LOG(ERROR) << "PostProcessOutput failure";
    return MLPERF_FAILURE;
  }

  *data = inferenceReturn.m_ImageData;
  return MLPERF_SUCCESS;
#else
  LOG(ERROR) << "Stable Diffusion support not compiled in";
  return MLPERF_FAILURE;
#endif
}

mlperf_status_t SdExecutor::execute(ft_callback /*callback*/,
                                    void* /*context*/) {
#ifdef STABLEDIFFUSION_FLAG
  if (!sd_pipeline) {
    LOG(ERROR) << "SD pipeline not initialized";
    return MLPERF_FAILURE;
  }

  for (int stepIdx = 0; stepIdx < num_steps; stepIdx++) {
    bool runVAE = ((stepIdx + 1) == num_steps);
    if (!sd_pipeline->RunInference(runVAE)) {
      LOG(ERROR) << "RunInference failure at step " << stepIdx;
      return MLPERF_FAILURE;
    }
  }
  return MLPERF_SUCCESS;
#else
  LOG(ERROR) << "Stable Diffusion support not compiled in";
  return MLPERF_FAILURE;
#endif
}

void* SdExecutor::getBuffer(size_t n) {
  // Stable Diffusion doesn't use this buffer management approach
  LOG(WARNING) << "getBuffer not implemented for Stable Diffusion";
  return nullptr;
}

void SdExecutor::deregister(void* p) {
  // Stable Diffusion doesn't use this buffer management approach
  LOG(WARNING) << "deregister not implemented for Stable Diffusion";
}

const char* SdExecutor::get_name_() const { return name_; }

bool SdExecutor::getUseIonBuffers_() const { return false; }

std::vector<mlperf_data_t> SdExecutor::getInputFormat_() const {
  return inputFormat_;
}

std::vector<mlperf_data_t> SdExecutor::getOutputFormat_() const {
  return outputFormat_;
}

// setting configs
void SdExecutor::setConfigs(const mlperf_backend_configuration_t* configs) {
  // Setting defaults
  // TODO: check if its requried or not
  // setDelegate_utils(configs->accelerator);
}