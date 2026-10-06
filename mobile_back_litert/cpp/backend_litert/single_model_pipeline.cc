/* Copyright 2020-2026 The MLPerf Authors. All Rights Reserved.

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
#include "single_model_pipeline.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <future>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#if __ANDROID__
#include <sys/system_properties.h>
#endif

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#include "absl/log/log.h"
#include "flutter/cpp/c/type.h"
#include "litert/cc/litert_compiled_model.h"
#include "litert/cc/litert_element_type.h"
#include "litert/cc/litert_environment.h"
#include "litert/cc/litert_environment_options.h"
#include "litert/cc/litert_expected.h"
#include "litert/cc/litert_model.h"
#include "litert/cc/litert_options.h"
#include "litert/cc/litert_tensor_buffer.h"
#include "litert_env.h"
#include "thread_pool.h"

namespace {

constexpr char kDelegateCpu[] = "CPU";
constexpr char kDelegateGpu[] = "GPU";
#if defined(__APPLE__)
// The Apple settings name the GPU choice after the accelerator that serves it,
// the same way the TFLite backend's iOS settings do.
constexpr char kDelegateMetal[] = "Metal";
#endif

// The vision/NLP models expose a single (default) signature.
constexpr size_t kSignatureIndex = 0;

// Per-tensor metadata captured once at create time, so the harness-facing
// calls never have to touch LiteRT objects.
struct TensorInfo {
  mlperf_data_t::Type type = mlperf_data_t::Float32;
  size_t per_sample_elements = 0;
  size_t per_sample_bytes = 0;
  size_t batch_bytes = 0;  // per_sample_bytes * real_batch_size
};

mlperf_data_t::Type ToMlperfType(litert::ElementType type) {
  switch (type) {
    case litert::ElementType::Float32:
      return mlperf_data_t::Float32;
    case litert::ElementType::UInt8:
      return mlperf_data_t::Uint8;
    case litert::ElementType::Int8:
      return mlperf_data_t::Int8;
    case litert::ElementType::Float16:
      return mlperf_data_t::Float16;
    case litert::ElementType::Int32:
      return mlperf_data_t::Int32;
    case litert::ElementType::Int64:
      return mlperf_data_t::Int64;
    default:
      LOG(ERROR) << "Unsupported element type: " << static_cast<int>(type);
      return mlperf_data_t::Float32;
  }
}

size_t ByteWidthOf(mlperf_data_t::Type type) {
  switch (type) {
    case mlperf_data_t::Uint8:
    case mlperf_data_t::Int8:
      return 1;
    case mlperf_data_t::Float16:
      return 2;
    case mlperf_data_t::Float32:
    case mlperf_data_t::Int32:
      return 4;
    case mlperf_data_t::Int64:
      return 8;
    default:
      return 4;
  }
}

#if __ANDROID__
bool IsEmulator() {
  char ro_build_characteristics[PROP_VALUE_MAX + 1];
  if (__system_property_get("ro.build.characteristics",
                            ro_build_characteristics)) {
    if (strstr(ro_build_characteristics, "emulator")) return true;
  }
  return false;
}
#endif

struct LiteRTBackendData {
  const char *name = "LiteRT";
  const char *vendor = "Google";
  const char *accelerator = "CPU";

  // Declaration order matters: members are destroyed in reverse order, and
  // the buffers must go before the compiled models, which must go before the
  // environment.
  std::unique_ptr<litert::Environment> env;
  std::unique_ptr<litert::Model> model;
  std::vector<litert::CompiledModel> shards;
  std::vector<std::vector<litert::TensorBuffer>> input_bufs;   // [shard][i]
  std::vector<std::vector<litert::TensorBuffer>> output_bufs;  // [shard][i]

  // Host-side staging. The harness memcpys inputs in before issue_query and
  // reads output pointers after it returns; TensorBuffer memory is only
  // mapped between Lock/Unlock, so stable pointers must live here.
  std::vector<std::vector<std::vector<uint8_t>>> input_staging;
  std::vector<std::vector<std::vector<uint8_t>>> output_staging;

  std::vector<TensorInfo> inputs;
  std::vector<TensorInfo> outputs;

  int32_t shards_num = 1;
  uint32_t real_batch_size = 1;
  std::unique_ptr<Threadpool> executer;
};

bool backendExists = false;

// Reads the element types for the default signature from the model and plans
// the input resizes: the benchmark models export the batch dimension as
// dynamic (-1), so every input whose leading dimension is not
// real_batch_size gets resized on the compiled model before buffers are
// created (the legacy interpreter pipeline did the same). Tensor byte sizes
// are not computed here — dynamic shapes have none until after the resize —
// but from the created TensorBuffers in BuildShards.
bool PlanTensors(LiteRTBackendData *backend_data,
                 std::vector<std::vector<int>> *input_resizes) {
  auto input_names = backend_data->model->GetSignatureInputNames();
  auto output_names = backend_data->model->GetSignatureOutputNames();
  if (!input_names || !output_names) {
    LOG(ERROR) << "Failed to read model signature tensor names";
    return false;
  }

  backend_data->inputs.resize(input_names->size());
  input_resizes->assign(input_names->size(), {});
  for (size_t i = 0; i < input_names->size(); ++i) {
    auto type = backend_data->model->GetInputTensorType(kSignatureIndex, i);
    if (!type) {
      LOG(ERROR) << "Failed to read input tensor type " << i << ": "
                 << type.Error().Message();
      return false;
    }
    backend_data->inputs[i].type = ToMlperfType(type->ElementType());
    auto dims = type->Layout().Dimensions();
    std::vector<int> target(dims.begin(), dims.end());
    for (size_t d = 1; d < target.size(); ++d) {
      if (target[d] <= 0) {
        LOG(ERROR) << "Input " << i << " has a dynamic non-batch dimension";
        return false;
      }
    }
    if (!target.empty() &&
        target[0] != static_cast<int>(backend_data->real_batch_size)) {
      target[0] = static_cast<int>(backend_data->real_batch_size);
      (*input_resizes)[i] = std::move(target);
    }
  }

  backend_data->outputs.resize(output_names->size());
  for (size_t i = 0; i < output_names->size(); ++i) {
    auto type = backend_data->model->GetOutputTensorType(kSignatureIndex, i);
    if (!type) {
      LOG(ERROR) << "Failed to read output tensor type " << i << ": "
                 << type.Error().Message();
      return false;
    }
    backend_data->outputs[i].type = ToMlperfType(type->ElementType());
  }
  return true;
}

// Fills the tensor sizes from the actual buffer sizes of the first shard
// (exact after any resize, unlike the model's static shapes).
bool ReadTensorSizes(LiteRTBackendData *backend_data) {
  auto fill = [&](std::vector<TensorInfo> &infos,
                  std::vector<litert::TensorBuffer> &bufs) -> bool {
    for (size_t i = 0; i < infos.size(); ++i) {
      auto packed = bufs[i].PackedSize();
      if (!packed || *packed == 0 ||
          *packed % backend_data->real_batch_size != 0) {
        LOG(ERROR) << "Unusable tensor buffer size for tensor " << i;
        return false;
      }
      infos[i].batch_bytes = *packed;
      infos[i].per_sample_bytes = *packed / backend_data->real_batch_size;
      infos[i].per_sample_elements =
          infos[i].per_sample_bytes / ByteWidthOf(infos[i].type);
    }
    return true;
  };
  return fill(backend_data->inputs, backend_data->input_bufs[0]) &&
         fill(backend_data->outputs, backend_data->output_bufs[0]);
}

// Compiles one model per shard for the given accelerator, applies the
// planned input resizes, and creates the tensor buffers and host staging.
// Returns false (with everything it built cleared) so the caller can retry
// on another accelerator.
bool BuildShards(LiteRTBackendData *backend_data, const char *model_path,
                 const std::vector<std::vector<int>> &input_resizes,
                 bool use_gpu, int num_threads) {
  backend_data->shards.clear();
  for (int k = 0; k < backend_data->shards_num; ++k) {
    auto options = litert::Options::Create();
    if (!options) {
      LOG(ERROR) << "Options::Create failed";
      return false;
    }
    if (use_gpu) {
      // CPU stays in the set so ops the GPU cannot take fall back per-op
      // instead of failing compilation outright.
      options->SetHardwareAccelerators(litert::HwAccelerators::kGpu |
                                       litert::HwAccelerators::kCpu);
    } else {
      options->SetHardwareAccelerators(litert::HwAccelerators::kCpu);
    }
    if (num_threads > 0) {
      auto cpu_options = options->GetCpuOptions();
      if (cpu_options) {
        cpu_options->SetNumThreads(num_threads);
      } else {
        LOG(WARNING) << "GetCpuOptions failed; using default thread count";
      }
    }
    auto compiled = litert::CompiledModel::Create(
        *backend_data->env, std::string(model_path), *options);
    if (!compiled) {
      LOG(ERROR) << "CompiledModel::Create (" << (use_gpu ? "GPU" : "CPU")
                 << ") failed: " << compiled.Error().Message();
      backend_data->shards.clear();
      return false;
    }
    backend_data->shards.push_back(std::move(*compiled));
  }

  auto fail = [backend_data]() -> bool {
    backend_data->input_bufs.clear();
    backend_data->output_bufs.clear();
    backend_data->input_staging.clear();
    backend_data->output_staging.clear();
    backend_data->shards.clear();
    return false;
  };

  backend_data->input_bufs.clear();
  backend_data->output_bufs.clear();
  backend_data->input_staging.clear();
  backend_data->output_staging.clear();
  for (int k = 0; k < backend_data->shards_num; ++k) {
    for (size_t i = 0; i < input_resizes.size(); ++i) {
      const auto &dims = input_resizes[i];
      if (dims.empty()) continue;
      auto resized = backend_data->shards[k].ResizeInputTensorNonStrict(
          kSignatureIndex, i,
          litert::Span<const int>(dims.data(), dims.size()));
      if (!resized) {
        LOG(ERROR) << "Failed to resize input " << i << ": "
                   << resized.Error().Message();
        return fail();
      }
    }
    // Propagate the new shapes before creating the buffers. A resize only
    // marks the signature as needing allocation; LiteRT defers AllocateTensors
    // to Run, so the output tensors still report their pre-resize sizes here.
    // CreateOutputBuffers sizes a CPU buffer from exactly that value, and Run
    // then registers it as a TFLite custom allocation and reallocates, which
    // fails with "Custom allocation is too small for tensor idx" once the real
    // shapes land. Asking for the output layouts with update_allocation forces
    // the allocation first. The GPU path is unaffected (its requirements come
    // from the accelerator, not from tensor->bytes), and Run allocates again
    // anyway, so this is safe for both.
    auto layouts = backend_data->shards[k].GetOutputTensorLayouts(
        kSignatureIndex, /*update_allocation=*/true);
    if (!layouts) {
      LOG(ERROR) << "Failed to update the tensor allocation for shard " << k
                 << ": " << layouts.Error().Message();
      return fail();
    }
    auto input_bufs = backend_data->shards[k].CreateInputBuffers();
    auto output_bufs = backend_data->shards[k].CreateOutputBuffers();
    if (!input_bufs || !output_bufs) {
      LOG(ERROR) << "Failed to create tensor buffers for shard " << k;
      return fail();
    }
    if (input_bufs->size() != backend_data->inputs.size() ||
        output_bufs->size() != backend_data->outputs.size()) {
      LOG(ERROR) << "Tensor buffer count does not match the model signature";
      return fail();
    }
    backend_data->input_bufs.push_back(std::move(*input_bufs));
    backend_data->output_bufs.push_back(std::move(*output_bufs));
  }

  if (!ReadTensorSizes(backend_data)) return fail();

  for (int k = 0; k < backend_data->shards_num; ++k) {
    std::vector<std::vector<uint8_t>> in_staging;
    for (const auto &info : backend_data->inputs) {
      in_staging.emplace_back(info.batch_bytes);
    }
    backend_data->input_staging.push_back(std::move(in_staging));
    std::vector<std::vector<uint8_t>> out_staging;
    for (const auto &info : backend_data->outputs) {
      out_staging.emplace_back(info.batch_bytes);
    }
    backend_data->output_staging.push_back(std::move(out_staging));
  }
  return true;
}

}  // namespace

