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

#include "backend_utils.h"

#include "soc_utility.h"

// TODO: Remove these global variables
std::string delegate;
std::string scenario;

size_t calcSizeFromDims_utils(const size_t rank, const size_t *dims) {
  if (rank == 0) return 0;
  size_t size = 1;
  for (size_t i = rank; i > 0; i--) {
    if (*dims != 0)
      size *= *dims;
    else
      size *= 10;
    dims++;
  }
  return size;
}

// Helper for splitting tokenized strings
static void split_utils(std::vector<std::string> &split_string,
                        const std::string &tokenized_string,
                        const char separator) {
  split_string.clear();
  std::istringstream tokenized_string_stream(tokenized_string);

  while (!tokenized_string_stream.eof()) {
    std::string value;
    getline(tokenized_string_stream, value, separator);
    if (!value.empty()) {
      split_string.push_back(value);
    }
  }
}

Snpe_TensorShape_Handle_t calcStrides_utils(
    Snpe_TensorShape_Handle_t dimsHandle, size_t elementSize) {
  std::vector<size_t> strides(Snpe_TensorShape_Rank(dimsHandle));
  strides[strides.size() - 1] = elementSize;
  size_t stride = strides[strides.size() - 1];
  for (size_t i = Snpe_TensorShape_Rank(dimsHandle) - 1; i > 0; i--) {
    if (Snpe_TensorShape_At(dimsHandle, i) != 0)
      stride *= Snpe_TensorShape_At(dimsHandle, i);
    else
      stride *= 10;
    strides[i - 1] = stride;
  }
  Snpe_TensorShape_Handle_t tensorShapeHandle = Snpe_TensorShape_CreateDimsSize(
      strides.data(), Snpe_TensorShape_Rank(dimsHandle));
  return tensorShapeHandle;
}

Snpe_Runtime_t Str2Delegate_utils(const snpe_runtimes_ delegate) {
  Snpe_Runtime_t runtime;
  std::string delegate_used = "unlisted";
  switch (delegate) {
    case snpe_runtimes_::SNPE_DSP:
      runtime = SNPE_RUNTIME_DSP;
      delegate_used = "SNPE_DSP";
      break;
    case snpe_runtimes_::SNPE_GPU:
      runtime = SNPE_RUNTIME_GPU;
      delegate_used = "SNPE_GPU";
      break;
    case snpe_runtimes_::SNPE_CPU:
      runtime = SNPE_RUNTIME_CPU;
      delegate_used = "SNPE_CPU";
      break;
    case snpe_runtimes_::SNPE_GPU_FP16:
      runtime = SNPE_RUNTIME_GPU_FLOAT16;
      delegate_used = "SNPE_GPU_FP16";
      break;
    default:
      runtime = SNPE_RUNTIME_UNSET;
      LOG(ERROR) << "runtime not supported";
      break;
  }

  if (Snpe_Util_IsRuntimeAvailableCheckOption(
          runtime, SNPE_RUNTIME_CHECK_OPTION_UNSIGNEDPD_CHECK)) {
    LOG(INFO) << "runtime " << delegate_used
              << " is available on this platform";
  } else {
    LOG(FATAL) << "runtime " << delegate_used
               << " is not available on this platform";
  }

  return runtime;
}

Snpe_StringList_Handle_t ResolveCommaSeparatedList_utils(std::string &line) {
  Snpe_StringList_Handle_t stringListHandle = Snpe_StringList_Create();
  if (!line.empty()) {
    std::vector<std::string> names;
    split_utils(names, line.substr(0), ',');
    for (auto &name : names)
      Snpe_StringList_Append(stringListHandle, name.c_str());
  }
  return stringListHandle;
}

bool IsRuntimeAvailable_utils(const snpe_runtimes_ delegate) {
  return (Str2Delegate_utils(delegate) != SNPE_RUNTIME_UNSET);
}

void get_accelerator_instances_utils(int &num_dsp, int &num_gpu, int &num_cpu,
                                     int &num_gpu_fp16) {
  // std::string &delegate = delegate;
  num_dsp = 0;
  num_gpu = 0;
  num_cpu = 0;
  num_gpu_fp16 = 0;
  if (scenario == "Offline") {
    Socs::soc_offline_core_instance(num_dsp, num_gpu, num_cpu, num_gpu_fp16,
                                    delegate);
  } else {
    if (delegate == "snpe_dsp" || delegate == "psnpe_dsp") {
      num_dsp = 1;
      Socs::set_use_dsp_features(true);
    } else if (delegate == "snpe_gpu" || delegate == "psnpe_gpu") {
      num_gpu = 1;
      Socs::set_use_dsp_features(false);
    } else if (delegate == "snpe_cpu" || delegate == "psnpe_cpu") {
      num_cpu = 1;
      Socs::set_use_dsp_features(false);
    } else if (delegate == "snpe_gpu_fp16" || delegate == "psnpe_gpu_fp16") {
      num_gpu_fp16 = 1;
      Socs::set_use_dsp_features(false);
    } else {
      LOG(FATAL) << "Error: Unsupported delegate " << delegate << " SoC ID "
                 << Socs::get_soc_name();
    }
  }
  LOG(INFO) << "Using " << num_dsp << " dsp " << num_gpu << " gpu" << num_cpu
            << " cpu" << num_gpu_fp16 << " gpu_fp16";
}

std::vector<float> get_normal_utils(unsigned numbers, unsigned seed = 5,
                                    float mean = 0.0, float stddev = 1.0) {
  std::default_random_engine generator(seed);
  std::normal_distribution<float> distribution(mean, stddev);

  std::vector<float> d;
  for (unsigned i = 0; i < numbers; i++) d.push_back(distribution(generator));

  return d;
}

void setScenario_utils(std::string scenario_) { scenario = scenario_; }

void setDelegate_utils(std::string delegate_) { delegate = delegate_; }