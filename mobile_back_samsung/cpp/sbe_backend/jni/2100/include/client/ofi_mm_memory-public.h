/*
 * Copyright (C) 2018 Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or
 * distributed, transmitted, transcribed, stored in a retrieval system or
 * translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed to third parties
 * without the express written permission of Samsung Electronics.
 *
 */

#ifndef SOURCE_DSP_NN_MEMORY_MANAGER_OFI_MM_MEMORY_PUBLIC_H_
#define SOURCE_DSP_NN_MEMORY_MANAGER_OFI_MM_MEMORY_PUBLIC_H_

#include <stdlib.h>

enum MemType {
  MEM_TYPE_NONE = 0,
  MEM_TYPE_MALLOC = 1,
  MEM_TYPE_ION_INT,
  MEM_TYPE_ION_INT_CACHE_FORCE,
  MEM_TYPE_ION_EXT_UH,  // Under HIDL
  MEM_TYPE_ION_EXT,
  MEM_TYPE_MALLOC_EXT,
  MEM_TYPE_MAX,
};

/**
 * @brief MemItem_t: Basic object type of memory manager
 *
 */
typedef struct memItem_t {
  uint64_t m_id;   ///< MemoryObject ID. Address of MemItem_t
  int32_t size;    ///< request memory size
  int32_t offset;  ///< Memory offset (physical offset)
  void *va;        ///< virtual address for cpu. va may be not used in
  uint32_t MAGIC;  ///< to check integrity
} MemItem_t;

#ifdef BUILD_X86
#define MEM_TYPE_PLATFORM_DEPEND MEM_TYPE_MALLOC
#define CPU_MEM_TYPE_PLATFORM_DEPEND MEM_TYPE_MALLOC
#define DEV_MEM_TYPE_PLATFORM_DEPEND MEM_TYPE_MALLOC
#else
#define MEM_TYPE_PLATFORM_DEPEND MEM_TYPE_ION_INT
#define CPU_MEM_TYPE_PLATFORM_DEPEND MEM_TYPE_ION_INT_CACHE_FORCE
#define DEV_MEM_TYPE_PLATFORM_DEPEND MEM_TYPE_ION_INT
#endif

#endif  // SOURCE_DSP_NN_MEMORY_MANAGER_OFI_MM_MEMORY_PUBLIC_H_