// Destroy the backend pointer and its data.
void SingleModelPipeline::backend_delete(mlperf_backend_ptr_t backend_ptr) {
  delete static_cast<LiteRTBackendData *>(backend_ptr);
  backendExists = false;
}

// Create a new backend and return the pointer to it.
mlperf_backend_ptr_t SingleModelPipeline::backend_create(
    const char *model_path, mlperf_backend_configuration_t *configs,
    const char *native_lib_path) {
  // Verify only one instance of the backend exists at any time
  if (backendExists) {
    LOG(ERROR) << "Only one backend instance should exist at a time";
    return nullptr;
  }

  auto *backend_data = new LiteRTBackendData();
  backendExists = true;

  int num_threads = 0;
  int32_t shards_num_setting = 0;
  for (int i = 0; i < configs->count; ++i) {
    if (strcmp(configs->keys[i], "num_threads") == 0) {
      num_threads = atoi(configs->values[i]);
    } else if (strcmp(configs->keys[i], "shards_num") == 0) {
      shards_num_setting = atoi(configs->values[i]);
    }
  }

  if (configs->batch_size > 1) {
    // With batching the query is split over shards_num model instances
    // (default 2), each running batch_size / shards_num samples.
    backend_data->shards_num = shards_num_setting > 0 ? shards_num_setting : 2;
    if ((configs->batch_size % backend_data->shards_num) != 0) {
      LOG(ERROR) << "Batch size is not dividable by shards_num: "
                 << configs->batch_size << " % " << backend_data->shards_num
                 << " != 0";
      backend_delete(backend_data);
      return nullptr;
    }
    backend_data->real_batch_size =
        configs->batch_size / backend_data->shards_num;
  }

  backend_data->executer =
      std::make_unique<Threadpool>(backend_data->shards_num);

  auto env = CreateLiteRtEnvironment();
  if (!env) {
    LOG(ERROR) << "Environment::Create failed";
    backend_delete(backend_data);
    return nullptr;
  }
  backend_data->env = std::make_unique<litert::Environment>(std::move(*env));

  auto model = litert::Model::CreateFromFile(model_path);
  if (!model) {
    LOG(ERROR) << "Failed to load model: " << model_path;
    backend_delete(backend_data);
    return nullptr;
  }
  backend_data->model = std::make_unique<litert::Model>(std::move(*model));

  std::vector<std::vector<int>> input_resizes;
  if (!PlanTensors(backend_data, &input_resizes)) {
    backend_delete(backend_data);
    return nullptr;
  }

  bool use_gpu = strcmp(configs->delegate_selected, kDelegateGpu) == 0;
#if defined(__APPLE__)
  use_gpu = use_gpu || strcmp(configs->delegate_selected, kDelegateMetal) == 0;
#endif
  // Report an unrecognized selection before any platform downgrade below, so a
  // valid choice that we deliberately fall back from is not logged as unknown.
  if (!use_gpu && strcmp(configs->delegate_selected, kDelegateCpu) != 0) {
    LOG(ERROR) << "Unknown delegate_selected: " << configs->delegate_selected
               << "; using the CPU accelerator";
  }
#if __ANDROID__
  if (use_gpu && IsEmulator()) {
    LOG(INFO) << "Emulator detected, using the CPU accelerator";
    use_gpu = false;
  }
#elif defined(__APPLE__) && TARGET_OS_SIMULATOR
  // Only the ios_arm64 (device) slice of the Metal accelerator is downloaded by
  // litert_backend.mk, so there is nothing for the simulator to dlopen.
  if (use_gpu) {
    LOG(INFO) << "Simulator detected, using the CPU accelerator";
    use_gpu = false;
  }
#endif

  if (use_gpu && backend_data->shards_num > 1) {
    // Two shards Run() concurrently in issue_query; on the shared LiteRT GPU
    // context that deadlocks (observed on Pixel 10 Pro). Batched GPU
    // benchmarks should set shards_num to 1 and rely on the batch resize.
    LOG(WARNING) << "shards_num=" << backend_data->shards_num
                 << " with the GPU accelerator may deadlock";
  }
  if (use_gpu && !BuildShards(backend_data, model_path, input_resizes,
                              /*use_gpu=*/true, num_threads)) {
    LOG(WARNING) << "GPU compilation failed; falling back to CPU";
    use_gpu = false;
  }
  if (!use_gpu && !BuildShards(backend_data, model_path, input_resizes,
                               /*use_gpu=*/false, num_threads)) {
    backend_delete(backend_data);
    return nullptr;
  }
  backend_data->accelerator = use_gpu ? "GPU" : "CPU";

  return backend_data;
}

