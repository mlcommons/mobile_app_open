/* Copyright 2018 The MLPerf Authors. All Rights Reserved.
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

#ifndef NN_INCLUDE_EDEN_NN_BENCHMARK_API_H_
#define NN_INCLUDE_EDEN_NN_BENCHMARK_API_H_

#include <cstdint>  // int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t

#include "eden_nn_types.h"  // NnRet
#include "eden_types.h"  // EdenModelFile, EdenPrefernece, EdenBuffer, EdenRequest
#include "osal_types.h"  // addr_t

typedef struct __BatchBuffer {
  char* addr;
  int32_t size;
  int32_t batchN;
} BatchBuffer;

typedef struct __EdenBatchRequest {
  uint32_t modelId;           /*!< model id to invoke */
  BatchBuffer* inputBuffers;  /*!< input buffer set information */
  BatchBuffer* outputBuffers; /*!< output buffer set information */
  EdenCallback* callback;     /*!< callback */
  HwPreference hw;
} EdenBatchRequest;

typedef struct __BatchBufferBoard {
  uint32_t modelId;
  int32_t occupied;
  HwPreference hw;
  EdenBuffer* inputBuffer;
  EdenBuffer* outputBuffer;
} BatchBufferBoard;

#ifdef __cplusplus
extern "C" {
#endif
NnRet Initialize(void);
NnRet OpenModel(EdenModelFile* modelFile, uint32_t* modelId,
                EdenPreference preference);
NnRet OpenModelFromMemory(ModelTypeInMemory modelTypeInMemory, int8_t* addr,
                          int32_t size, bool encrypted, uint32_t* modelId,
                          EdenPreference preference);
NnRet AllocateInputBuffers(uint32_t modelId, EdenBuffer** buffers,
                           int32_t* numOfBuffers);
NnRet AllocateOutputBuffers(uint32_t modelId, EdenBuffer** buffers,
                            int32_t* numOfBuffers);
NnRet ExecuteModel(EdenRequest* request, addr_t* requestId,
                   EdenPreference preference);

NnRet SetBatchMode(uint32_t modelId, uint32_t batchNumber,
                   EdenPreference preference);
NnRet ExecuteModelBatch(EdenBatchRequest* request, addr_t* requestId,
                        EdenPreference preference);

NnRet FreeBuffers(uint32_t modelId, EdenBuffer* buffers);
NnRet CloseModel(uint32_t modelId);
NnRet Shutdown(void);

NnRet GetInputBufferShape(uint32_t modelId, int32_t inputIndex, int32_t* width,
                          int32_t* height, int32_t* channel, int32_t* number);
NnRet GetOutputBufferShape(uint32_t modelId, int32_t outputIndex,
                           int32_t* width, int32_t* height, int32_t* channel,
                           int32_t* number);

#ifdef __cplusplus
}
#endif

#endif  // NN_INCLUDE_EDEN_NN_API_H_