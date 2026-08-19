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

// MediaTek's neuron_delegate.h includes "tensorflow/lite/c/common.h", but this
// app links LiteRT rather than TensorFlow Lite. LiteRT's tflite/c/common.h is
// the same header under a different path, so forward to it.
//
// This keeps a single TF Lite C API in the backend. Depending on
// @org_tensorflow//tensorflow/lite/c:common to satisfy the include instead
// links a second implementation of that API and libtfliteneuronbackend.so
// fails with duplicate symbols; patching the archive breaks its own targets,
// which are built against TensorFlow throughout.
//
// Reachable only through the include path the neuron_delegate target in this
// package adds, so nothing else sees it.

#ifndef MLPERF_NEURON_TF_COMPAT_TENSORFLOW_LITE_C_COMMON_H_
#define MLPERF_NEURON_TF_COMPAT_TENSORFLOW_LITE_C_COMMON_H_

#include "tflite/c/common.h"

#endif  // MLPERF_NEURON_TF_COMPAT_TENSORFLOW_LITE_C_COMMON_H_
