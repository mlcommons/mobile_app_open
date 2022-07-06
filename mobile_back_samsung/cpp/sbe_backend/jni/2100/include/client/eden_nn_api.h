/*
 * Copyright (C) 2018 Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or
 * distributed, transmitted, transcribed, stored in a retrieval system or
 * translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung
 * Electronics.
 */

/**
 * @file    eden_nn_api.h
 * @brief   This is common EDEN NN API
 * @details This header defines EDEN NN API.
 *          They are compatible with C-language.
 * @version 0.1 Basic scenario support.
 *          Supported functions are as below.
 *          NnRet Initialize(void)
 *          NnRet OpenModel(EdenModelFile* modelFile, uint32_t* modelId)
 *          NnRet ExecuteModel(EdenRequest request, uint32_t* requestId);
 *          NnRet CloseModel(uint32_t modelId)
 * @version 0.2 Basic scenario support & modified
 *          Supported functions are as below.
 *          NnRet Initialize(void)
 *          NnRet OpenModel(EdenModelFile* modelFile, uint32_t* modelId,
 EdenPreference preference)
 *          NnRet AllocateInputBuffers(uint32_t modelId, EdenBuffer** buffers,
 int32_t* numOfBuffers)
 *          NnRet AllocateOutputBuffers(uint32_t modelId, EdenBuffer** buffers,
 int32_t* numOfBuffers)
 *          NnRet ExecuteModel(EdenRequest request, uint32_t* requestId,
 EdenPreference preference);
 *          NnRet FreeBuffers(uint32_t modelId, EdenBuffer* buffers);
 *          NnRet CloseModel(uint32_t modelId)
 *          NnRet Shutdown(void)
 * @version 0.6 ~ 0.8 Basic scenario support & modified
 *          Supported functions are as below.
 *          NnRet Initialize(void)
 *          NnRet OpenModel(EdenModelFile* modelFile, uint32_t* modelId,
 EdenPreference preference)
 *          NnRet AllocateInputBuffers(uint32_t modelId, EdenBuffer** buffers,
 int32_t* numOfBuffers)
 *          NnRet AllocateOutputBuffers(uint32_t modelId, EdenBuffer** buffers,
 int32_t* numOfBuffers)
 *          NnRet ExecuteModel(EdenRequest request, uint32_t* requestId,
 EdenPreference preference);
 *          NnRet FreeBuffers(uint32_t modelId, EdenBuffer* buffers);
 *          NnRet CloseModel(uint32_t modelId)
 *          NnRet Shutdown(void)

 */

#ifndef NN_INCLUDE_EDEN_NN_API_H_
#define NN_INCLUDE_EDEN_NN_API_H_

#include <cstdint>  // int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t
// nn
#include "eden_nn_types.h"  // NnRet
// common
#include "eden_types.h"  // EdenModelFile, EdenPrefernece, EdenBuffer, EdenRequest
// osal
#include "osal_types.h"  // addr_t

