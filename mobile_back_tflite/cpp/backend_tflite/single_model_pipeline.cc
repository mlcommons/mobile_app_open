/* Copyright 2020-2021 The MLPerf Authors. All Rights Reserved.

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

#include <cstdlib>
#include <cstring>

#if defined(MTK_TFLITE_NEURON_BACKEND) && defined(__ANDROID__)
#include <dlfcn.h>

#include "neuron/APUWareUtilsApi.h"
#endif

#include "flutter/cpp/c/type.h"
#include "tensorflow/lite/c/c_api.h"
#include "tensorflow/lite/c/common.h"
#if __ANDROID__
#include <sys/system_properties.h>

#if MTK_TFLITE_NEURON_BACKEND
#include "neuron/neuron_backend.h"
#include "neuron/neuron_builder.h"
#include "neuron/neuron_delegate.h"
#endif

#include "tensorflow/lite/delegates/gpu/delegate.h"
#include "tensorflow/lite/delegates/nnapi/nnapi_delegate.h"
#endif
#include "tensorflow/core/platform/logging.h"
#include "thread_pool.h"
#include "utils.h"

#if __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#include "tensorflow/lite/delegates/coreml/coreml_delegate.h"
#include "tensorflow/lite/delegates/gpu/metal_delegate.h"
#endif
#endif

#if MTK_TFLITE_NEURON_BACKEND
// This is a sugar pointer to the backend for allocator
static struct AdapterBackendData *neuron_backend = nullptr;
#endif

struct TFLiteBackendData {
  const char *name = "TFLite";
  const char *vendor = "Google";
  const char *accelerator = "CPU";
  TfLiteModel *model{nullptr};
  std::vector<TfLiteInterpreterOptions *> options{};
  std::vector<TfLiteInterpreter *> interpreter{};
  int32_t shards_num = 1;
  uint32_t real_batch_size = 1;
  std::unique_ptr<Threadpool> executer;
  int32_t original_tensor_size = 0;
#ifdef MTK_TFLITE_NEURON_BACKEND
  neuron_backend_ptr_t neuronBackendData{nullptr};
#endif
};

static bool backendExists = false;

static constexpr const char *const kDelegateCpu = "CPU";

#if defined(__ANDROID__)
static constexpr const char *const kDelegateGpu = "GPU";
static constexpr const char *const kDelegateNnapi = "NNAPI";
#endif

#if TARGET_OS_IPHONE
static constexpr const char *const kDelegateMetal = "Metal";
static constexpr const char *const kDelegateCoreMl = "Core ML";
#endif

#if defined(MTK_TFLITE_NEURON_BACKEND) && defined(__ANDROID__)
static int mtk_perf_handle = 0;
static bool mtk_use_gpu = false;
#endif

inline mlperf_data_t::Type TfType2Type(TfLiteType type) {
  switch (type) {
    case kTfLiteFloat32:
      return mlperf_data_t::Float32;
    case kTfLiteUInt8:
      return mlperf_data_t::Uint8;
    case kTfLiteInt8:
      return mlperf_data_t::Int8;
    case kTfLiteFloat16:
      return mlperf_data_t::Float16;
    case kTfLiteInt32:
      return mlperf_data_t::Int32;
    case kTfLiteInt64:
      return mlperf_data_t::Int64;
    default:
      LOG(ERROR) << "TfLiteType " << type << " is not supported";
      return mlperf_data_t::Float32;
  }
}

#ifdef MTK_TFLITE_NEURON_BACKEND
inline mlperf_data_t::Type NeuronType2Type(neuron_data_t::Type type) {
  switch (type) {
    case neuron_data_t::Float32:
      return mlperf_data_t::Float32;
    case neuron_data_t::Uint8:
      return mlperf_data_t::Uint8;
    case neuron_data_t::Int8:
      return mlperf_data_t::Int8;
    case neuron_data_t::Float16:
      return mlperf_data_t::Float16;
    case neuron_data_t::Int32:
      return mlperf_data_t::Int32;
    case neuron_data_t::Int64:
      return mlperf_data_t::Int64;
    default:
      printf("NeuronType %d not supported\n", type);
      return mlperf_data_t::Float32;
  }
}
#endif

size_t TFLiteNumElements(const TfLiteTensor *tensor) {
  size_t result = 1;
  for (int i = 0; i < TfLiteTensorNumDims(tensor); ++i) {
    result *= TfLiteTensorDim(tensor, i);
  }
  return result;
}

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#if __ANDROID__
bool is_emulator() {
  char ro_build_characteristics[PROP_VALUE_MAX + 1];
  if (__system_property_get("ro.build.characteristics",
                            ro_build_characteristics)) {
    char *ptr;
    ptr = strstr(ro_build_characteristics, "emulator");
    if (ptr) return true;
  }
  return false;
}
#endif

// Destroy the backend pointer and its data.
void SingleModelPipeline::backend_delete(mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;

#ifdef MTK_TFLITE_NEURON_BACKEND
  AdapterBackendData *neuron_data =
      (AdapterBackendData *)backend_data->neuronBackendData;
  neuron_backend = nullptr;
  if (delete_neuron_backend(backend_data->neuronBackendData)) {
    delete neuron_data;
    delete backend_data;
    backendExists = false;
    return;
  }
  delete neuron_data;
#endif

  TfLiteModelDelete(backend_data->model);
  for (int i = 0; i < backend_data->shards_num; i++) {
    TfLiteInterpreterOptionsDelete(backend_data->options[i]);
    TfLiteInterpreterDelete(backend_data->interpreter[i]);
  }
  delete backend_data;
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

  TFLiteBackendData *backend_data = new TFLiteBackendData();

  backendExists = true;

#ifdef MTK_TFLITE_NEURON_BACKEND
  AdapterBackendData *neuronData = new AdapterBackendData();
  neuron_backend = neuronData;

  backend_data->neuronBackendData = (neuron_backend_ptr_t)neuronData;
  LOG(INFO) << "ML-Perf Model path: " << std::string(model_path);

  if (configs->batch_size <= 1 && need_neuron_backend(model_path)) {
    create_neuron_backend(backend_data->neuronBackendData, model_path);
    return backend_data;
  }
#endif

  // Load the model.
  backend_data->model = TfLiteModelCreateFromFile(model_path);
  if (!backend_data->model) {
    LOG(ERROR) << "Failed to load model: " << model_path;
    backend_delete(backend_data);
    return nullptr;
  }

  if (configs->batch_size > 1) {
    // If we use batching, make shards_num 2
    //   if it is not specified in settings.
    // If we don't use batching, we will use shards_num=1,
    //   which is the default value.
#if defined(__ANDROID__) && defined(MTK_TFLITE_NEURON_BACKEND)
    backend_data->shards_num = 4;
#else
    backend_data->shards_num = 2;
#endif

    // TODO convert this to a member var
    for (int i = 0; i < configs->count; ++i) {
      if (strcmp(configs->keys[i], "shards_num") == 0) {
        backend_data->shards_num = atoi(configs->values[i]);
      }
    }

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
      std::unique_ptr<Threadpool>(new Threadpool(backend_data->shards_num));

  // Create interpreter options function.
  auto create_option = [&](TfLiteInterpreterOptions *&option_ptr) -> void {
    option_ptr = TfLiteInterpreterOptionsCreate();
    TfLiteDelegate *delegate = nullptr;

    // TODO convert this to a member var
    for (int i = 0; i < configs->count; ++i) {
      if (strcmp(configs->keys[i], "num_threads") == 0) {
        TfLiteInterpreterOptionsSetNumThreads(option_ptr,
                                              atoi(configs->values[i]));
      }
    }

#if __ANDROID__
    if (strcmp(configs->delegate_selected, kDelegateCpu) == 0) {
      backend_data->accelerator = "CPU";
    } else if (!is_emulator() &&
               (strcmp(configs->delegate_selected, kDelegateGpu) == 0)) {
      backend_data->accelerator = "GPU";
      auto options = TfLiteGpuDelegateOptionsV2Default();
      options.inference_priority1 = TFLITE_GPU_INFERENCE_PRIORITY_MIN_LATENCY;
      delegate = TfLiteGpuDelegateV2Create(&options);
#if MTK_TFLITE_NEURON_BACKEND
      mtk_use_gpu = true;
#endif
    } else if (strcmp(configs->delegate_selected, kDelegateNnapi) == 0) {
      backend_data->accelerator = "NPU";
      auto options = tflite::StatefulNnApiDelegate::Options();
      options.allow_fp16 = true;
      options.disallow_nnapi_cpu = true;
      delegate = new tflite::StatefulNnApiDelegate(options);
#if MTK_TFLITE_NEURON_BACKEND
    } else if (strstr(configs->accelerator, "neuron") != NULL) {
      backend_data->accelerator = "Neuron";
      auto options = TfLiteNeuronDelegateOptionsDefault();
      if (strcmp(configs->accelerator, "neuron-mdla") == 0) {
        strcpy(options.accelerator_name, "mtk-mdla");
      }
      if (configs->batch_size > 1) {
        // The kOptimizationBatchProcessor doesn't work yet.
        options.optimization_hint = 0;
      } else {
        options.optimization_hint = kOptimizationLowLatency;
      }
      if (strcmp(configs->accelerator, "neuron-no-ahwb") != 0) {
        options.use_ahwb = true;
        options.use_cacheable_buffer = true;
      }
      options.execution_preference = kTurboBoost;
      options.allow_fp16 = true;
      delegate = TfLiteNeuronDelegateCreate(&options);
#endif  // MTK_TFLITE_NEURON_BACKEND
    } else {
      LOG(ERROR) << "Unknown delegate_selected: " << configs->delegate_selected;
    }
#endif  // __ANDROID__

#if TARGET_OS_SIMULATOR
#elif TARGET_OS_IPHONE
    if (strcmp(configs->delegate_selected, kDelegateCpu) == 0) {
      backend_data->accelerator = "CPU";
    } else if (strcmp(configs->delegate_selected, kDelegateMetal) == 0) {
      backend_data->accelerator = "GPU";
      TFLGpuDelegateOptions opts{
          .allow_precision_loss = false,
          .wait_type = TFLGpuDelegateWaitType::TFLGpuDelegateWaitTypePassive,
          .enable_quantization = true,
      };
      delegate = TFLGpuDelegateCreate(&opts);
      std::cout << "Enabling Metal delegate " << delegate << "\n";
    } else if (strcmp(configs->delegate_selected, kDelegateCoreMl) == 0) {
      backend_data->accelerator = "ANE";
      TfLiteCoreMlDelegateOptions opts{
          .enabled_devices = TfLiteCoreMlDelegateAllDevices,
          .coreml_version = 3,
          .max_delegated_partitions = 0,
          .min_nodes_per_partition = 2,
      };
      delegate = TfLiteCoreMlDelegateCreate(&opts);
      std::cout << "Enabling Core ML delegate " << delegate << "\n";
    } else {
      LOG(ERROR) << "Unknown delegate_selected: " << configs->delegate_selected;
    }
#endif
    if (delegate != nullptr) {
      TfLiteInterpreterOptionsAddDelegate(option_ptr, delegate);
    } else {
      LOG(INFO) << "No delegate created.";
    }
  };

  backend_data->options.resize(backend_data->shards_num);
  backend_data->interpreter.resize(backend_data->shards_num);

  for (int k = 0; k < backend_data->shards_num; k++) {
    create_option(backend_data->options[k]);

    backend_data->interpreter[k] =
        TfLiteInterpreterCreate(backend_data->model, backend_data->options[k]);
    if (!backend_data->interpreter[k]) {
      LOG(WARNING) << "Fallback to a vanilla interpreter";
      backend_data->interpreter[k] = TfLiteInterpreterCreate(
          backend_data->model, TfLiteInterpreterOptionsCreate());
      if (!backend_data->interpreter[k]) {
        LOG(ERROR) << "Failed to create the interpreter";
        backend_delete(backend_data);
        return nullptr;
      }
    }
  }

  const int32_t input_tensor_count =
      TfLiteInterpreterGetInputTensorCount(backend_data->interpreter[0]);

  for (int shard_index = 0; shard_index < backend_data->shards_num;
       shard_index++) {
    TfLiteInterpreter *&shard = backend_data->interpreter[shard_index];

    for (int input_index = 0; input_index < input_tensor_count; input_index++) {
      TfLiteTensor *tensor =
          TfLiteInterpreterGetInputTensor(shard, input_index);

      backend_data->original_tensor_size = tensor->bytes;

      if (backend_data->real_batch_size != tensor->dims->data[0]) {
        std::vector<int32_t> dims;
        dims.resize(tensor->dims->size);
        dims[0] = backend_data->real_batch_size;
        for (int i = 1; i < tensor->dims->size; i++) {
          dims[i] = tensor->dims->data[i];
        }
        if (TfLiteInterpreterResizeInputTensor(shard, input_index, dims.data(),
                                               tensor->dims->size) !=
            kTfLiteOk) {
          LOG(ERROR) << "Failed to resize input";
          backend_delete(backend_data);
          return nullptr;
        }
      }
    }

    if (TfLiteInterpreterAllocateTensors(shard) != kTfLiteOk) {
      LOG(ERROR) << "Failed to allocate tensors";
      backend_delete(backend_data);
      return nullptr;
    }
  }

  return backend_data;
}

// Vendor name who create this backend.
const char *SingleModelPipeline::backend_vendor_name(
    mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
  return backend_data->vendor;
}

// TODO: Return the name of the accelerator.
const char *SingleModelPipeline::backend_accelerator_name(
    mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
  return backend_data->accelerator;
}

// Return the name of this backend.
const char *SingleModelPipeline::backend_name(
    mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
  return backend_data->name;
}

// Run the inference for a sample.
mlperf_status_t SingleModelPipeline::backend_issue_query(
    mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;

#ifdef MTK_TFLITE_NEURON_BACKEND
  if (neuron_issue_query(backend_data->neuronBackendData)) {
    return MLPERF_SUCCESS;
  }
#endif

  auto task = [&backend_data](int index) -> TfLiteStatus {
    return TfLiteInterpreterInvoke(backend_data->interpreter[index]);
  };

  std::vector<std::future<TfLiteStatus>> f;
  f.resize(backend_data->shards_num);
  // dispatch workers for shards
  for (int k = 1; k < backend_data->shards_num; k++) {
    f[k] = backend_data->executer->submit(task, k);
  }
  // main thread for the first shard
  if (task(0) != kTfLiteOk) {
    LOG(ERROR) << "Failed to run the inference";
    return MLPERF_FAILURE;
  }
  // sync and get result of workers
  for (int k = 1; k < backend_data->shards_num; k++) {
    if (f[k].get() != kTfLiteOk) {
      LOG(ERROR) << "Failed to run the inference";
      return MLPERF_FAILURE;
    }
  }
  return MLPERF_SUCCESS;
}

// Flush the staged queries immediately.
mlperf_status_t SingleModelPipeline::backend_flush_queries(
    mlperf_backend_ptr_t backend_ptr) {
  return MLPERF_SUCCESS;
}

// Return the number of inputs of the model.
int32_t SingleModelPipeline::backend_get_input_count(
    mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
#ifdef MTK_TFLITE_NEURON_BACKEND
  int32_t count = 0;
  if (neuron_get_in_out_count(backend_data->neuronBackendData,
                              /* isIn */ true, &count)) {
    return count;
  }
