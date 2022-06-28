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
 * @file enn_api-public.h
 * @author Hoon Choi (hoon98.choi@)
 * @brief public header file for enn API
 * @version ENN_API_VERSION_H 2.0
 * @date 2021-10-15
 */

#include "enn_api-type.h"

#ifndef SRC_CLIENT_INCLUDE_ENN_API_PUBLIC_H_
#define SRC_CLIENT_INCLUDE_ENN_API_PUBLIC_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************
 *  Model related APIs   *
 *************************/

/**
 * @brief Initialize Exynos NN framework
 *
 * @return EnnReturn result
 */
extern EnnReturn EnnInitialize(void);

/**
 * @brief Open Model with model_file and preference
 *
 * @param model_file output from graph-gen (ex .nnc)
 * @param preference contains preferences for executions
 * @return EnnModelId model ID. Successful if return value is a positive
 * number
 */
extern EnnReturn EnnOpenModel(const char *model_file, EnnModelId *model_id);

/**
 * @brief Open Model with {va, size} and preference
 *
 * @param model_file buffer includes output from graph-gen (ex .nnc)
 * @param preference contains preferences for executions
 * @return EnnModelId model ID. Successful if return value is a positive
 * number
 */
extern EnnReturn EnnOpenModelFromMemory(const char *va, const uint32_t size, EnnModelId *model_id);

/**
 * @brief Execute model with model ID with execute id [0]
 *
 * @param model_id model ID from load_model
 * @return EnnReturn zero if successful
 */
extern EnnReturn EnnExecuteModel(const EnnModelId model_id);

/**
 * @brief Execute the model of given model ID using default session ID.
 *        This function is returned immediately. Client must wait using EnnExecuteModelWait(const EnnModelId)
 *        if it returns ENN_RET_SUCCESS.
 *
 * @param model_id model ID to be executed.
 * @return EnnReturn ENN_RET_SUCCESS if successful.
 */
extern EnnReturn EnnExecuteModelAsync(const EnnModelId model_id);

/**
 * @brief Wait to complete of the execution invoked by EnnExecuteModelAsync(const EnnModelId).
 *        This function is blocked until the execution is complete.
 *
 * @param model_id model ID to be executed.
 * @return EnnReturn ENN_RET_SUCCESS if successful, others for failed.
 */
extern EnnReturn EnnExecuteModelWait(const EnnModelId model_id);

/**
 * @brief Close Model from Framework. Free related resources
 *
 * @param model_id model ID from load_model
 * @return EnnReturn zero if successful
 */
extern EnnReturn EnnCloseModel(const EnnModelId model_id);

/**
 * @brief Deinitialize Framework. Close target if internal reference counter is zero
 *
 * @return EnnReturn zero if successful
 */
extern EnnReturn EnnDeinitialize(void);



/**************************
 *  Memory related APIs   *
 **************************/
/**
 * @brief Allocate all buffers for single execution
 *
 * @param model_id model ID
 * @param out_buffers output: enn_buffer array
 * @param out_buffers_info {input # of buffers, output # of buffers}
 * @return EnnReturn
 */
extern EnnReturn EnnAllocateAllBuffers(const EnnModelId model_id, EnnBufferPtr **out_buffers,
                                       NumberOfBuffersInfo *out_buffers_info);

/**
 * @brief Allocate all buffers for single execution without commit
 *
 * @param model_id model ID
 * @param out_buffers output: enn_buffer array
 * @param out_buffers_info {input # of buffers, output # of buffers}
 * @return EnnReturn
 */
extern EnnReturn EnnAllocateAllBuffersWithoutCommit(const EnnModelId model_id, EnnBufferPtr **out_buffers,
                                                    NumberOfBuffersInfo *out_buffers_info);

/**
 * @brief Release(free) Listed Buffers
 *
 * @param buffers buffers to free
 * @param numOfBuffers number of allocated buffers
 * @return EnnReturn zero if successful
 */
extern EnnReturn EnnReleaseBuffers(EnnBufferPtr *buffers, const int32_t numOfBuffers);

