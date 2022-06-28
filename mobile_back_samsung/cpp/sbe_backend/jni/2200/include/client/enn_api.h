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
 * @file enn_api.h
 * @author Hoon Choi (hoon98.choi@)
 * @brief header file for enn API
 * @version 0.1
 * @date 2020-12-14
 */

#ifndef SRC_CLIENT_INCLUDE_ENN_API_H_
#define SRC_CLIENT_INCLUDE_ENN_API_H_

#include <iostream>
#include <memory>

#include "client/enn_api-public.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *  Data structures  *
 *********************/

/**
 * @brief execution preference
 */
typedef struct edenExecPreference {
    EnnCallbackFunctionPtr callback_func;  // callback_func if nullptr, the model
                                           // works with blocking mode. (default)
    uint32_t gpu_disable;                  // cpu only or use both cpu & gpu (high priority to
                                           // load_model)
    uint32_t reserved[13];
} EdenExecPreference;

/**
 * @brief extended preference for load model
 */
typedef struct ennModelExtendedPreference {
    enn_preset_id _base_preset;
    uint32_t MAGIC;
    uint32_t latency;       // legacy options
    uint32_t gpu_disable;   // use only cpu or both cpu & gpu
    uint32_t reserved[13];  // ( 64B aligned - 12B ) / 4 = 13
} EnnModelExtendedPreference;

/**
 * @brief Open Model with extended preference
 *
 * @param model_file model ID from load_model
 * @param preference extended preference for load model
 * @return EnnModelId
 */
extern EnnModelId EnnOpenModelExtension(const char* model_file, const EnnModelExtendedPreference& preference);

/**
 * @brief Execute Model with preference
 *
 * @param model_id model ID from load_model
 * @param preference Preference for Execution
 * @return EnnReturn zero if succussful
 */
extern EnnReturn EnnExecuteModelExtended(EnnModelId model_id, const EdenExecPreference& preference);

/**
 * @brief Execute Model with buffer set and preference
 *
 * @param model_id model ID from load_model
 * @param bufferSet Full buffer set for execution
 * @return EnnReturn zero if succussful
 */
extern EnnReturn EnnExecuteModelWithMemorySet(EnnModelId model_id, const EnnBufferSet& bufferSet);

/**
 * @brief Execute Model with buffer set and preference
 *
 * @param model_id model ID from load_model
 * @param bufferSet Full buffer set for execution
 * @param preference Preference for Execution
 * @return EnnReturn zero if successful
 */
extern EnnReturn EnnExecuteModelExtendedSetWithMemory(EnnModelId model_id, const EnnBufferSet& bufferSet,
                                                      const EdenExecPreference& preference);


/**
 * @brief Get Model information.
 *
 * @param info_id    type of information (please ref to enum EnnMetaTypeId)
 * @param output_str string output
 * @return EnnReturn   zero if successful
 */
extern EnnReturn EnnGetModelInfo(EnnMetaTypeId info_id, char output_str[ENN_INFO_GRAPH_STR_LENGTH_MAX]);

/**
 * @brief Generate buffer spaces : deprecated
 *
 * @param out_buffers allocated buffers are filled in a buffer array
 * @param out_buf_n number of allocated buffers
 * @return EnnReturn zero if successful
 */
extern EnnReturn EnnGenerateBufferSpaceDev(const EnnModelId model_id, const int n_set);


/****************************************
 * Preference Related APIs (not public) *
 ****************************************/

/* setter */
extern EnnReturn EnnSetPreferenceTargetLatency(const uint32_t val);
extern EnnReturn EnnSetPreferenceTileNum(const uint32_t val);
extern EnnReturn EnnSetPreferenceCoreAffinity(const uint32_t val);
extern EnnReturn EnnSetPreferencePriority(const uint32_t val);
extern EnnReturn EnnSetPreferenceTriggerMode(const uint32_t val);

/* getter */
extern EnnReturn EnnGetPreferenceTargetLatency(uint32_t *val_ptr);
extern EnnReturn EnnGetPreferenceTileNum(uint32_t *val_ptr);
extern EnnReturn EnnGetPreferenceCoreAffinity(uint32_t *val_ptr);
extern EnnReturn EnnGetPreferencePriority(uint32_t *val_ptr);
extern EnnReturn EnnGetPreferenceTriggerMode(uint32_t *val_ptr);

/* Reset as default */
extern EnnReturn EnnResetPreferenceAsDefault();

extern EnnReturn EnnSetFastIpc();
extern EnnReturn EnnExecuteModelFastIpc(const EnnModelId model_id, int client_sleep_usec);
extern EnnReturn EnnUnsetFastIpc();



#define ENN_BUFFER_MAX (100)
typedef struct _bufferMeta {
    enn_buf_dir_e direction;
    uint32_t index;
    uint32_t size;
    char label[ENN_SHAPE_NAME_MAX];
} BufferMeta;


#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SRC_CLIENT_INCLUDE_ENN_API_H_