#endif
  return TfLiteInterpreterGetInputTensorCount(backend_data->interpreter[0]);
}

// Return the type of the ith input.
mlperf_data_t SingleModelPipeline::backend_get_input_type(
    mlperf_backend_ptr_t backend_ptr, int32_t i) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
#ifdef MTK_TFLITE_NEURON_BACKEND
  neuron_data_t neuronType;
  if (neuron_get_in_out_datatype(backend_data->neuronBackendData, i,
                                 /* isIn */ true, &neuronType)) {
    mlperf_data_t type;
    type.type = NeuronType2Type(neuronType.type);
    type.size = neuronType.size;
    return type;
  }
#endif
  const TfLiteTensor *tensor =
      TfLiteInterpreterGetInputTensor(backend_data->interpreter[0], i);
  mlperf_data_t type;
  type.type = TfType2Type(TfLiteTensorType(tensor));
  type.size = TFLiteNumElements(tensor);
  type.size /= backend_data->real_batch_size;
  return type;
}

// Set the data for ith input.
mlperf_status_t SingleModelPipeline::backend_set_input(
    mlperf_backend_ptr_t backend_ptr, int32_t batch_index, int32_t i,
    void *data) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
#if defined(MTK_TFLITE_NEURON_BACKEND) && defined(__ANDROID__)
  if (mtk_use_gpu) {
    static std::vector<int32_t> kParams = {
        0x00414000,  // PERF_RES_CPUFREQ_PERF_MODE
        1,
        0x0143c000,  // PERF_RES_SCHED_ISOLATION_CPU,
        128,
        0x01000000,  // PERF_RES_DRAM_OPP_MIN,
        0};

    // mtk_perf_handle =
    //    acquirePerformanceLock(mtk_perf_handle, FAST_SINGLE_ANSWER_MODE,
    //    2000);
    mtk_perf_handle = acquirePerfParamsLock(mtk_perf_handle, 2000,
                                            kParams.data(), kParams.size());
  }