/**
 * @brief Create Memory Buffer
 *
 * @param req_size size to request
 * @param flag ion flag
 * @return EnnBufferPtr nullptr if failed
 */
extern EnnReturn EnnCreateBuffer(const uint32_t req_size, const uint32_t flag, EnnBufferPtr *out);

/**
 * @brief Create Memory Buffer with cache enabled
 *
 * @param req_size size to request
 * @return EnnBufferPtr nullptr if failed
 */
extern EnnReturn EnnCreateBufferCache(const uint32_t req_size, EnnBufferPtr *out);

/**
 * @brief Import ION Buffer from outside
 *
 * @param fd ion fd
 * @param size size of buffer
 * @return EnnBufferPtr nullptr if failed
 */
extern EnnReturn EnnCreateBufferFromFd(const uint32_t fd, const uint32_t size, EnnBufferPtr *out);

/**
 * @brief Import ION buffer from outside (offset ++ size)
 *
 * @param fd ion fd
 * @param size size of buffer
 * @param offset start point of offset in byte.
 * @return EnnBufferPtr nullptr if failed
 * @note This API should use only non-cached buffer if auto cache coherency is
 *       not supported with your device
 */
extern EnnReturn EnnCreateBufferFromFdWithOffset(const uint32_t fd, const uint32_t size, const uint32_t offset,
                                                 EnnBufferPtr *out);

/**
 * @brief Enn Get File Descriptor From EnnBuffer
 *
 * @param buffer [IN] EnnBuffer*
 * @param fd [OUT] fill fd, if not available, fill -1
 * @ingroup api_memory
 * @return EnnReturn result, 0 is success
 */
extern EnnReturn EnnGetFileDescriptorFromEnnBuffer(EnnBufferPtr buffer, int32_t *fd);

/**
 * @brief Release(Free) EnnBuffer
 *
 * @return EnnReturn zero if successful
 */
extern EnnReturn EnnReleaseBuffer(EnnBufferPtr );



/*********************************************
 *  Set/Get Information from/to opened model *
 *********************************************/

/**
 * @brief Get buffers information from loaded model
 *
 * @param buffers_info [OUT] number of in / out buffer which caller should commit.
 * @param model_id [IN] model id from OpenModel()
 * @return EnnReturn result
 */
extern EnnReturn EnnGetBuffersInfo(const EnnModelId model_id, NumberOfBuffersInfo *buffers_info);

/* Model Buffer related APIs */

/**
 * @brief Set memory object to commit-space.
 * A user can generates buffer space to commit. (Basically the framework generates 16 spaces)
 * "Set Buffer" means a caller can put its memory object to it's space.
 * "Commit" means send memory-buffer set which can run opened model completely to service core.
 * @param model_id [IN] model ID from load_model
 * @param direction [IN] Direction (IN/OUT)
 * @param index [IN] index number of buffer
 * @param buf [IN] memory object from EnnCreateBufferXXX()
 * @return EnnReturn result, 0 is success
 */
extern EnnReturn EnnSetBufferByIndex(const EnnModelId model_id, const enn_buf_dir_e direction, const uint32_t index,
                                     EnnBufferPtr buf);

/**
 * @brief Set buffer with Label in model
 *
 * @param model_id model ID from load_model
 * @param label string pointer
 * @param buf buffer to set
 * @return EnnReturn zero if successful
 */
extern EnnReturn EnnSetBufferByLabel(const EnnModelId model_id, const char *label, EnnBufferPtr buf);

/**
 * @brief Set all ext-buffers for model
 *
 * @param model_id model ID from load_model
 * @param EnnBuffer** EnnBufferPtr array to set. buffers should be arranged by in->out->ext order.
 * @param sum_ioe sum of numbers of input, output and ext buffers.
 * @return EnnReturn zero if successful
 */
extern EnnReturn EnnSetBuffers(const EnnModelId model_id, EnnBufferPtr *bufs, const int32_t sum_ioe);

/**
 * @brief Get Buffer shape with Index
 *
 * @param model_id model ID from load_model
 * @param direction direction (IN, OUT)
 * @param index buffer's index number in model
 * @param out_shape shape to fill from framework
 * @return EnnReturn zero if successful
 */
