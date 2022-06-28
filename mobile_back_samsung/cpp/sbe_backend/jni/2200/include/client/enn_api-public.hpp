/*
 * Copyright (C) 2020 Samsung Electronics Co. LTD
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
 * @file enn_api-public.hpp
 * @version ENN_API_VERSION_HPP 2.0
 * @author Hoon Choi (hoon98.choi@samsung.com)
 * @date 2021-10-15
 */

/**
 * @mainpage Introduction
 * @section ENN Framework API
 *
 * public header file for enn API supported by C++ syntax
 *
 * 1. Basically, user can simply execute model with this call flow:
 * ~~~~~~~~~~~~~~~~~~~~~
 *      EnnInitialize() ->
 *      EnnOpenModel() ->
 *      EnnAllocateAllBuffers() ->
 *      (copy input to input buffer) ->
 *      EnnExecuteModel() ->
 *      (check output from output buffer) ->
 *      EnnReleaseBuffers() ->
 *      EnnCloseModel() ->
 *      EnnDeinitialize()
 * ~~~~~~~~~~~~~~~~~~~~~
 *
 * 2. User can manually set each memory buffer as this flow:
 * ~~~~~~~~~~~~~~~~~~~~~
 *      EnnInitialize() ->
 *      EnnOpenModel() ->
 *      (Get Buffer Information with EnnGetBuffersInfo(), EnnGetBufferInfoByXXX()) ->
 *      (Create In/Out Buffers with EnnCreateBuffer(), EnnCreateBufferFromFd()) ->
 *      (Set Buffer for commit with EnnSetBufferByIndex(), EnnSetBufferByLabel()) ->
 *      EnnBufferCommit() ->
 *      EnnExecuteModel() ->
 *      (Release Buffers with EnnReleaseBuffer()) ->
 *      EnnCloseModel() ->
 *      EnnDeinitialize()
 * ~~~~~~~~~~~~~~~~~~~~~
 *
 *
 * 3. User can set multiple set of execution buffer as flow:
 * ( With these APIs, the user manages buffer set with circular queue )
 * ~~~~~~~~~~~~~~~~~~~~~
 *      EnnInitialize() ->
 *      EnnOpenModel() ->
 *      (Get Buffer Information with EnnGetBuffersInfo(), EnnGetBufferInfoByXXX()) ->
 *      (Create Multiple set of Buffers with EnnCreateBuffer(), EnnCreateBufferFromFd()) ->
 *      (Set Buffer for commit to multiple session ID with EnnSetBufferByIndex(), EnnSetBufferByLabel()) ->
 *      EnnBufferCommit(...., session_id) ->
 *      EnnExecuteModel(model_id, session_id) ->
 *      (Release Buffers with EnnReleaseBuffer()) ->
 *      EnnCloseModel() ->
 *      EnnDeinitialize()
 * ~~~~~~~~~~~~~~~~~~~~~
 * > User can manage buffers pipelined and ping-pong
 * >   with import ion(dmabufheap) allocated buffer from outside
 * >   and multiple buffer spaces without memory copies
 *
 * 4. A caller also can call preference, security, version related APIs
 * ~~~~~~~~~~~~~~~~~~~~~
 *      EnnGetMetaInfo()
 *      EnnSet/GetPreferenceXXX()
 *      EnnSecureXXXX()
 *       :
 *       :
 * ~~~~~~~~~~~~~~~~~~~~~
 *
 * @version 0.1
 * @date 2021-06-03, 2021-09-03
 * @note This file wraps enn_api-public.h
 *
 */


#pragma once
#include "enn_api-type.h"