#endif

#ifdef MTK_TFLITE_NEURON_BACKEND
  if (neuron_set_input(backend_data->neuronBackendData, i, data)) {
    return MLPERF_SUCCESS;
  }
#endif

  const int shard_index = batch_index / backend_data->real_batch_size;
  TfLiteTensor *tensor = TfLiteInterpreterGetInputTensor(
      backend_data->interpreter[shard_index], i);
  const int data_offset = backend_data->original_tensor_size *
                          (batch_index % backend_data->real_batch_size);
  memcpy(tensor->data.raw + data_offset, data,
         backend_data->original_tensor_size);

  return MLPERF_SUCCESS;
}

// Return the number of outputs for the model.
int32_t SingleModelPipeline::backend_get_output_count(
    mlperf_backend_ptr_t backend_ptr) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
#ifdef MTK_TFLITE_NEURON_BACKEND
  int32_t count = 0;
  if (neuron_get_in_out_count(backend_data->neuronBackendData,
                              /* isIn */ false, &count)) {
    return count;
  }
#endif
  return TfLiteInterpreterGetOutputTensorCount(backend_data->interpreter[0]);
}

// Return the type of ith output.
mlperf_data_t SingleModelPipeline::backend_get_output_type(
    mlperf_backend_ptr_t backend_ptr, int32_t i) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