extern EnnReturn EnnGetBufferInfoByIndex(const EnnModelId model_id, const enn_buf_dir_e direction, const uint32_t index,
                                         EnnBufferInfo *out_buf_info);
/**
 * @brief Get Buffer shape with Label
 *
 * @param model_id model ID from load_model
 * @param label string pointer
 * @param out_shape shape to fill from framework
 * @return EnnReturn  zero if successful
 */
extern EnnReturn EnnGetBufferInfoByLabel(const EnnModelId model_id, const char *label, EnnBufferInfo *out_buf_info);



/*************************
 *  Commit Related APIs  *
 *************************/

/**
 * @brief Generate buffer spaces : deprecated
 *
 * @param out_buffers allocated buffers are filled in a buffer array
 * @param out_buf_n number of allocated buffers
 * @return EnnReturn zero if successful
 */
extern EnnReturn EnnGenerateBufferSpace(const EnnModelId model_id, const int n_set);

/**
 * @brief Commit user-defined buffers that a context holds. For simple API, you can commit only one buffer set.
 *
 * @param model_id model ID that returns from OpenModel()
 * @return EnnExecuteModelId
 */
extern EnnReturn EnnBufferCommit(const EnnModelId model_id);



/*********************************************
 * Multiple session related APIs (extension) *
 *********************************************/

/**
 * @brief Commit Buffer with selective session
 *
 * @param model_id model ID from load_model
 * @param session_id session ID in user's session
 * @return EnnExecuteModelId zero if successful
 */
extern EnnReturn EnnBufferCommitWithSessionId(const EnnModelId model_id, const int session_id);

/**
 * @brief Execute Model with selective session
 *
 * @param model_id model ID from load model
 * @param session_id session ID in user's session
 * @return EnnReturn zero if successful
 */
extern EnnReturn EnnExecuteModelWithSessionId(const EnnModelId model_id, const int session_id);

/**
 * @brief Set memory object to commit-space.
 * A user can generates buffer space to commit. (Basically the framework generates 16 spaces)
 * "Set Buffer" means a caller can put its memory object to it's space. "Commit" means send
 * "Commit" means send memory-buffer set which can run opened model completely to service core.
 * @param model_id [IN] model ID from load_model
 * @param direction [IN] Direction (IN/OUT)
 * @param index [IN] index number of buffer
 * @param buf [IN] memory object from EnnCreateBufferXXX()
 * @param session_id [IN] If a caller generates 2 or more buffer space, session_id can be an identifier
 * @return EnnReturn result, 0 is success
 */
extern EnnReturn EnnSetBufferByIndexWithSessionId(const EnnModelId model_id, const enn_buf_dir_e direction,
                                                  const uint32_t index, EnnBufferPtr buf, const int session_id);
/**
 * @brief Set buffer with Label in model
 *
 * @param model_id model ID from load_model
 * @param label string pointer
 * @param buf buffer to set
 * @param session_id session id (starts from zero)
 * @return EnnReturn zero if successful
 */
extern EnnReturn EnnSetBufferByLabelWithSessionId(const EnnModelId model_id, const char *label, EnnBufferPtr buf,
                                                  const int session_id);

/**
 * @brief Set all ext-buffers for model
 *
 * @param model_id model ID from load_model
 * @param EnnBuffer** EnnBufferPtr array to set. buffers should be arranged by in->out->ext order.
 * @param sum_ioe sum of numbers of input, output and ext buffers.
 * @param session_id session id (starts from zero)
 * @return EnnReturn zero if successful
 */
extern EnnReturn EnnSetBuffersWithSessionId(const EnnModelId model_id, EnnBufferPtr *bufs, const int32_t sum_io,
                                            const int session_id);

/**
 * @brief Allocate all buffers for single execution
 *
 * @param model_id model ID
 * @param out_buffers output: enn_buffer array
 * @param out_buffers_info {input # of buffers, output # of buffers}
 * @param session_id session id (starts from zero)
 * @param do_commit commit after allocation automatically
 * @return EnnReturn
 */
