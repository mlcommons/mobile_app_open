/**
 * Copyright (C) 2018 Samsung Electronics Co., Ltd. All Rights Reserved
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or
 * distributed, transmitted, transcribed, stored in a retrieval system or
 * translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise or disclosed to third parties
 * without the express written permission of Samsung Electronics.
 */

/**
 * @file osal_types.h
 * @brief The data type defined here will be used for all SW code
 *        development for SW Architecture Development Project.
 */
#ifndef OSAL_INCLUDE_OSAL_TYPES_H_
#define OSAL_INCLUDE_OSAL_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef int int32_t;
typedef unsigned int uint32_t;

#ifdef __LP64__
typedef long int64_t;
typedef unsigned long uint64_t;
typedef unsigned long addr_t;
#else
typedef long long int64_t;
typedef unsigned long long uint64_t;
typedef unsigned int addr_t;
#endif

/**
 *  System Specific Standard
 */
typedef uint32_t osal_ret_t;  //*< Error Code Define

enum {
  //* common
  PASS = 0,
  FAIL,
  ERR_INVALID,
  ERR_NO_ID,  //*< If there is no relevant ID.

  //* Semaphore
  ERR_NO_SEM,  //*< If there is no available semaphore to be created
  ERR_LOCKED,  //*< If the semaphore is taken and cannot be deleted.

  //* Thread
  ERR_NO_THREAD,     //*< If there is no available task to be created
  ERR_INV_PRIORITY,  //*< If the priority given is bad.

  //* Message Queue
  ERR_NO_MsgQ,   //*< If there is no available Message Queue to be created
  ERR_BIG_DATA,  //*< Size of uiSize is smaller than puiReadSize's
  //* or Size of uiSize is larger than max size of message Queue

  //* Mutex, Cond
  ERR_BUSY,
  ERR_PERM,
  ERR_DEADLK,
  ERR_TIMEOUT,

  //* Invalid Argrumens
  ERR_ARG_1,
  ERR_ARG_2,
  ERR_ARG_3,
  ERR_ARG_4,
  ERR_ARG_5
};

#ifdef __cplusplus
}
#endif

#endif  // OSAL_INCLUDE_OSAL_TYPES_H_
