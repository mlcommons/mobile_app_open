/* Copyright 2020 The MLPerf Authors. All Rights Reserved.

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
#ifndef EXYNOS_OFI_WRAPPER_H_
#define EXYNOS_OFI_WRAPPER_H_

#include "backend_c.h"
#include "eden_nn_types.h"

NnRet ExynosOFI_Initialize();
NnRet ExynosOFI_OpenModel(EdenModelFile* modelFile, uint32_t* modelId,
                          EdenPreference preference);
NnRet ExynosOFI_OpenEdenModelFromMemory(ModelTypeInMemory modelTypeInMemory,
                                        int8_t* addr, int32_t size,
                                        bool encrypted, uint32_t* modelId,
                                        EdenModelOptions& options);
NnRet ExynosOFI_AllocateInputBuffers(uint32_t modelId, EdenBuffer** buffers,
                                     int32_t* numOfBuffers);
NnRet ExynosOFI_AllocateOutputBuffers(uint32_t modelId, EdenBuffer** buffers,
                                      int32_t* numOfBuffers);
NnRet ExynosOFI_CopyToBuffer(EdenBuffer* buffer, int32_t offset,
                             const char* input, size_t size,
                             IMAGE_FORMAT image_format);
NnRet ExynosOFI_CopyFromBuffer(char* dst, EdenBuffer* buffer, size_t size);
NnRet ExynosOFI_ExecuteModel(EdenRequest* request, addr_t* requestId,
                             EdenPreference preference);
NnRet ExynosOFI_ExecuteEdenModel(EdenRequest* request, addr_t* requestId,
                                 const EdenRequestOptions& options);
NnRet ExynosOFI_GetInputBufferShape(uint32_t modelId, int32_t inputIndex,
                                    int32_t* width, int32_t* height,
                                    int32_t* channel, int32_t* number);
NnRet ExynosOFI_GetOutputBufferShape(uint32_t modelId, int32_t outputIndex,
                                     int32_t* width, int32_t* height,
                                     int32_t* channel, int32_t* number);
NnRet ExynosOFI_FreeBuffers(uint32_t modelId, EdenBuffer* buffers);
NnRet ExynosOFI_CloseModel(uint32_t modelId);
NnRet ExynosOFI_Shutdown(void);

#endif  // EXYNOS_OFI_WRAPPER_H_