extern EnnReturn EnnAllocateAllBuffersWithSessionId(const EnnModelId model_id, EnnBufferPtr **out_buffers,
                                                    NumberOfBuffersInfo *out_buffers_info, const int session_id,
                                                    const bool do_commit);

/**
 * @brief Execute the model of given model ID using given session ID.
 *        This function is returned immediately. Client must wait using EnnExecuteModelWait(const EnnModelId, const int)
 *        if it returns ENN_RET_SUCCESS.
 *
 * @param model_id model ID to be executed.
 * @param session_id session ID in user's session.
 * @return EnnReturn ENN_RET_SUCCESS if successful.
 */
extern EnnReturn EnnExecuteModelWithSessionIdAsync(const EnnModelId model_id, const int session_id);

/**
 * @brief Wait to complete of the execution invoked by EnnExecuteModelAsync(const EnnModelId, const int).
 *        This function is blocked until the execution is complete.
 *
 * @param model_id model ID to be executed.
 * @param session_id session ID in user's session.
 * @return EnnReturn ENN_RET_SUCCESS if successful, others for failed.
 */
extern EnnReturn EnnExecuteModelWithSessionIdWait(const EnnModelId model_id, const int session_id);



/************************
 * Secure related APIs  *
 ************************/

/**
 * @brief To change normal mode to secure mode, the framework support EnnSecureOpen()
 *
 * @param heap_size size to allocate buffer for secure space
 * @param secure_heap_addr address from secure device driver
 * @return EnnReturn ENN_RET_SUCCESS if successful, others for failed.
 */
extern EnnReturn EnnSecureOpen(const uint32_t heap_size, uint64_t* secure_heap_addr);

/**
 * @brief Close Secure mode
 *
 * @return EnnReturn ENN_RET_SUCCESS if successful, others for failed.
 */
extern EnnReturn EnnSecureClose(void);


/************************************
 * Preference Related APIs (public) *
 ************************************/

/* setter */
extern EnnReturn EnnSetPreferencePresetId(const uint32_t val);
extern EnnReturn EnnSetPreferencePerfMode(const uint32_t val);
extern EnnReturn EnnSetPreferenceTimeOut(const uint32_t val);
extern EnnReturn EnnSetPreferencePerfConfigId(const uint32_t val);

/* getter */
extern EnnReturn EnnGetPreferencePresetId(uint32_t *val_ptr);
extern EnnReturn EnnGetPreferencePerfMode(uint32_t *val_ptr);
extern EnnReturn EnnGetPreferenceTimeOut(uint32_t *val_ptr);
extern EnnReturn EnnGetPreferencePerfConfigId(uint32_t *val_ptr);


extern EnnReturn EnnDspGetSessionId(const EnnModelId model_id, int32_t *out);

/**
 * @brief Update Enn Model elements.
 * 
 * @param type  type
 * @param model_id  model_id
 * @param component  component
 * @param id  id
 * @param key  key
 * @param value_to_change_v  value_to_change_v : string array
 * @param n_param  number of array
 * @return EnnReturn zero if successful
 */
extern EnnReturn EnnExecuteCommand(const char *action, const char *model_id, const char *target_type,
                                   const char *target_id, const char *key, const char *value);


/***********************************
 * Meta data related APIs (public) *
 ***********************************/
// NOTE(hoon98.choi): If Meta information is not related to model, model_id can be ignored
extern EnnReturn EnnGetMetaInfo(const EnnMetaTypeId info_id, const EnnModelId model_id, char output_str[ENN_INFO_GRAPH_STR_LENGTH_MAX]);

/**
 * @brief Set frequency of execution message print
 * @param rate if rate is N, the exe msg shows every {1, N+1, 2N+1..} times.
 * @return EnnReturn
 */
extern EnnReturn EnnSetExecMsgAlwaysOn();
extern EnnReturn EnnSetExecMsgAlwaysOff();
extern EnnReturn EnnSetExecMsgRate(uint32_t rate);


#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SRC_CLIENT_INCLUDE_ENN_API_PUBLIC_H_
