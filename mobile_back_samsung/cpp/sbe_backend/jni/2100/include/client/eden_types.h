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
 * @file    eden_types.h
 * @brief   This is common EDEN data structure types.
 * @details This header defines EDEN data structure used by EDEN framework.
 *          They are compatible with C-language.
 * @author  minsu.jeon (minsu.jeon@samsung.com)
 */

#ifndef COMMON_EDEN_TYPES_H_
#define COMMON_EDEN_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

// osal
#include "osal_types.h"  // osal data type

#if defined(__ANDROID__)
typedef _Float16 _Float16_t;
#elif (__linux__)
typedef float _Float16_t;
#endif

#define INVALID_MODEL_ID 0xffffffff
#define INVALID_REQUEST_ID 0xffffffff

#define TARGET_ALL 0x00000000
#define TARGET_CPU 0x00000001
#define TARGET_GPU 0x00000002
#define TARGET_NPU 0x00000004
#define TARGET_DSP 0x00000008

#define NPU_BOUND_CORE0 0x00000000
#define NPU_UNBOUND 0xFFFFFFFF
#define BOUND_NA 0xFFFFFFFF

#define VERSION_LENGTH_MAX 256

typedef enum {
  DevicesType_NONE = 0,
  DevicesType_NPU,
  DevicesType_GPU,
  DevicesType_CPU,
  DevicesType_DSP,
  DevicesType_COUNT,
} DevicesType;

/*! User Preference */
typedef enum {
  ALL_HW = 0,
  NPU_ONLY,
  GPU_ONLY,
  CPU_ONLY,
  DSP_ONLY,
  HWCOUNT,
} HwPreference;

typedef enum {
  NORMAL_MODE,
  BOOST_MODE,
  // DIRECT_MODE, // NOT SUPPORTED YET
  // LOW_POWER_MODE, // NOT SUPPORTED YET
  // HIGH_ACCURACY, // NOT SUPPORTED YET

  /**
   * BOOST_ON_EXECUTE
   * boost device clock when execute request.
   * just execute!, exclude open/close
   * deprecated
   */
  BOOST_ON_EXECUTE,
  BENCHMARK_MODE,
  RESERVED,
  MODECOUNT, /* set max limit for uninitialized preference */
} ModePreference;

typedef struct __InputBufferPreference {
  int8_t enable;
  int8_t setInputAsFloat;
} InputBufferPreference;

typedef enum {
  DEFAULT = 0,
  APP,      // SNAP or 3rd party App
  LIBRARY,  // NFD(VPL_library on Camera-Hal)
  MAX_COUNT_TYPE,
} EdenUserType;

typedef struct __EdenPreference {
  HwPreference hw;
  ModePreference mode;
  InputBufferPreference inputBufferMode;  // Used by OpenModel
} EdenPreference;

typedef enum {
  EDEN_NN_API = 0,
  ANDROID_NN_API = 1,
  CAFFE2_NN_API = 2,
  APICOUNT,
} NnApiType;

typedef struct __ModelPreference {
  EdenPreference userPreference;
  NnApiType nnApiType;
} ModelPreference;

typedef enum {
  P0 = 0,
  P1,
  P2,
  P3,
  P4,
  P_DEFAULT,
  P6,
  P7,
  P8,
  P9,
  P10
} ModelPriority;

#ifdef __cplusplus
struct EdenModelOptions {
  ModelPreference modelPreference;
  ModelPriority priority;
  uint32_t latency;
  uint32_t boundCore;
  int32_t reserved[32];
  EdenModelOptions()
      : modelPreference{},
        priority(P_DEFAULT),
        latency(0),
        boundCore(BOUND_NA),
        reserved{} {}
};
#else
typedef struct __EdenModelOptions {
  ModelPreference modelPreference;
  ModelPriority priority;
  uint32_t latency;
  uint32_t boundCore;
  int32_t reserved[32];
} EdenModelOptions;
#endif

typedef enum {
  NONE,
  BLOCK,
  NONBLOCK,
} RequestMode;

typedef struct __RequestPreference {
  EdenPreference userPreference;
} RequestPreference;

typedef struct __EdenRequestOptions {
  EdenPreference userPreference;
  RequestMode requestMode;
  int32_t reserved[32];
} EdenRequestOptions;

typedef struct __UpdatedOperations {
  int32_t numOfOperations;
  int32_t* operations;
} UpdatedOperations;

typedef struct __RequestOptions {
  RequestPreference requestPreference;
  UpdatedOperations updatedOperations;
  int32_t reserved[32];
} RequestOptions;

