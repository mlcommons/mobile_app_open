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

#ifndef RUNTIME_INCLUDE_EDENRUNTIME_H_
#define RUNTIME_INCLUDE_EDENRUNTIME_H_

#include <cstdint>  // uint32_t

// common
#include "eden_types.h"  // EdenRequest, EdenEvent, EdenState
// runtime
#include "EdenRuntimeType.h"  // ModelPreference, RequestPreference, RtRet

namespace eden {
namespace nn {
class EdenModel;
}  // namespace nn
}  // namespace eden

namespace eden {
namespace rt {

/**
 *  @brief Init EDEN Runtime
 *  @details This API function initializes the CPU/GPU/NPU handler.
 *  @param void
 *  @returns return code
 */
RtRet Init(void);

/**
 * @brief Initialize EDEN NN with specific target device
 * @details This function initializes the EDEN NN framework.
 *          Internal data structure and related resource preparation is taken
 * place.
 * @param [in] target It represents for a specific target device.
 * @returns return code
 */
RtRet Init(uint32_t target);

/**
 *  @brief [Deprecated] Open EDEN Model
 *  @details This API function set a unique ID to EdenModel, preference, and
 * compute the running path.
 */
RtRet OpenModel(eden::nn::EdenModel* model, uint32_t* modelId,
                ModelPreference preference);

/**
 *  @brief Open EDEN Model
 *  @details This API function set a unique ID to EdenModel, options, and
 * compute the running path.
 *  @param[in] model it contains overall information.
 *  @param[out] modelId unique id for EDEN Model.
 *  @param[in] options it determines how to run model with options.
 *  @returns return code
 */
RtRet OpenModel(eden::nn::EdenModel* model, uint32_t* modelId,
                const EdenModelOptions& options);

/**
 * @brief [Deprecated] Open a model file and generates an in-memory model
 * structure
 * @details Deprecated function.
 */
RtRet OpenModelFromFile(EdenModelFile* modelFile, uint32_t* modelId,
                        ModelPreference preference);

/**
 * @brief Open a model file and generates an in-memory model structure
 * @details This function reads a model file and construct an in-memory model
 * structure.
 *          The model file should be one of the supported model file format.
 *          Once EDEN NN successes to parse a given model file,
 *          unique model id is returned via modelId.
 * @param[in] modelFile It is representing for EDEN model file such as file
 * path.
 * @param[out] modelId It is representing for constructed EdenModel with a
 * unique id.
 * @param[in] options It is representing for a model options.
 * @returns return code
 */
RtRet OpenModelFromFile(EdenModelFile* modelFile, uint32_t* modelId,
                        const EdenModelOptions& options);

/**
 *  @brief [Deprecated] Read a in-memory model on address and open it as a
 * EdenModel
 *  @details Deprecated function.
 */
RtRet OpenModelFromMemory(ModelTypeInMemory modelTypeInMemory, int8_t* addr,
                          int32_t size, bool encrypted, uint32_t* modelId,
                          ModelPreference preference);

/**
 *  @brief Read a in-memory model on address and open it as a EdenModel
 *  @details This function reads a in-memory model on a given address and
 * convert it to EdenModel.
 *           The in-memory model should be one of the supported model type in
 * memory.
 *           Once it successes to parse a given in-memory model,
 *           unique model id is returned via modelId.
 *  @param[in] modelTypeInMemory it is representing for in-memory model such as
 * Android NN Model.
 *  @param[in] addr address of in-memory model
 *  @param[in] size size of in-memory model
 *  @param[in] encrypted data on addr is encrypted
 *  @param[out] modelId It is representing for constructed EdenModel with a
 * unique id.
 *  @param[in] options It is representing for a model options.
 *  @returns return code
 */
RtRet OpenModelFromMemory(ModelTypeInMemory modelTypeInMemory, int8_t* addr,
                          int32_t size, bool encrypted, uint32_t* modelId,
                          const EdenModelOptions& options);

/**
 *  @brief [Deprecated] Execute EDEN Req
 *  @details Deprecated function.
 */
RtRet ExecuteReq(EdenRequest* req, EdenEvent** evt,
                 RequestPreference preference);

/**
 *  @brief Execute EDEN Req
 *  @details This API function executes EdenRequest with preference.
 *  @param[in] req It consists of EDEN Model ID, input/output buffers and
 * callback.
 *  @param[in] evt Callback function defined by User.
 *  @param[in] options it determines how to run EdenModel with options.
 *  @returns return code
 */
RtRet ExecuteReq(EdenRequest* req, EdenEvent** evt,
                 const EdenRequestOptions& options);

/**
 *  @brief Execute EDEN Request (Android NN)
 *  @details This API function executes EdenRequest with RequestOptions.
 *  @returns return code
 */
RtRet ExecuteRequest(EdenRequest* request, RequestOptions requestOptions);

/**
 *  @brief Close EDEN Model
 *  @details This API function releases resources related with the EDEN Model.
 *  @param[in] modelId It is a unique id for EDEN Model.
 *  @returns return code
 */
RtRet CloseModel(uint32_t modelId);

/**
 *  @brief Shutdown EDEN Runtime
 *  @details This API function close all EDEN Models with related resources for
 * shutdown EDEN Framework.
 *  @param void
 *  @returns return code
 */
RtRet Shutdown(void);

/**
 *  @brief Allocate a buffer for input to execute a model
 *  @details This function allocates an efficient buffer to execute a model.
 *  @param[in] modelId The model id to be applied by.
 *  @param[out] buffers Array of EdenBuffers for input
 *  @param[out] numOfBuffers # of buffers
 *  @returns return code
 */
RtRet AllocateInputBuffers(uint32_t modelId, EdenBuffer** buffers,
                           int32_t* numOfBuffers);

/**
 *  @brief Allocate a buffer for output to execute a model
 *  @details This function allocates an efficient buffer to execute a model.
 *  @param[in] modelId The model id to be applied by.
 *  @param[out] buffers Array of EdenBuffers for input
 *  @param[out] numOfBuffers # of buffers
 *  @returns return code
 */
RtRet AllocateOutputBuffers(uint32_t modelId, EdenBuffer** buffers,
                            int32_t* numOfBuffers);

/**
 * @brief Load a buffer of external to execute a model
 * @details This function loads to execute a buffer.
 * @param[in] modelId The model id to be applied by.
 * @param[in] Array of Userbuffers for input
 * @param[in] numOfBuffers # of buffers.
 * @param[out] buffers Array of EdenBuffers for input
 * @returns return code
 */
RtRet LoadInputBuffers(uint32_t modelId, UserBuffer* userBuffers,
                       int32_t numOfBuffers, EdenBuffer** edenBuffers);

/**
 * @brief Load a buffer of external to execute a model
 * @details This function loads to execute a buffer.
 * @param[in] modelId The model id to be applied by.
 * @param[in] Array of Userbuffers for output
 * @param[in] numOfBuffers # of buffers.
 * @param[out] buffers Array of EdenBuffers for output
 * @returns return code
 */
RtRet LoadOutputBuffers(uint32_t modelId, UserBuffer* userBuffers,
                        int32_t numOfBuffers, EdenBuffer** edenBuffers);

/**
 *  @brief Release a buffer allocated by Eden framework
 *  @details This function releases a buffer returned by AllocateXXXBuffers.
 *  @param[in] modelId The model id to be applied by.
 *  @param[in] buffers Buffer pointer allocated by AllocateXXXBuffers
 *  @returns return code
 */
RtRet FreeBuffers(uint32_t modelId, EdenBuffer* buffers);

/**
 *  @brief Get Processing Units state
 *  @details This API function returns state of CPU/GPU/NPU.
 *  @param[out] state
 *  @returns return code
 */
RtRet GetState(EdenState* state);

/**
 * @brief Return EMABuffer pointer match to a given EdenBuffer
 * @details This function returns a pointer of EMABuffer which is pair of a
 * given EdenBuffer.
 * @param[in] modelId EdenModel instance id.
 * @param[in] buffers pointer of EdenBuffer
 * @param[out] emaBuffers pointer of eden_memory_t
 * @returns pointer of eden_memory_t pair of a given EdenBuffer*
 */
RtRet GetMatchedEMABuffers(uint32_t modelId, EdenBuffer* buffers,
                           void** emaBuffers);

/**
 *  @brief Get the input buffer information
 *  @details This function gets buffer shape for input buffer of a specified
 * model.
 *  @param[in] modelId The model id to be applied by.
 *  @param[in] inputIndex Input index starting 0.
 *  @param[out] width Width
 *  @param[out] height Height
 *  @param[out] channel Channel
 *  @param[out] number Number
 *  @returns return code
 */
RtRet GetInputBufferShape(uint32_t modelId, int32_t inputIndex, int32_t* width,
                          int32_t* height, int32_t* channel, int32_t* number);

/**
 *  @brief Get the output buffer information
 *  @details This function gets buffer shape for output buffers of a specified
 * model.
 *  @param[in] modelId The model id to be applied by.
 *  @param[in] outputIndex Output index starting 0.
 *  @param[out] width Width
 *  @param[out] height Height
 *  @param[out] channel Channel
 *  @param[out] number Number
 *  @returns return code
 */
RtRet GetOutputBufferShape(uint32_t modelId, int32_t outputIndex,
                           int32_t* width, int32_t* height, int32_t* channel,
                           int32_t* number);

/**
 *  @brief Get Eden version
 *  @details This function gets EdenVersion with a current EDEN framework
 * version.
 *           It includes hardware and software version too.
 *  @param[out] version It is representing for EDEN version information.
 *         This function is returned with version filled with a current
 * information.
 *  @returns return code
 */
RtRet GetEdenVersion(uint32_t modelId, int32_t* versions);

/**
 *  @brief Get model compile version
 *  @details This function gets compile version
 *  @param[in] modelId The model id to be applied by.
 *  @param[in] modelFile It is representing for EDEN model file such as file
 * path.
 *  @param[out] version It is representing for compile version information.
 *         This function is returned with version filled with a current
 * information.
 *  @returns return code
 */
RtRet GetCompileVersion(uint32_t modelId, EdenModelFile* modelFile,
                        char versions[][VERSION_LENGTH_MAX]);

/**
 *  @brief Get model compile version
 *  @details This function gets compile version
 *  @param[in] modelTypeInMemory it is representing for in-memory model such as
 * Android NN Model.
 *  @param[in] addr address of in-memory model
 *  @param[in] size size of in-memory model
 *  @param[in] encrypted data on addr is encrypted
 *  @param[out] version It is representing for compile version information.
 *         This function is returned with version filled with a current
 * information.
 *  @returns return code
 */
RtRet GetCompileVersionFromMemory(ModelTypeInMemory typeInMemory, int8_t* addr,
                                  int32_t size, bool encrypted,
                                  char versions[][VERSION_LENGTH_MAX]);

/**
 *  @brief Get NCP buffer.
 *  @details This function gets NCP buffer
 *  @param[in] modelId.
 *  @param[out] ncpBuffer
 *  @returns return code
 */
RtRet GetNcpBuffer(uint32_t modelId, void*& ncpBuffer);

}  // namespace rt
}  // namespace eden

#endif  // RUNTIME_INCLUDE_EDENRUNTIME_H_