#ifdef MTK_TFLITE_NEURON_BACKEND
  neuron_data_t neuronType;
  if (neuron_get_in_out_datatype(backend_data->neuronBackendData, i,
                                 /* isIn */ false, &neuronType)) {
    mlperf_data_t type;
    type.type = NeuronType2Type(neuronType.type);
    type.size = neuronType.size;
    return type;
  }
#endif
  const TfLiteTensor *tensor =
      TfLiteInterpreterGetOutputTensor(backend_data->interpreter[0], i);
  mlperf_data_t type;
  type.type = TfType2Type(TfLiteTensorType(tensor));
  type.size = TFLiteNumElements(tensor);
  type.size /= backend_data->real_batch_size;
  return type;
}

// Get the data from ith output.
mlperf_status_t SingleModelPipeline::backend_get_output(
    mlperf_backend_ptr_t backend_ptr, uint32_t batch_index, int32_t i,
    void **data) {
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
#ifdef MTK_TFLITE_NEURON_BACKEND
  if (neuron_get_output(backend_data->neuronBackendData, i, data)) {
    return MLPERF_SUCCESS;
  }
#endif
  const int shard_index = batch_index / backend_data->real_batch_size;

  const TfLiteTensor *output_tensor = TfLiteInterpreterGetOutputTensor(
      backend_data->interpreter[shard_index], i);
  batch_index %= backend_data->real_batch_size;

  int non_batch_size = 1;
  for (int i = 1; i < output_tensor->dims->size; i++) {
    non_batch_size *= output_tensor->dims->data[i];
  }

  switch (output_tensor->type) {
    case kTfLiteFloat32:
      *data = (output_tensor->data.f + (batch_index * non_batch_size));
      break;
    case kTfLiteUInt8:
      *data = (output_tensor->data.uint8 + (batch_index * non_batch_size));
      break;
    case kTfLiteInt8:
      *data = (output_tensor->data.int8 + (batch_index * non_batch_size));
      break;
    case kTfLiteFloat16:
      *data = (output_tensor->data.f16 + (batch_index * non_batch_size));
      break;
    case kTfLiteInt32:
      *data = (output_tensor->data.i32 + (batch_index * non_batch_size));
      break;
    case kTfLiteInt64:
      *data = (output_tensor->data.i64 + (batch_index * non_batch_size));
      break;
    default:
      LOG(ERROR) << "Data type not yet supported: " << output_tensor->type;
      return MLPERF_FAILURE;
  }
  return MLPERF_SUCCESS;
}

void SingleModelPipeline::backend_convert_inputs(
    mlperf_backend_ptr_t backend_ptr, int bytes, int width, int height,
    uint8_t *data) {
#ifdef MTK_TFLITE_NEURON_BACKEND
  TFLiteBackendData *backend_data = (TFLiteBackendData *)backend_ptr;
  neuron_convert_input(backend_data->neuronBackendData, bytes, (void *)data);
#endif
}

void *SingleModelPipeline::backend_get_buffer(size_t n) {
#ifdef MTK_TFLITE_NEURON_BACKEND
  if (neuron_backend != nullptr) {
    return ::operator new(n * 2);
  }
#endif
  return ::operator new(n);
}

void SingleModelPipeline::backend_release_buffer(void *p) {
  ::operator delete(p);
}

#ifdef __cplusplus
};
#endif  // __cplusplus