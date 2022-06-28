/* Copyright 2020-2022 Samsung Electronics Co. LTD  All Rights Reserved.
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
#ifndef EXTERN_OFI_STUB_H_
#define EXTERN_OFI_STUB_H_

/**
 * @file extern_ofi_stub.hpp
 * @brief samsung ofi wrapper for 2100.
 * @date 2022-06-09
 * @author soobong Huh (soobong.huh@samsung.com)
 */

#include <unordered_map>
#include "sbe_utils.hpp"
#include "eden_nn_api.h"
#include "eden_nn_types.h"
#include "ofi_api-public.h"

namespace ofi_stub {
  typedef enum IMAGE_FORMAT {
  // BGR by pixel order
  IMAGE_FORMAT_BGR = 0x1,
  // BGR by channel order
  IMAGE_FORMAT_BGRC,
} IMAGE_FORMAT;

  typedef struct _ofi_buf_set {
    exynos_nn_buf_dir_e dir;
    int num_buf;
    ofi_memory* base;  // base addr of buf_set
  } ofi_buf_set;

  typedef std::unordered_map<void*, ofi_buf_set> ofi_buf_set_map;
  typedef struct _exynos_info {
    uint32_t exynos_id;
    uint32_t mdl_id;
    exynos_nn_type_e type;
    int32_t eden_buffer_count;
    ofi_buf_set_map map_ofi_buf_set;
  } exynos_info;

  static std::mutex m_lock_mtx;
  static std::unordered_map<uint32_t, exynos_info*> m_info_map;

  uint32_t getUniqueModelId(void);
  NnRet update_ofi_mdl_id(ofi_model_id ofi_mdl_id, uint32_t* mdl_id);

  // APIs
  NnRet Initialize();
  NnRet OpenModel(EdenModelFile*, uint32_t*);
  NnRet OpenEdenModelFromMemory(int8_t*, int32_t, uint32_t*);
  NnRet AllocateBuffers(uint32_t, EdenBuffer**, exynos_nn_buf_dir_e);
  NnRet CopyToBuffer(EdenBuffer*, int32_t, void*, size_t, IMAGE_FORMAT);
  NnRet CopyFromBuffer(void*, EdenBuffer*, size_t);
  NnRet ExecuteModel(EdenRequest*);
  NnRet ExecuteEdenModel(EdenRequest*);
#if 0
  NnRet GetBufferShape(uint32_t modelId, int32_t index,
                                    int32_t* width, int32_t* height,
                                    int32_t* channel, int32_t* number,
                                    exynos_nn_buf_dir_e dir);
#endif
  NnRet FreeBuffers(uint32_t, EdenBuffer*);
  NnRet CloseModel(uint32_t);
  NnRet Shutdown(void);
}  // ofi_stub

#endif