/*! file format */
typedef enum {
  NCP = 0,
  TFLITE = 1,
  PROTOTXT = 2,
  ONNX = 3,
  NNEF = 4,
} EdenFileFormat;

typedef struct __EdenModelFile {
  EdenFileFormat modelFileFormat; /*!< model file format (TFLITE etc) */
  int8_t* pathToModelFile;        /*!< path to model file ending with "\0" */
  int32_t lengthOfPath;           /*!< length of path string */
  int8_t*
      pathToWeightBiasFile; /*!< path to weight & bias file ending with "\0" */
  int32_t lengthOfWeightBias; /*!< length of path string */
} EdenModelFile;

typedef enum {
  MODEL_TYPE_IN_MEMORY_ANDROID_NN = 0,
  MODEL_TYPE_IN_MEMORY_TENSORFLOW = 1,
  MODEL_TYPE_IN_MEMORY_CAFFE = 2,
  MODEL_TYPE_IN_MEMORY_TFLITE = 3,
  MODEL_TYPE_IN_MEMORY_CAFFE2 = 4,
} ModelTypeInMemory;

typedef struct __EdenBuffer {
  void* addr;   /*!< buffer address */
  int32_t size; /*!< size of buffer */
} EdenBuffer;

typedef enum {
  VADDR = 0,
  FD = 1,
} UserBufferType;

typedef struct __UserBuffer {
  UserBufferType type;
  union {
    void* addr;
    int32_t fd;
  } elem;
  int32_t size;
} UserBuffer;

typedef void (*NotifyFuncPtr)(addr_t*,
                              addr_t); /*!< function pointer for notify */
typedef int32_t (*WaitForFuncPtr)(addr_t*, uint32_t,
                                  uint32_t); /*!< function pointer for wait */

typedef struct __EdenInferenceResult {
  int32_t retCode; /*!< return code */
} EdenInferenceResult;

typedef struct __EdenCallback {
  addr_t requestId; /*!< request id updated by callee */
  union {
    EdenInferenceResult inference; /*!< inference result */
    /** @todo Will be added as per release plan. */
  } executionResult;
  NotifyFuncPtr notify;   /*!< function pointer for notify */
  WaitForFuncPtr waitFor; /*!< function pointer for wait */
} EdenCallback;

typedef struct __EdenRequest {
  uint32_t modelId;          /*!< model id to invoke */
  EdenBuffer* inputBuffers;  /*!< input buffer set information */
  EdenBuffer* outputBuffers; /*!< output buffer set information */
  EdenCallback* callback;    /*!< callback */
  HwPreference hw;
} EdenRequest;

typedef struct __EdenEvent {
  int32_t event; /*! event */
  int32_t reserved01;
  int32_t reserved02;
  int32_t reserved03;
} EdenEvent;

typedef enum {
  UNKNOWN = 0,
  POWER_OFF,
  POWER_ON,
  EMERGENCY_RECOVERY,
  INITIALIZED,
  RUNNING,
  SUSPENDED,
  RESUMED,
  SHUTDOWN,
} DEVICE_STATE;

typedef enum {
  DEVICE_HEALTH_STATE_NORMAL = 0,
  DEVICE_HEALTH_STATE_NO_MORE_MODEL_OPEN = 1,
} EdenDeviceHealthState;

typedef struct __EdenState {
  DEVICE_STATE deviceState[DevicesType_COUNT];
  uint8_t numOfCores[DevicesType_COUNT];
  uint32_t reserved[3];
} EdenState;

typedef enum {
  EDEN_VERSION_MAJOR,
  EDEN_VERSION_MINOR,
  EDEN_VERSION_BUILD,
  PLATFORM_VERSION_NCP,
  PLATFORM_VERSION_SOC,
  VERSION_MAX = 10
} VERSION_INDEX;

typedef enum {
  MODEL_NPUC_VERSION = 0,
  MODEL_NCP_VERSION = 1,
  MODEL_OPT_LEVEL = 2,
  MODEL_SOC_TYPE = 3,
  MODEL_QUANT_MODE = 4,
  MODEL_QUANT_BW = 5,
  MODEL_QUANT_DEV = 6,
  MODEL_HW_CFU = 7,
  MODEL_NQ_FOLDING = 8,
  // MAX_ENUM
  COMPILE_VERSION_MAX = 10
} COMPILE_VERSION_INDEX;

#ifdef __cplusplus
}
#endif

#endif  // COMMON_EDEN_TYPES_H_
