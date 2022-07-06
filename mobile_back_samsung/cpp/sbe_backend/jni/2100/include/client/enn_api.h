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

#ifndef NN_INCLUDE_ENN_API_H_
#define NN_INCLUDE_ENN_API_H_

#include <cstdint>  // int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t
// nn
#include "eden_nn_types.h"  // NnRet
// common
#include "enn_types.h"  // EnnModelFile, EnnPrefernece, EnnBuffer, EnnRequest
//#include "eden_types.h"  // EnnModelFile, EnnPrefernece, EnnBuffer, EnnRequest
// osal
#include "osal_types.h"  // addr_t

#ifdef __cplusplus
extern "C" {
#endif

// Public functions

/**
 * @brief Initialize Exynos NN.
 * @details This function initializes the Exynos NN framework.
 *          Internal data structure and related resource preparation is taken
 * place.
 * @param void
 * @returns return code
 */
NnRet Initialize(void);

/**
 * @brief Initialize Exynos NN with specific target device
 * @details This function initializes the Exynos NN framework.
 *          Internal data structure and related resource preparation is taken
 * place.
 * @param [in] target It represents for a specific target device.
 * @returns return code
 */
NnRet InitializeTarget(uint32_t target);

/**
 * @brief Load a model file and construct in-memory model structure
 * @details This function reads a model file and construct an in-memory model
 * structure.
 *          The model file should be one of the supported model file format.
 *          Once Exynos NN succeeds in parsing a given model file,
 *          unique model id is returned via modelId.
 * @param[in] modelFile It represents for Exynos NN model file such as file
 * path.
 * @param[out] modelId It represents for constructed EnnModel with a unique id.
 * @param[in] options It represents for a model options.
 * @returns return code
 */
NnRet OpenEnnModel(EnnModelFile* modelFile, uint32_t* modelId,
                   EnnModelOptions& options);

/**
 *  @brief Read a in-memory model on address and open it as a EnnModel
 *  @details This function reads a in-memory model on a given address and
 * convert it to EnnModel.
 *           The in-memory model should be one of the supported model type in
 * memory.
 *           Once it successes to parse a given in-memory model,
 *           unique model id is returned via modelId.
 *  @param[in] modelTypeInMemory it is representing for in-memory model such as
 * Android NN Model.
 *  @param[in] addr address of in-memory model
 *  @param[in] size size of in-memory model
 *  @param[in] encrypted data on addr is encrypted
 *  @param[out] modelId It is representing for constructed EnnModel with a
 * unique id.
 *  @param[in] options It is representing for a model options.
 *  @returns return code
 */
NnRet OpenEnnModelFromMemory(ModelTypeInMemory modelTypeInMemory, int8_t* addr,
                             int32_t size, bool encrypted, uint32_t* modelId,
                             EnnModelOptions& options);

/**
 * @brief Allocate a buffer to execute a model
 * @details This function allocates an efficient buffer to execute a buffer.
 * @param[in] modelId The model id to be applied by.
 * @param[out] buffers Array of EnnBuffers for input
 * @param[out] numOfBuffers # of buffers
 * @returns return code
 */
NnRet AllocateInputBuffers(uint32_t modelId, EnnBuffer** buffers,
                           int32_t* numOfBuffers);

/**
 * @brief Allocate a buffer to execute a model
 * @details This function allocates an efficient buffer to execute a buffer.
 * @param[in] modelId The model id to be applied by.
 * @param[out] buffers Array of EnnBuffers for output
 * @param[out] numOfBuffers # of buffers
 * @returns return code
 */
NnRet AllocateOutputBuffers(uint32_t modelId, EnnBuffer** buffers,
                            int32_t* numOfBuffers);

/**
 * @brief Load the buffers of user to execute a model
 * @details This function loads the user buffers for execution
 * @param[in] modelId The model id to be applied by
 * @param[in] Array of Userbuffers for input
 * @param[in] numOfBuffers # of buffers.
 * @param[out] buffers Array of EnnBuffers for input
 * @returns return code
 */
NnRet LoadInputBuffers(uint32_t modelId, UserBuffer* userBuffers,
                       int32_t numOfBuffers, EnnBuffer** EnnBuffers);

/**
 * @brief Execute a model with given buffers in nonblocking mode.
 * @details This function executes a model with input/output buffers.
 *          Internally Exynos NN creates a request to execute a model with
 * buffers,
 *          and this request is tagged with an unique id.
 *          This unique id is returned to caller via requestId.
 *          When the execution is complete, the callback's notify is executed by
 * Exynos NN.
 * @param[in] request It represents for Enn model, input/output buffers and
 * callback.
 * @param[out] requestId Unique id representing for this request.
 * @param[in] options It represents for a request options.
 * @returns return code
 */
NnRet ExecuteEnnModel(EnnRequest* request, addr_t* requestId,
                      const EnnExecutionOptions& options);

/**
 * @brief Release a buffer allocated by Enn framework
 * @details This function releases a buffer returned by AllocateXXXBuffers.
 * @param[in] modelId The model id to be applied by.
 * @param[in] buffers Buffer pointer allocated by AllocateXXXBuffers
 * @returns return code
 */
NnRet FreeBuffers(uint32_t modelId, EnnBuffer* buffers);

/**
 * @brief Close an Enn model
 * @details This function releases a model related resources and destroies a
 * model.
 * @param[in] modelId The model id to be applied by.
 * @returns return code
 */
NnRet CloseEnnModel(uint32_t modelId);

/**
 * @brief Shutdown Exynos NN framework
 * @details This function destorys the Exynos NN framework.
 *          Exynos NN lets its Runtime know there is no more NN activity.
 * @param void
 * @returns return code
 */
NnRet Shutdown(void);

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
NnRet GetInputBufferShape(uint32_t modelId, int32_t inputIndex, int32_t* width,
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
NnRet GetOutputBufferShape(uint32_t modelId, int32_t outputIndex,
                           int32_t* width, int32_t* height, int32_t* channel,
                           int32_t* number);

/**
 *  @brief Get Exynos NN version
 *  @details This function gets Exynos NN Version with a current Exynos
 * framework version.
 *           It includes hardware and software version too.
 *  @param[out] version It is representing for Exynos NN version information.
 *             This function is returned with version filled with a current
 * information.
 *  @returns return code
 */
NnRet GetEnnVersion(uint32_t modelId, int32_t* versions);

/**
 *  @brief Get model compile version
 *  @details This function gets compile version
 *  @param[in] modelId The model id to be applied by.
 *  @param[in] modelFile It is representing for Exynos NN model file such as
 * file path.
 *  @param[out] version It is representing for compile version information.
 *         This function is returned with version filled with a current
 * information.
 *  @returns return code
 */
NnRet GetCompileVersions(uint32_t modelId, EnnModelFile* modelFile,
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
NnRet GetCompileVersionsFromMemory(ModelTypeInMemory typeInMemory, int8_t* addr,
                                   int32_t size, bool encrypted,
                                   char versions[][VERSION_LENGTH_MAX]);

#ifdef __cplusplus
}
#endif

#endif  // NN_INCLUDE_ENN_API_H_
