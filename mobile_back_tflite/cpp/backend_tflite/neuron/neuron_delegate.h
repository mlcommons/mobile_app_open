/*
* Copyright (C) 2021 MediaTek Inc., this file is modified on 02/26/2021
* by MediaTek Inc. based on MIT License .
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the ""Software""), to
* deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED ""AS IS"", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#ifndef TENSORFLOW_LITE_EXPERIMENTAL_DELEGATES_NEURON_NEURON_DELEGATE_H_
#define TENSORFLOW_LITE_EXPERIMENTAL_DELEGATES_NEURON_NEURON_DELEGATE_H_

#include <memory>

#include "tensorflow/lite/c/common.h"

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

#endif  // TENSORFLOW_LITE_EXPERIMENTAL_DELEGATES_NEURON_NEURON_DELEGATE_H_