namespace enn {
namespace api {

/**
 * @defgroup api_context Context initialize / deinitialize
 */

/**
 * @ingroup api_context
 * @brief Initialize Enn Framework.
 * Framework generates context in a caller's process.
 * Context counts initialize/deinitialize pair.
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnInitialize(void);

/**
 * @ingroup api_context
 * @brief Deinitialize Enn Framework.
 * Framework degenerates context in a caller's process.
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnDeinitialize(void);

/**
 * @defgroup api_openmodel OpenModel / CloseModel related
 */

/**
 * @ingroup api_openmodel
 * @brief OpenModel with model file
 *
 * @param model_file [IN] model_file, output from graph-gen. A caller should access the file.
 * @param model_id [OUT] model_id, 64 bit unsigned int
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnOpenModel(const char *model_file, EnnModelId *model_id);

/**
 * @ingroup api_openmodel
 * @brief OpenModel from memory buffer.
 *
 * @param va [IN] address which a model loaded from
 * @param size [IN] size of the buffer
 * @param model_id [OUT] model_id, 64 bit unsigned int
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnOpenModelFromMemory(const char *va, const uint32_t size, EnnModelId *model_id);

/**
 * @ingroup api_closemodel
 * @brief close model and free all resources in OpenModel()
 *
 * @param model_id [IN] model_id from OpenModel()
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnCloseModel(const EnnModelId model_id);

/**
 * @defgroup api_memory Memory Handling
 */

/**
 * @ingroup api_memory
 * @brief Create Buffer with request size
 * support ION or dmabufheap which can be used in a device(DSP/CPU/NPU/GPU)
 * @param out [OUT] output buffer pointer. User can get va, size, offset through *out
 * @param req_size [IN] request size
 * @param is_cached [IN] flag, the buffer uses cache or not
 * @return EnnReturn
 */
EnnReturn EnnCreateBuffer(EnnBufferPtr *out, const uint32_t req_size, const bool is_cached = true);

/**
 * @brief import ION/DMABUFHEAP allocated buffer and create EnnBuffer Object
 *
 * @param out [OUT] output buffer pointer. User can get va, size, offset through *out
 * @param fd [IN] file descriptor from ion/dmabufheap allocation
 * @param size [IN] size of buffer
 * @param offset [IN] If a caller wants to make buffer from a part of fd, use this.
 * @ingroup api_memory
 * @return EnnReturn  result
 */
EnnReturn EnnCreateBufferFromFd(EnnBufferPtr *out, const uint32_t fd, const uint32_t size, const uint32_t offset = 0);

/**
 * @brief Enn Get File Descriptor From EnnBuffer
 *
 * @param buffer [IN] EnnBuffer*
 * @param fd [OUT] fill fd, if not available, fill -1
 * @ingroup api_memory
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnGetFileDescriptorFromEnnBuffer(EnnBufferPtr buffer, int32_t *fd);

/**
 * @brief Allocate all buffers which a caller should allocate
 *
 * @param model_id model_id from OpenModel()
 * @param out_buffers [OUT] pointer of EnnBuffer array
 * @param buf_info [OUT] size of the array
 * @param session_id [IN] after generate buffer space, user can set this field if session_id > 0
 * @param do_commit [IN] if true, the framework tries to commit after buffer allocation
 * @ingroup api_memory
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnAllocateAllBuffers(const EnnModelId model_id, EnnBufferPtr **out_buffers, NumberOfBuffersInfo *buf_info,
                                const int session_id = 0, const bool do_commit = true);

/**
 * @brief Release buffer array from EnnAllocatedAllBuffers()
 * This API includes releasing all elements in the array.
 * @param buffers [IN] pointer of buffer array
 * @param numOfBuffers [IN] size of bufefr array
 * @ingroup api_memory
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnReleaseBuffers(EnnBufferPtr *buffers, const int32_t numOfBuffers);

/**
 * @brief release buffer from EnnCreateBuffer()
 *
 * @param buffer [IN] buffer object from EnnCreateBuffer()
 * @ingroup api_memory
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnReleaseBuffer(EnnBufferPtr buffer);


/**
 * @defgroup api_model Setters and Getters for model
 */

/**
 * @brief Get buffers information from loaded model
 *
 * @param buffers_info [OUT] number of in / out buffer which caller should commit.
 * @param model_id [IN] model id from OpenModel()
 * @ingroup api_model
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnGetBuffersInfo(NumberOfBuffersInfo *buffers_info, const EnnModelId model_id);

/**
 * @brief Get one buffer information from loaded model
 * ```
 * typedef struct _ennBufferInfo {
 *     bool     is_able_to_update;  // this is not used
 *     uint32_t n;
 *     uint32_t width;
 *     uint32_t height;
 *     uint32_t channel;
 *     uint32_t size;
 *     const char *label;
 * } EnnBufferInfo;
 * ```
 * a caller can identify a buffer as {DIR, Index} such as {IN, 0}
 *
 * @param out_buf_info [OUT] output buffer information
 * @param model_id [IN] model ID from load_model
 * @param direction [IN] direction (IN, OUT)
 * @param index [IN] buffer's index number in model
 * @ingroup api_model
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnGetBufferInfoByIndex(EnnBufferInfo *out_buf_info, const EnnModelId model_id, const enn_buf_dir_e direction,
                                  const uint32_t index);

/**
 * @brief Get one buffer information from loaded model
 * ```
 * typedef struct _ennBufferInfo {
 *     bool     is_able_to_update;  // this is not used
 *     uint32_t n;
 *     uint32_t width;
 *     uint32_t height;
 *     uint32_t channel;
 *     uint32_t size;
 *     const char *label;
 * } EnnBufferInfo;
 * ```
 * a caller can identify a buffer as {label} or {tensor name}
 *
 * @param out_buf_info [OUT] output buffer information
 * @param model_id [IN] model ID from load_model
 * @param label [IN] label. if .nnc includes redundent label, the framework returns information of the first founded tensor.
 * C-style string type.
 * @ingroup api_model
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnGetBufferInfoByLabel(EnnBufferInfo *out_buf_info, const EnnModelId model_id, const char *label);

/**
 * @brief Set memory object to commit-space.
 * A user can generates buffer space to commit. (Basically the framework generates 16 spaces)
 * "Set Buffer" means a caller can put its memory object to it's space. "Commit" means send
 * memory-buffer set which can run opened model completely to service core.
 * @param model_id [IN] model ID from load_model
 * @param direction [IN] Direction (IN/OUT)
 * @param index [IN] index number of buffer
 * @param buf [IN] memory object from EnnCreateBufferXXX()
 * @param session_id [IN] If a caller generates 2 or more buffer space, session_id can be an identifier
 * @ingroup api_model
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnSetBufferByIndex(const EnnModelId model_id, const enn_buf_dir_e direction, const uint32_t index,
                              EnnBufferPtr buf, const int session_id = 0);

/**
 * @brief Set memory object to commit-space.
 * A user can generates buffer space to commit. (Basically the framework generates 16 spaces)
 * "Set Buffer" means a caller can put its memory object to it's space. "Commit" means send
 * memory-buffer set which can run opened model completely to service core.
 * @param model_id [IN] model ID from load_model
 * @param label [IN] label. if .nnc includes redundent label, the framework returns information of the first founded tensor.
 * C-style string type.
 * @param buf [IN] memory object from EnnCreateBufferXXX()
 * @param session_id [IN] If a caller generates 2 or more buffer space, session_id can be an identifier
 * @ingroup api_model
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnSetBufferByLabel(const EnnModelId model_id, const char *label, EnnBufferPtr buf, const int session_id = 0);

/**
 * @brief Set multiple buffers to buffer space once.
 * EnnBuffer list should have the order: [IN/0] [IN/1] ... [IN/n-1] [OUT/0] [OUT/1]... [OUT/n-1]  (Direction/index)
 * @param model_id [IN] model ID from load_model
 * @param label [IN] label. if .nnc includes redundent label, the framework returns information of the first founded tensor.
 * C-style string type.
 * @param bufs [IN] memory object list
 * @param sum_io [IN] number of bufs. (number of input + nubmer of output)
 * @param session_id [IN] If a caller generates 2 or more buffer space, session_id can be an identifier
 * @ingroup api_model
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnSetBuffers(const EnnModelId model_id, EnnBuffer **bufs, const int32_t sum_io, const int session_id = 0);

/**
 * @defgroup api_commit Commit Buffer
 */

/**
 * @brief Explicitly generats buffer space to commit. If user don't call this, the system automatically generates one space.
 * Currently n_set should be less then 16.
 * "Commit" means send memory-buffer set which can run opened model completely to service core.
 * @note This function is deprecated. Framework initializes 16 sessions by D/D restrictions.
 * @param model_id [IN] model ID from load_model
 * @param n_set [IN] Number of buffer spaces
 * @ingroup api_commit
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnGenerateBufferSpace(const EnnModelId model_id, const int n_set = 16);

/**
 * @brief Send buffer-set to service core. session_id indicates which buffer space should be sent.
 * The committed buffers are released if a caller calls CloseModel()
 * @note uncommit() is not supported now (2021.09)
 * @param model_id [IN] model ID from load_model
 * @ingroup api_commit
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnBufferCommit(const EnnModelId model_id, const int session_id = 0);

/**
 * @defgroup api_execute Execute Models
 */

/**
 * @brief Request to service core to execute model with commited buffers
 *
 * @note this function runs in block mode
 * @param model_id [IN] model ID from load_model
 * @param session_id [IN] session ID
 * @ingroup api_execute
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnExecuteModel(const EnnModelId model_id, const int session_id = 0);

/**
 * @brief Request to service core to execute model in background asynchronously
 *
 * @param model_id [IN] model ID from load_model
 * @param session_id [IN] session ID
 * @ingroup api_execute
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnExecuteModelAsync(const EnnModelId model_id, const int session_id = 0);

/**
 * @brief Wait result of calling EnnExecuteModelAsync()
 * If execution is finished, this function is returned intermediatelly
 * If not, this function would be blocked until the execution finished
 *
 * @param model_id [IN] model ID from load_model
 * @param session_id [IN] session ID
 * @ingroup api_execute
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnExecuteModelWait(const EnnModelId model_id, const int session_id = 0);

/**
 * @defgroup api_miscellaneous Security, preference, get meta information..
 */

/**
 * @brief Try to set secure mode
 *
 * @param heap_size [IN] request heap size
 * @param secure_heap_addr [OUT] 64 bit address from secure driver
 * @ingroup api_miscellaneous
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnSecureOpen(const uint32_t heap_size, uint64_t *secure_heap_addr);

/**
 * @brief close secure mode
 *
 * @ingroup api_miscellaneous
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnSecureClose(void);

/**
 * @brief Setting Preset ID for operation performance
 *
 * @param val [IN] value to set preset ID
 * @ingroup api_miscellaneous
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnSetPreferencePresetId(const uint32_t val);

/**
 * @brief Setting PerfConfig ID for operation performance
 *
 * @param val [IN] value to set PerfConfig ID
 * @ingroup api_miscellaneous
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnSetPreferencePerfConfigId(const uint32_t val);

/**
 * @brief Setting Performance Mode
 *
 * @param val [IN] value to set Performance Mode
 * @ingroup api_miscellaneous
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnSetPreferencePerfMode(const uint32_t val);

/**
 * @brief Setting Preset ID for time out
 *
 * @note in second
 * @param val [IN] value to set time out
 * @ingroup api_miscellaneous
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnSetPreferenceTimeOut(const uint32_t val);

/**
 * @brief Setting priority value for NPU
 *
 * @param val [IN] value to set NPU job priority
 * @ingroup api_miscellaneous
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnSetPreferencePriority(const uint32_t val);

/**
 * @brief Setting affinity to set NPU core operation
 *
 * @note in second
 * @param val [IN] value to set affinity
 * @ingroup api_miscellaneous
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnSetPreferenceCoreAffinity(const uint32_t val);

/**
 * @brief Get current information for Preset ID
 *
 * @param val [OUT] current value of Preset ID
 * @ingroup api_miscellaneous
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnGetPreferencePresetId(uint32_t *val_ptr);

/**
 * @brief Get current information for PerfConfig ID
 *
 * @param val [OUT] current value of PerfConfig ID
 * @ingroup api_miscellaneous
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnGetPreferencePerfConfigId(uint32_t *val_ptr);

/**
 * @brief Get current information for Performance Mode
 *
 * @param val [OUT] current value of Performance Mode
 * @ingroup api_miscellaneous
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnGetPreferencePerfMode(uint32_t *val_ptr);

/**
 * @brief Get current information for Time Out
 *
 * @param val [OUT] current value of Time Out
 * @ingroup api_miscellaneous
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnGetPreferenceTimeOut(uint32_t *val_ptr);


/**
 * @brief Get current information for NPU Priority
 *
 * @param val [OUT] current value of NPU Priority
 * @ingroup api_miscellaneous
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnGetPreferencePriority(uint32_t *val_ptr);


/**
 * @brief Get current information for NPU Core affinity
 *
 * @param val [OUT] current value of NPU Core affinity
 * @ingroup api_miscellaneous
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnGetPreferenceCoreAffinity(uint32_t *val_ptr);

/**
 * @brief Get Session ID for DSP
 *
 * @param val [OUT] current value of Time Out
 * @ingroup api_miscellaneous
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnDspGetSessionId(const EnnModelId model_id, int32_t *out);


/**
 * @brief Update Enn Model elements.
 * 
 * @param type  type
 * @param model_id  model_id
 * @param component  component
 * @param id  id
 * @param key  key
 * @param value_to_change  value_to_change
 * @return EnnReturn zero if successful
 */
extern EnnReturn EnnExecuteCommand(const char *action, const char *model_id, const char *target_type,
                                   const char *target_id, const char *key, const char *value);

/**
 * @brief Get Meta Information
 *
 * This API includes loaded model information as well as framework information
 * @param info_id info_id can be below:
 * currently, ENN_META_VERSION_FRAMEWORK, ENN_META_VERSION_COMMIT, ENN_META_VERSION_MODEL_COMPILER_NPU is done.
 * ```
 * ENN_META_VERSION_FRAMEWORK
 * ENN_META_VERSION_COMMIT
 * ENN_META_VERSION_MODEL_COMPILER_NNC
 * ENN_META_VERSION_MODEL_COMPILER_NPU
 * ENN_META_VERSION_MODEL_COMPILER_DSP
 * ENN_META_VERSION_MODEL_SCHEMA
 * ENN_META_VERSION_MODEL_VERSION
 * ENN_META_VERSION_DD
 * ENN_META_VERSION_UNIFIED_FW
 * ENN_META_VERSION_NPU_FW
 * ENN_META_VERSION_DSP_FW
 * ```
 * @param model_id
 * @param output_str
 * @ingroup api_miscellaneous
 * @return EnnReturn result, 0 is success
 */
EnnReturn EnnGetMetaInfo(const EnnMetaTypeId info_id, const EnnModelId model_id, char output_str[ENN_INFO_GRAPH_STR_LENGTH_MAX]);

/**
 * @brief Set frequency of execution message print
 * @param rate if rate is N, the exe msg shows every {1, N+1, 2N+1..} times.
 * @return EnnReturn
 */
extern EnnReturn EnnSetExecMsgAlwaysOn();
extern EnnReturn EnnSetExecMsgAlwaysOff();
extern EnnReturn EnnSetExecMsgRate(uint32_t rate);

extern EnnReturn EnnSetFastIpc();
extern EnnReturn EnnExecuteModelFastIpc(const EnnModelId model_id, int client_sleep_usec);
extern EnnReturn EnnUnsetFastIpc();

const EnnModelId INVALID_MDL = 0;
const int MAX_INSTANCE = 4;

}  // namespace api
}  // namespace enn