// Vendor name who create this backend.
const char *SingleModelPipeline::backend_vendor_name(
    mlperf_backend_ptr_t backend_ptr) {
  auto *backend_data = static_cast<LiteRTBackendData *>(backend_ptr);
  return backend_data->vendor;
}

// Return the name of the accelerator.
const char *SingleModelPipeline::backend_accelerator_name(
    mlperf_backend_ptr_t backend_ptr) {
  auto *backend_data = static_cast<LiteRTBackendData *>(backend_ptr);
  return backend_data->accelerator;
}

// Return the name of this backend.
const char *SingleModelPipeline::backend_name(
    mlperf_backend_ptr_t backend_ptr) {
  auto *backend_data = static_cast<LiteRTBackendData *>(backend_ptr);
  return backend_data->name;
}

// Run the inference for a sample.
mlperf_status_t SingleModelPipeline::backend_issue_query(
    mlperf_backend_ptr_t backend_ptr, ft_callback callback, void *context) {
  auto *backend_data = static_cast<LiteRTBackendData *>(backend_ptr);

  auto task = [backend_data](int k) -> bool {
    for (size_t i = 0; i < backend_data->input_bufs[k].size(); ++i) {
      const auto &staging = backend_data->input_staging[k][i];
      auto written = backend_data->input_bufs[k][i].Write<uint8_t>(
          litert::Span<const uint8_t>(staging.data(), staging.size()));
      if (!written) {
        LOG(ERROR) << "Failed to write input " << i << ": "
                   << written.Error().Message();
        return false;
      }
    }
    auto run = backend_data->shards[k].Run(kSignatureIndex,
                                           backend_data->input_bufs[k],
                                           backend_data->output_bufs[k]);
    if (!run) {
      LOG(ERROR) << "CompiledModel::Run failed: " << run.Error().Message();
      return false;
    }
    for (size_t i = 0; i < backend_data->output_bufs[k].size(); ++i) {
      auto &staging = backend_data->output_staging[k][i];
      auto read = backend_data->output_bufs[k][i].Read<uint8_t>(
          litert::Span<uint8_t>(staging.data(), staging.size()));
      if (!read) {
        LOG(ERROR) << "Failed to read output " << i << ": "
                   << read.Error().Message();
        return false;
      }
    }
    return true;
  };

  std::vector<std::future<bool>> f;
  f.resize(backend_data->shards_num);
  // dispatch workers for shards
  for (int k = 1; k < backend_data->shards_num; k++) {
    f[k] = backend_data->executer->submit(task, k);
  }
  // main thread for the first shard
  bool status = task(0);
  // sync and get result of workers
  for (int k = 1; k < backend_data->shards_num; k++) {
    status = f[k].get() && status;
  }
  return status ? MLPERF_SUCCESS : MLPERF_FAILURE;
}