#ifdef __cplusplus
extern "C" {
#endif

// Public functions

/**
 * @brief Initialize EDEN NN.
 * @details This function initializes the EDEN NN framework.
 *          Internal data structure and related resource preparation is taken
 * place.
 * @param void
 * @returns return code
 */
NnRet Initialize(void);

/**
 * @brief Initialize EDEN NN with specific target device
 * @details This function initializes the EDEN NN framework.
 *          Internal data structure and related resource preparation is taken
 * place.
 * @param [in] target It represents for a specific target device.
 * @returns return code
 */
NnRet InitializeTarget(uint32_t target);

/**
 * @brief [Deprecated] Load a model file and construct in-memory model structure
 * @details Deprecated function.
 */
NnRet OpenModel(EdenModelFile* modelFile, uint32_t* modelId,
                EdenPreference preference);

/**
 * @brief Load a model file and construct in-memory model structure
 * @details This function reads a model file and construct an in-memory model
 * structure. The model file should be one of the supported model file format.
 *          Once EDEN NN succeeds in parsing a given model file,
 *          unique model id is returned via modelId.
 * @param[in] modelFile It represents for EDEN model file such as file path.
 * @param[out] modelId It represents for constructed EdenModel with a unique id.
 * @param[in] options It represents for a model options.
 * @returns return code
 */
NnRet OpenEdenModel(EdenModelFile* modelFile, uint32_t* modelId,
                    EdenModelOptions& options);

/**
 * @brief [Deprecated] Read a in-memory model on address and open it as a
 * EdenModel
 * @details Deprecated function.
 */
NnRet OpenModelFromMemory(ModelTypeInMemory modelTypeInMemory, int8_t* addr,
                          int32_t size, bool encrypted, uint32_t* modelId,
                          EdenPreference preference);

/**
 *  @brief Read a in-memory model on address and open it as a EdenModel
 *  @details This function reads a in-memory model on a given address and
 * convert it to EdenModel. The in-memory model should be one of the supported
 * model type in memory. Once it successes to parse a given in-memory model,
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
NnRet OpenEdenModelFromMemory(ModelTypeInMemory modelTypeInMemory, int8_t* addr,
                              int32_t size, bool encrypted, uint32_t* modelId,
                              EdenModelOptions& options);

/**
 * @brief Allocate a buffer to execute a model
 * @details This function allocates an efficient buffer to execute a buffer.
 * @param[in] modelId The model id to be applied by.
 * @param[out] buffers Array of EdenBuffers for input
 * @param[out] numOfBuffers # of buffers
 * @returns return code
 */
NnRet AllocateInputBuffers(uint32_t modelId, EdenBuffer** buffers,
                           int32_t* numOfBuffers);

/**
 * @brief Allocate a buffer to execute a model
 * @details This function allocates an efficient buffer to execute a buffer.
 * @param[in] modelId The model id to be applied by.
 * @param[out] buffers Array of EdenBuffers for output
 * @param[out] numOfBuffers # of buffers
 * @returns return code
 */
NnRet AllocateOutputBuffers(uint32_t modelId, EdenBuffer** buffers,
                            int32_t* numOfBuffers);

/**
 * @brief Load the buffers of user to execute a model
 * @details This function loads the user buffers for execution
 * @param[in] modelId The model id to be applied by
 * @param[in] Array of Userbuffers for input
 * @param[in] numOfBuffers # of buffers.
 * @param[out] buffers Array of EdenBuffers for input
 * @returns return code
 */
NnRet LoadInputBuffers(uint32_t modelId, UserBuffer* userBuffers,
                       int32_t numOfBuffers, EdenBuffer** edenBuffers);

/**
 * @brief Load the buffers of user to execute a model
 * @details This function loads the user buffers for execution
 * @param[in] modelId The model id to be applied by
 * @param[in] Array of Userbuffers for output
 * @param[in] numOfBuffers # of buffers.
 * @param[out] buffers Array of EdenBuffers for output
 * @returns return code
 */
NnRet LoadOutputBuffers(uint32_t modelId, UserBuffer* userBuffers,
                        int32_t numOfBuffers, EdenBuffer** edenBuffers);

/**
 * @brief [Deprecated] Execute a model with given buffers in nonblocking mode.
 * @details Deprecated function.
 */
NnRet ExecuteModel(EdenRequest* request, addr_t* requestId,
                   EdenPreference preference);

/**
 * @brief Execute a model with given buffers in nonblocking mode.
 * @details This function executes a model with input/output buffers.
 *          Internally EDEN NN creates a request to execute a model with
 * buffers, and this request is tagged with an unique id. This unique id is
 * returned to caller via requestId. When the execution is complete, the
 * callback's notify is executed by EDEN NN.
 * @param[in] request It represents for eden model, input/output buffers and
 * callback.
 * @param[out] requestId Unique id representing for this request.
 * @param[in] options It represents for a request options.
 * @returns return code
 */
NnRet ExecuteEdenModel(EdenRequest* request, addr_t* requestId,
                       const EdenRequestOptions& options);

/**
 * @brief Release a buffer allocated by Eden framework
 * @details This function releases a buffer returned by AllocateXXXBuffers.
 * @param[in] modelId The model id to be applied by.
 * @param[in] buffers Buffer pointer allocated by AllocateXXXBuffers
 * @returns return code
 */
NnRet FreeBuffers(uint32_t modelId, EdenBuffer* buffers);

/**
 * @brief Close a model
 * @details This function releases a model related resources and destroies a
 * model.
 * @param[in] modelId The model id to be applied by.
 * @returns return code
 */
NnRet CloseModel(uint32_t modelId);

/**
 * @brief Shutdown NN framework
 * @details This function destorys the Eden NN framework.
 *          Eden NN lets Eden Runtime know there is no more NN activity.
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
 *  @brief Get Eden version
 *  @details This function gets EdenVersion with a current EDEN framework
 * version. It includes hardware and software version too.
 *  @param[in] version It is representing for EDEN version information.
 *             This function is returned with version filled with a current
 * information.
 *  @returns return code
 */
NnRet GetEdenVersion(uint32_t modelId, int32_t* versions);

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
NnRet GetCompileVersion(uint32_t modelId, EdenModelFile* modelFile,
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
NnRet GetCompileVersionFromMemory(ModelTypeInMemory typeInMemory, int8_t* addr,
                                  int32_t size, bool encrypted,
                                  char versions[][VERSION_LENGTH_MAX]);

const int NPU_INSTANCE = 8;
const int DSP_INSTANCE = 4; // temporary
const int MAX_INSTANCE = 12; // temporary
const uint32_t EDEN_NN_TIMEOUT = UINT32_MAX;
const int NUM_NPUDSP_HEADER = 16;

#ifdef __cplusplus
}
#endif

#endif  // NN_INCLUDE_EDEN_NN_API_H_
