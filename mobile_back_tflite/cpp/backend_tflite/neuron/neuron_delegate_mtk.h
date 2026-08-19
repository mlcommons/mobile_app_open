/* Copyright 2025 The MLPerf Authors. All Rights Reserved.

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

// Declarations for MediaTek's Neuron delegate, mirroring neuron/
// neuron_delegate.h in the @neuron_delegate archive.
//
// The vendor header includes "tensorflow/lite/c/common.h", but this app links
// LiteRT rather than TensorFlow Lite. Satisfying that include by depending on
// @org_tensorflow//tensorflow/lite/c:common puts a second implementation of
// the TF Lite C API in libtfliteneuronbackend.so and the link fails on
// duplicate symbols. It cannot be redirected with an include path either:
// Bazel passes -iquote external/org_tensorflow, which wins over anything
// -isystem, and the archive's own targets are built against TensorFlow
// throughout so patching it in place breaks them.
//
// The only thing needed from that header is TfLiteDelegate, which LiteRT
// declares identically, so declare the delegate's API here against LiteRT's
// copy and leave the archive alone.
//
// Keep in sync with the archive if its pinned sha256 in WORKSPACE is ever
// bumped: NeuronDelegateOptions must match the layout the prebuilt
// libtensorflowlite_neuron_jni.so was built with.

#ifndef MLPERF_BACKEND_TFLITE_NEURON_NEURON_DELEGATE_MTK_H_
#define MLPERF_BACKEND_TFLITE_NEURON_NEURON_DELEGATE_MTK_H_

#include <memory>

#include "tflite/c/common.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

enum {
  NN_FLAG_NO_SMP = 1U << 0,
  NN_FLAG_H2O = 1U << 1,
};

enum ExecutionPreference {
  kUndefined = -1,
  kLowPower = 0,
  kFastSingleAnswer = 1,
  kSustainedSpeed = 2,
  kTurboBoost = 3,
};

enum ExecutionPriority {
  kPriorityUndefined = -1,
  kPriorityLow = 90,
  kPriorityMedium = 100,
  kPriorityHigh = 110,
};

enum OptimizationHint {
  kOptimizationNone = 0,
  kOptimizationLowLatency = 1 << 0,
  kOptimizationDeepFusion = 1 << 1,
  kOptimizationBatchProcessor = 1 << 2,
  kOptimizationDefault = kOptimizationNone,
};

typedef struct {
  // Default execution_preference = kFastSingleAnswer
  ExecutionPreference execution_preference;
  // Default execution_priority = kPriorityHigh
  ExecutionPriority execution_priority;
  // Default optimization_hint = kOptimizationDefault
  int optimization_hint;
  // Default allow_fp16 = false
  bool allow_fp16;
  // Additional system performance boost time
  // Default boost_duration = 0
  uint32_t boost_duration;
  // The nul-terminated cache dir.
  // Default to nullptr, which implies the Neuron will not try caching the
  // compilation.
  const char* cache_dir;
  // The unique nul-terminated token string.
  // Default to nullptr, which implies the Neuron will not try caching the
  // compilation. It is the caller's responsibility to ensure there is no
  // clash of the tokens.
  // NOTE: when using compilation caching, it is not recommended to use the
  // same delegate instance for multiple models.
  const char* model_token;
  // Whether to use ahwb
  bool use_ahwb;
  // Whether to use cacheable ahwb
  bool use_cacheable_buffer;
  // Set compile options
  // TODO: temporary solution to avoid pointing to garbled.
  char compile_options[200];
  // Set target device
  char accelerator_name[50];
} NeuronDelegateOptions;

// Returns a structure with the default delegate options.
NeuronDelegateOptions TfLiteNeuronDelegateOptionsDefault();

// Creates a new delegate instance that needs to be destroyed with
// `TfLiteNeuronDelegateDelete` when delegate is no longer used by TFLite.
// When `options` is set to `nullptr`, the above default values are used:
TfLiteDelegate* TfLiteNeuronDelegateCreate(
    const NeuronDelegateOptions* options);

// Destroys a delegate created with `TfLiteNeuronDelegateCreate` call.
void TfLiteNeuronDelegateDelete(TfLiteDelegate* delegate);
#ifdef __cplusplus
}
#endif  // __cplusplus

// A convenient wrapper that returns C++ std::unique_ptr for automatic memory
// management.
inline std::unique_ptr<TfLiteDelegate, void (*)(TfLiteDelegate*)>
TfLiteNeuronDelegateCreateUnique(const NeuronDelegateOptions* options) {
  return std::unique_ptr<TfLiteDelegate, void (*)(TfLiteDelegate*)>(
      TfLiteNeuronDelegateCreate(options), TfLiteNeuronDelegateDelete);
}

#endif  // MLPERF_BACKEND_TFLITE_NEURON_NEURON_DELEGATE_MTK_H_