// Flush the staged queries immediately.
mlperf_status_t SingleModelPipeline::backend_flush_queries(
    mlperf_backend_ptr_t backend_ptr) {
  return MLPERF_SUCCESS;
}

// Return the number of inputs of the model.
int32_t SingleModelPipeline::backend_get_input_count(
    mlperf_backend_ptr_t backend_ptr) {
  auto *backend_data = static_cast<LiteRTBackendData *>(backend_ptr);
  return static_cast<int32_t>(backend_data->inputs.size());
}

// Return the type of the ith input.
mlperf_data_t SingleModelPipeline::backend_get_input_type(
    mlperf_backend_ptr_t backend_ptr, int32_t i) {
  auto *backend_data = static_cast<LiteRTBackendData *>(backend_ptr);
  mlperf_data_t type;
  type.type = backend_data->inputs[i].type;
  type.size = backend_data->inputs[i].per_sample_elements;
  return type;
}

// Set the data for ith input.
mlperf_status_t SingleModelPipeline::backend_set_input(
    mlperf_backend_ptr_t backend_ptr, int32_t batch_index, int32_t i,
    void *data) {
  auto *backend_data = static_cast<LiteRTBackendData *>(backend_ptr);
  const int shard_index = batch_index / backend_data->real_batch_size;
  const size_t per_sample_bytes = backend_data->inputs[i].per_sample_bytes;
  const size_t offset =
      per_sample_bytes * (batch_index % backend_data->real_batch_size);
  memcpy(backend_data->input_staging[shard_index][i].data() + offset, data,
         per_sample_bytes);
  return MLPERF_SUCCESS;
}

