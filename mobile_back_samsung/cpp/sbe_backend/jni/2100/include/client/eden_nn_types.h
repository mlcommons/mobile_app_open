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
 * @file    eden_nn_types.h
 * @brief   This is common EDEN data structure types.
 * @details This header defines EDEN data structure used by EDEN framework.
 *          They are compatible with C-language.
 * @author  minsu.jeon (minsu.jeon@samsung.com)
 */

#ifndef COMMON_EDEN_NN_TYPES_H_
#define COMMON_EDEN_NN_TYPES_H_

#define INVALID_MODEL_ID 0xffffffff
#define INVALID_REQUEST_ID 0xffffffff

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  RET_OK = 0,

  ////////////////////////////////////////////////
  ///////////////////// NN ///////////////////////
  ////////////////////////////////////////////////
  // Will be added later
  RET_EDEN_NN_IS_NULL = 1,
  RET_MODEL_ID_IS_INVALID = 2,

  // Runtime related error.
  RET_ERROR_ON_RT_INIT,
  RET_ERROR_ON_RT_OPEN_MODEL,
  RET_ERROR_ON_RT_ALLOCATE_INPUT_BUFFERS,
  RET_ERROR_ON_RT_ALLOCATE_OUTPUT_BUFFERS,
  RET_ERROR_ON_RT_EXECUTE_MODEL,
  RET_ERROR_ON_RT_FREE_BUFFERS,
  RET_ERROR_ON_RT_CLOSE_MODEL,
  RET_ERROR_ON_RT_SHUTDOWN,
  RET_ERROR_ON_RT_GET_INPUT_BUFFER_SHAPE,
  RET_ERROR_ON_RT_GET_OUTPUT_BUFFER_SHAPE,
  RET_ERROR_ON_RT_GET_VERSION,
  RET_ERROR_ON_RT_LOAD_BUFFERS,

  ////////////////////////////////////////////////
  /////////////////// MODEL///////////////////////
  ////////////////////////////////////////////////
  RET_MODEL_FILE_PATH_IS_INVALID = 1000,
  RET_MODEL_FILE_READ_IS_FAILED,
  RET_MODEL_FILE_IS_INVALID,
  RET_NO_INPUT_TENSOR_FOR_MODEL,
  RET_NO_OUTPUT_TENSOR_FOR_MODEL,
  RET_MODEL_ALREADY_HAS_VALID_MODEL_ID,

  RET_MODEL_IS_NULL,
  RET_OPERATION_ID_IS_NOT_UNIQUE,
  RET_NOT_SUPPORTED_OPERATOR,
  RET_NO_OPERATION_MATCHED_TO_ID,

  RET_INVALID_BUFFER_IS_DELIVERED_TO_BE_FREED,
  RET_NO_MATCHED_BUFFERS,
  RET_UNSUPPORTED_MEM_TYPE,
  RET_INVALID_BUFFER_LAYOUT,

  RET_INVALID_SEQUENCIAL_ORDER,
  RET_NOT_SUPPORTED_FEATURE,

  // EMA related error.
  RET_ERROR_ON_MEM_ALLOCATE,
  RET_ERROR_ON_MEM_FREE,

  // Warning code start
  RET_MODEL_ID_IS_ALREADY_REGISTERED,
  RET_MODEL_ID_IS_NOT_REGISTERED,

  RET_PARAM_INVALID,
  RET_SERVICE_NOT_AVAILABLE,

  RET_INVALID_CODE = 0xffffffff
} NnRet;

typedef enum _exynos_nn_type_e {
  EXYNOS_NN_TYPE_EDEN = 0,
  EXYNOS_NN_TYPE_OFI = 1,
  EXYNOS_NN_TYPE_BOTH = 2,
  EXYNOS_NN_TYPE_MAX,
} exynos_nn_type_e;

typedef enum _exynos_nn_device_e {
  EXYNOS_NN_TARGET_CPU = 0,
  EXYNOS_NN_TARGET_GPU,
  EXYNOS_NN_TARGET_DEVICE,
  EXYNOS_NN_TARGET_MAX,
} exynos_nn_target_e;

typedef enum exynos_nn_buf_dir_e {
  EXYNOS_NN_BUF_DIR_IN,     ///< buffer directions in a layer: Input
  EXYNOS_NN_BUF_DIR_OUT,    ///< buffer directions in a layer: Output
  EXYNOS_NN_BUF_DIR_INTER,  ///< buffer directions in a layer: Intermediate
} exynos_nn_buf_dir_e;

typedef enum exynos_nn_mode_e {
  EXYNOS_NN_NORMAL_MODE,
  EXYNOS_NN_BOOST_MODE,
  EXYNOS_NN_BOOST_ON_EXECUTE
} exynos_nn_mode_e;

typedef struct exynos_nn_memory {
  int32_t fd;
  int32_t size;
  void* address;
} exynos_nn_memory_t;

typedef struct exynos_nn_preference {
  int32_t mode;
  int32_t target;
  int8_t input_as_float;
  int32_t reserved01;
  int32_t reserved02;
  int32_t reserved03;
} exynos_nn_pref_t;

typedef int (*notify_func_ptr)(int, void**);  ///< callback prototype

typedef struct exynos_nn_callback {
  notify_func_ptr notify;
  uint32_t num_param;  ///< number of callback params
  void** param;        ///< callback param
} exynos_nn_callback_t;

typedef int
    exynos_nn_mm_opt;  ///< memory allocation option: use cache(ion only)
typedef void* exynos_nn_usr_image;  ///< image object

typedef struct exynos_nn_request {
  uint32_t exynos_id;
  exynos_nn_memory_t* inputs;
  exynos_nn_memory_t* outputs;
  exynos_nn_callback_t* callback;
  int32_t result;  ///< execution result
} exynos_nn_request_t;

typedef enum exynos_nn_model_mem_e {
  EXYNOS_NN_MODEL_TYPE_ANDROID_NN = 0,
  EXYNOS_NN_MODEL_TYPE_TENSORFLOW = 1,
  EXYNOS_NN_MODEL_TYPE_CAFFE = 2,
  EXYNOS_NN_MODEL_TYPE_TFLITE = 3,
  EXYNOS_NN_MODEL_TYPE_CAFFE2 = 4
} exynos_nn_model_mem_e;

#ifdef __cplusplus
}
#endif

#endif  // COMMON_EDEN_NN_TYPES_H_
