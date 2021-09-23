/* Copyright (c) 2020-2021 Samsung Electronics Co., Ltd. All rights reserved.

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
#include <jni.h>
#include <unistd.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "eden_nn_api.h"
#include "eden_types.h"
#include "gpu_boost.h"
//#include "enn_api.h"
#include <android/log.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

#define NUM_OF_BUFF_MAX 16

using Initialize_fn = decltype(Initialize);
using OpenModel_fn = decltype(OpenModel);
using OpenEdenModelFromMemory_fn = decltype(OpenEdenModelFromMemory);
using AllocateInputBuffers_fn = decltype(AllocateInputBuffers);
using AllocateOutputBuffers_fn = decltype(AllocateOutputBuffers);
using ExecuteModel_fn = decltype(ExecuteModel);
using ExecuteEdenModel_fn = decltype(ExecuteEdenModel);
using GetInputBufferShape_fn = decltype(GetInputBufferShape);
using GetOutputBufferShape_fn = decltype(GetOutputBufferShape);
using FreeBuffers_fn = decltype(FreeBuffers);
using CloseModel_fn = decltype(CloseModel);
using Shutdown_fn = decltype(Shutdown);

Initialize_fn *InitializeFunc;
OpenModel_fn *OpenModelFunc;
OpenEdenModelFromMemory_fn *OpenEdenModelFromMemoryFunc;
AllocateInputBuffers_fn *AllocateInputBuffersFunc;
AllocateOutputBuffers_fn *AllocateOutputBuffersFunc;
ExecuteModel_fn *ExecuteModelFunc;
ExecuteEdenModel_fn *ExecuteEdenModelFunc;
GetInputBufferShape_fn *GetInputBufferShapeFunc;
GetOutputBufferShape_fn *GetOutputBufferShapeFunc;
FreeBuffers_fn *FreeBuffersFunc;
CloseModel_fn *CloseModelFunc;
Shutdown_fn *ShutdownFunc;

void *libeden_nn_on_direct = nullptr;

namespace EdenAI {

bool jni_Initialize();
bool jni_OpenModel(const char *pathToModelFile, int device, int batch);
void setHwPreference(int hp);
uint32_t getInputSize();
uint32_t getOutputSize();
void setInputSize(int size);
void setOutputSize(int size);

bool jni_ExecuteModelBatch(char *input, char *output, int batch,
                           bool isMobileBert);
bool jni_CloseModel();
bool jni_Shutdown();

void edenNotify(addr_t *addr, addr_t value);

int32_t edenWaitFor(addr_t *addr, uint32_t value, uint32_t timeout);

EdenModelFile makeModel(const char *pathToModelFile);

}  // namespace EdenAI