// Return the number of outputs for the model.
int32_t SingleModelPipeline::backend_get_output_count(
    mlperf_backend_ptr_t backend_ptr) {
  auto *backend_data = static_cast<LiteRTBackendData *>(backend_ptr);
  return static_cast<int32_t>(backend_data->outputs.size());
}

// Return the type of ith output.
mlperf_data_t SingleModelPipeline::backend_get_output_type(
    mlperf_backend_ptr_t backend_ptr, int32_t i) {
  auto *backend_data = static_cast<LiteRTBackendData *>(backend_ptr);
  mlperf_data_t type;
  type.type = backend_data->outputs[i].type;
  type.size = backend_data->outputs[i].per_sample_elements;
  return type;
}

// Get the data from ith output.
mlperf_status_t SingleModelPipeline::backend_get_output(
    mlperf_backend_ptr_t backend_ptr, uint32_t batch_index, int32_t i,
    void **data) {
  auto *backend_data = static_cast<LiteRTBackendData *>(backend_ptr);
  const int shard_index = batch_index / backend_data->real_batch_size;
  const size_t offset = backend_data->outputs[i].per_sample_bytes *
                        (batch_index % backend_data->real_batch_size);
  *data = backend_data->output_staging[shard_index][i].data() + offset;
  return MLPERF_SUCCESS;
}

void SingleModelPipeline::backend_convert_inputs(
    mlperf_backend_ptr_t backend_ptr, int bytes, int width, int height,
    uint8_t *data) {}

void SingleModelPipeline::backend_convert_outputs(
    mlperf_backend_ptr_t backend_ptr, int bytes, int width, int height,
    uint8_t *data) {}

void *SingleModelPipeline::backend_get_buffer(size_t n) {
  return ::operator new(n);
}

void SingleModelPipeline::backend_release_buffer(void *p) {
  ::operator delete(p);
}
