#include <android/log.h>
#include <stdint.h>
#include <unistd.h>

#include <map>
#include <mutex>
#include <unordered_map>
#include <vector>

#define LOG_TAG "OFI"

#include "backend_c.h"
#include "eden_nn_api.h"
#include "eden_nn_types.h"
#include "eden_types.h"
#include "ofi_mm_memory-public.h"
#include "ofi_api-public.h"

// TODO:
#define OFI_NUM_BUFF (2)

#define IN_WIDTH 224
#define IN_HEIGHT 224
#define IN_CHANNEL 3
#define OUT_WIDTH 1
#define OUT_HEIGHT 1
#define OUT_CHANNEL 1001

typedef struct _ofi_buf_set {
  exynos_nn_buf_dir_e dir;
  int num_buf;
  ofi_memory* base;  // base addr of buf_set
} ofi_buf_set;

typedef std::unordered_map<void*, ofi_buf_set> ofi_buf_set_map;
typedef struct _exynos_info {
  uint32_t exynos_id;
  uint32_t model_id;
  exynos_nn_type_e type;
  int32_t eden_buffer_count;
  ofi_buf_set_map map_ofi_buf_set;
} exynos_info;

static std::mutex mutex_mExynosNN;
static std::map<uint32_t, exynos_info*> mMapTargetExynosId;

typedef int
    exynos_nn_mm_opt;  ///< memory allocation option: use cache(ion only)

uint32_t getUniqueModelId(void) {
  static uint32_t UID = 0;
  /** @todo, real unique */
  if (UID == (INVALID_MODEL_ID - 1)) {
    UID = 1;
    return UID;
  } else {
    return ++UID;
  }
}

int32_t ExynosNN_initialize() {
  int32_t ret = RET_OK;
  mMapTargetExynosId.clear();
  ret |= ofi_initialize();
  return ret;
}

int32_t ExynosNN_loadModel(const char* compiled_graph_path,
                           exynos_nn_type_e model_type,
                           exynos_nn_pref_t preference, uint32_t* exynos_id) {
  int32_t ret = RET_OK;
  switch (model_type) {
    case EXYNOS_NN_TYPE_OFI: {
      uint32_t ofi_model_id = 0;
      ofi_model_id = ofi_load_model(compiled_graph_path);
      /*
      ofi_model_id = ofi_load_model_at_service(compiled_graph_path);
      */
      if (ofi_model_id != 0) {
        exynos_info* info = new exynos_info;
        info->model_id = ofi_model_id;
        info->type = EXYNOS_NN_TYPE_OFI;
        info->exynos_id = *exynos_id = getUniqueModelId();
        mMapTargetExynosId.insert(std::make_pair(info->exynos_id, info));
      } else {
        *exynos_id = INVALID_MODEL_ID;
        ret = RET_ERROR_ON_RT_OPEN_MODEL;
        break;
      }
      ret |= ofi_execute_pre_model(ofi_model_id);
      ret |= ofi_verify_model(ofi_model_id);
      ret = RET_OK;
      break;
    }
    default: {
      *exynos_id = INVALID_MODEL_ID;
      ret = RET_ERROR_ON_RT_OPEN_MODEL;
      break;
    }
  }

  return ret;
}

int32_t ExynosNN_loadModelFromMemory(char* va, size_t size,
                                     uint32_t* exynos_id) {
  int32_t ret = RET_OK;
  ofi_model_id ofi_model_id = ofi_load_model_from_memory(va, size);
  if (ofi_model_id != 0) {
    exynos_info* info = new exynos_info;
    info->model_id = ofi_model_id;
    info->type = EXYNOS_NN_TYPE_OFI;
    info->exynos_id = *exynos_id = getUniqueModelId();
    mMapTargetExynosId.insert(std::make_pair(info->exynos_id, info));
  } else {
    *exynos_id = INVALID_MODEL_ID;
    ret = RET_ERROR_ON_RT_OPEN_MODEL;
  }
  ret |= ofi_execute_pre_model(ofi_model_id);
  ret |= ofi_verify_model(ofi_model_id);
  // MLOGV("mydbg ret=%d, model_id=%u", ret, *exynos_id);
  return ret;
}

int32_t ExynosNN_createMemory(uint32_t exynos_id, int32_t size,
                              exynos_nn_target_e mm_type,
                              exynos_nn_mm_opt mm_option,
                              exynos_nn_buf_dir_e direction,
                              exynos_nn_memory_t** mem,
                              int32_t* num_of_buffers) {
  std::lock_guard<std::mutex> lock(mutex_mExynosNN);

  int32_t ret = RET_OK;
  if (mMapTargetExynosId.count(exynos_id) == 0) {
    return RET_PARAM_INVALID;
  }
  exynos_info* info = mMapTargetExynosId.at(exynos_id);
  switch (info->type) {
    case EXYNOS_NN_TYPE_OFI: {
      uint32_t model_id = info->model_id;
      /* OFI framework allocates buffers. */
      ofi_memory* new_buffer_base;
      if (direction == EXYNOS_NN_BUF_DIR_IN) {
        ret = ofi_allocateInputs(model_id, num_of_buffers, &new_buffer_base);
      } else if (direction == EXYNOS_NN_BUF_DIR_OUT) {
        ret = ofi_allocateOutputs(model_id, num_of_buffers, &new_buffer_base);
      } else {
        ret = RET_PARAM_INVALID;
        break;
      }
      if (ret != RET_OK) {
        ret = (direction == EXYNOS_NN_BUF_DIR_IN)
                  ? (RET_ERROR_ON_RT_ALLOCATE_INPUT_BUFFERS)
                  : (RET_ERROR_ON_RT_ALLOCATE_OUTPUT_BUFFERS);
        break;
      }
      if (*num_of_buffers < 1) {
        if (direction == EXYNOS_NN_BUF_DIR_IN) {
          ofi_freeInputs(model_id, new_buffer_base);
        } else if (direction == EXYNOS_NN_BUF_DIR_OUT) {
          ofi_freeOutputs(model_id, new_buffer_base);
        }
        ret = RET_PARAM_INVALID;
        break;
      }

      exynos_nn_memory_t* exynos_nn_mem_set =
          new exynos_nn_memory_t[*num_of_buffers];

      // Insert OFI buffer_set to vector and copy all buffer's addr and size.
      for (int32_t buf_idx = 0; buf_idx < *num_of_buffers; buf_idx++) {
        exynos_nn_memory_t* tmp_mem = &(exynos_nn_mem_set[buf_idx]);
        tmp_mem->address = new_buffer_base[buf_idx]->va;
        tmp_mem->size = new_buffer_base[buf_idx]->size;
        for (int i = 0; i < *num_of_buffers; i++) {
        }
      }
      /* return mem_set to user. */
      *mem = exynos_nn_mem_set;

      /* create and save ofi_buf_set for execution. */
      ofi_buf_set new_buf_set;
      new_buf_set.dir = direction;
      new_buf_set.num_buf = *num_of_buffers;
      new_buf_set.base = new_buffer_base;
      info->map_ofi_buf_set.insert(
          std::make_pair(new_buffer_base[0]->va, new_buf_set));
      break;
    }
    case EXYNOS_NN_TYPE_BOTH:
    default:
      ret = RET_PARAM_INVALID;
      break;
  }

  return ret;
}

int32_t ExynosNN_executeModel(uint32_t exynos_id,
                              exynos_nn_request_t* exynos_request) {
  int32_t ret = RET_OK;
  if (mMapTargetExynosId.count(exynos_id) == 0 || exynos_request == nullptr) {
    return RET_PARAM_INVALID;
  }
  exynos_info* info = mMapTargetExynosId.at(exynos_id);
  switch (info->type) {
    case EXYNOS_NN_TYPE_OFI: {
      uint32_t model_id = info->model_id;
      /* attr represent OFI request. */
      ofi_model_exe_attr attr;
      auto in_buf_set =
          info->map_ofi_buf_set.find(exynos_request->inputs->address);
      if (in_buf_set == info->map_ofi_buf_set.end()) {
        ret = RET_NO_INPUT_TENSOR_FOR_MODEL;
        break;
      }
      /* Find out buffer_set. */
      auto out_buf_set =
          info->map_ofi_buf_set.find(exynos_request->outputs->address);
      if (out_buf_set == info->map_ofi_buf_set.end()) {
        ret = RET_NO_OUTPUT_TENSOR_FOR_MODEL;
        break;
      }

      attr.in_buffers = in_buf_set->second.base;
      attr.numOfIn = in_buf_set->second.num_buf;

      attr.out_buffers = out_buf_set->second.base;
      attr.numOfOut = out_buf_set->second.num_buf;

      attr.n_skip = 0;
      attr.is_wait = true;
      attr.is_async = true;

      if (exynos_request->callback) {
        // ExynosNN user-callback is directly used.
        attr.function_desc.func_p = exynos_request->callback->notify;
        attr.param.num_param = exynos_request->callback->num_param;
        attr.param.params = exynos_request->callback->param;
        // pointer to return result of execution to ExynosNN user
        attr.result = &exynos_request->result;
      } else {
        /* OFI SYNC session execution mode */
        attr.is_async = false;
        attr.function_desc.func_p = nullptr;
        attr.result = nullptr;
        attr.param.params = nullptr;
        attr.param.num_param = 0;
      }

      int* addr = (int*)exynos_request->outputs->address;
      *addr = 0;
      /* attr will be copied inside this func. */
      ret = ofi_execute_model_attr(model_id, &attr);
      break;
    }
    case EXYNOS_NN_TYPE_BOTH:
    default:
      ret = RET_PARAM_INVALID;
      break;
  }

  return ret;
}

int32_t ExynosNN_getMemoryShape(uint32_t exynos_id, exynos_nn_buf_dir_e dir,
                                int32_t index, int32_t* width, int32_t* height,
                                int32_t* channel, int32_t* number) {
  int32_t ret = RET_OK;
  if (mMapTargetExynosId.count(exynos_id) == 0) {
    return RET_PARAM_INVALID;
  }
  exynos_info* info = mMapTargetExynosId.at(exynos_id);
  switch (info->type) {
    case EXYNOS_NN_TYPE_OFI: {
      // TODO: nubmer is only revealed when allocating ofi buffers
      *number = 1;
      if (dir == EXYNOS_NN_BUF_DIR_IN) {
        ofi_get_inbuf_shape_info_by_index(info->model_id, (uint32_t)index,
                                          (uint32_t*)width, (uint32_t*)height,
                                          (uint32_t*)channel);
        break;
      }
      if (dir == EXYNOS_NN_BUF_DIR_OUT) {
        ofi_get_outbuf_shape_info_by_index(info->model_id, (uint32_t)index,
                                           (uint32_t*)width, (uint32_t*)height,
                                           (uint32_t*)channel);
        break;
      }
      break;
    }
    default: {
      ret = RET_PARAM_INVALID;
      break;
    }
  }
  return ret;
}

int32_t ExynosNN_releaseMemory(uint32_t exynos_id, exynos_nn_memory_t* mem) {
  std::lock_guard<std::mutex> lock(mutex_mExynosNN);
  if (mMapTargetExynosId.count(exynos_id) == 0) {
    return RET_PARAM_INVALID;
  }
  exynos_info* info = mMapTargetExynosId.at(exynos_id);
  int32_t ret = RET_OK;
  switch (info->type) {
    case EXYNOS_NN_TYPE_OFI: {
      /* TODO: code cleanup --> make function */

      auto itr = info->map_ofi_buf_set.find(mem->address);
      if (itr == info->map_ofi_buf_set.end()) {
        ret = RET_ERROR_ON_MEM_FREE;
        break;
      }
      ofi_buf_set* buf_set = &itr->second;
      if (buf_set->dir == EXYNOS_NN_BUF_DIR_IN) {
        ofi_freeInputs(info->model_id, buf_set->base);
      } else if (buf_set->dir == EXYNOS_NN_BUF_DIR_OUT) {
        ofi_freeOutputs(info->model_id, buf_set->base);
      }
      info->map_ofi_buf_set.erase(mem->address);
      break;
    }
    case EXYNOS_NN_TYPE_BOTH:
    default:
      ret = RET_PARAM_INVALID;
      break;
  }

  return 0;
}

int32_t ExynosNN_unloadModel(uint32_t exynos_id) {
  std::lock_guard<std::mutex> lock(mutex_mExynosNN);
  int32_t ret = RET_OK;
  if (mMapTargetExynosId.count(exynos_id) == 0) {
    return RET_PARAM_INVALID;
  }
  exynos_info* info = mMapTargetExynosId.at(exynos_id);
  switch (info->type) {
    case EXYNOS_NN_TYPE_OFI: {
      ret = ofi_unload_model(info->model_id);
      /* TODO: iterate buf_set_map and release remain memory. */
      break;
    }
    case EXYNOS_NN_TYPE_BOTH: {
      break;
    }
    default: {
      ret = RET_PARAM_INVALID;
      break;
    }
  }
  delete info;
  mMapTargetExynosId.erase(exynos_id);
  return ret;
}

int32_t ExynosNN_deinitialize() {
  int32_t ret = RET_OK;
  //  ret |= ofi_unset_preset("KPI_BOOST");
  ret |= ofi_deinitialize();
  mMapTargetExynosId.clear();
  return ret;
}

NnRet ExynosOFI_Initialize() {
  int32_t ret = ExynosNN_initialize();
  return static_cast<NnRet>(ret);
}

NnRet ExynosOFI_OpenModel(EdenModelFile* modelFile, uint32_t* modelId,
                          EdenPreference preference) {
  exynos_nn_pref_t unused_pref;
  const char* graph_path =
      reinterpret_cast<const char*>(modelFile->pathToModelFile);
  int32_t ret =
      ExynosNN_loadModel(graph_path, EXYNOS_NN_TYPE_OFI, unused_pref, modelId);
  return static_cast<NnRet>(ret);
}

NnRet ExynosOFI_OpenEdenModelFromMemory(ModelTypeInMemory modelTypeInMemory,
                                        int8_t* addr, int32_t size,
                                        bool encrypted, uint32_t* modelId,
                                        EdenModelOptions& options) {
  int32_t ret = ExynosNN_loadModelFromMemory(reinterpret_cast<char*>(addr),
                                             size, modelId);
  return static_cast<NnRet>(ret);
}

NnRet ExynosOFI_AllocateInputBuffers(uint32_t modelId, EdenBuffer** buffers,
                                     int32_t* numOfBuffers) {
  exynos_nn_memory_t** mem = reinterpret_cast<exynos_nn_memory_t**>(buffers);
  int32_t size = IN_WIDTH * IN_HEIGHT * IN_CHANNEL;
  int32_t ret = ExynosNN_createMemory(modelId, size, EXYNOS_NN_TARGET_DEVICE, 0,
                                      EXYNOS_NN_BUF_DIR_IN, mem, numOfBuffers);
  return static_cast<NnRet>(ret);
}

NnRet ExynosOFI_AllocateOutputBuffers(uint32_t modelId, EdenBuffer** buffers,
                                      int32_t* numOfBuffers) {
  exynos_nn_memory_t** mem = reinterpret_cast<exynos_nn_memory_t**>(buffers);
  int32_t size = OUT_WIDTH * OUT_HEIGHT * OUT_CHANNEL * 4;
  int32_t ret = ExynosNN_createMemory(modelId, size, EXYNOS_NN_TARGET_DEVICE, 0,
                                      EXYNOS_NN_BUF_DIR_OUT, mem, numOfBuffers);
  return static_cast<NnRet>(ret);
}

static void convert_bgrc_to_bgr(char* dst, const char* src, size_t size) {
  size_t channel_size = size / 3;
  int blueOffset = 0;
  int greenOffset = channel_size;
  int redOffset = channel_size * 2;
  for (size_t i = 0; i < channel_size; i++) {
    *dst++ = src[blueOffset++];
    *dst++ = src[greenOffset++];
    *dst++ = src[redOffset++];
  }
}

NnRet ExynosOFI_CopyToBuffer(EdenBuffer* buffer, int32_t offset,
                             const char* input, size_t size,
                             IMAGE_FORMAT image_format) {
  exynos_nn_memory_t* mem = reinterpret_cast<exynos_nn_memory_t*>(buffer);
  if (mem->size < (size + offset)) {
    MLOGE("OOM: buffer size: %d, input size = %zu", mem->size, size);
    return RET_ERROR_ON_RT_LOAD_BUFFERS;
  }

  switch (image_format) {
    case IMAGE_FORMAT_BGRC:
      convert_bgrc_to_bgr((char*)mem->address + offset, input, size);
      break;
    case IMAGE_FORMAT_BGR:
      memcpy((char*)mem->address + offset, input, size);
      break;
    default:
      return RET_PARAM_INVALID;
  }
  return RET_OK;
}

NnRet ExynosOFI_CopyFromBuffer(char* dst, EdenBuffer* buffer, size_t size) {
  exynos_nn_memory_t* mem = reinterpret_cast<exynos_nn_memory_t*>(buffer);
  if (mem->size < size) {
    MLOGE("OOM: buffer size: %d, dst size = %zu", mem->size, size);
    return RET_ERROR_ON_RT_LOAD_BUFFERS;
  }
  memcpy(dst, mem->address, size);
  return RET_OK;
}

NnRet ExynosOFI_ExecuteModel(EdenRequest* request, addr_t* requestId,
                             EdenPreference preference) {
  exynos_nn_request_t* exynos_request =
      reinterpret_cast<exynos_nn_request_t*>(request);
  int32_t ret = ExynosNN_executeModel(request->modelId, exynos_request);
  return static_cast<NnRet>(ret);
}

NnRet ExynosOFI_ExecuteEdenModel(EdenRequest* request, addr_t* requestId,
                                 const EdenRequestOptions& options) {
  exynos_nn_request_t* exynos_request =
      reinterpret_cast<exynos_nn_request_t*>(request);
  int32_t ret = ExynosNN_executeModel(request->modelId, exynos_request);
  return static_cast<NnRet>(ret);
}

NnRet ExynosOFI_GetInputBufferShape(uint32_t modelId, int32_t inputIndex,
                                    int32_t* width, int32_t* height,
                                    int32_t* channel, int32_t* number) {
  int32_t ret =
      ExynosNN_getMemoryShape(modelId, EXYNOS_NN_BUF_DIR_IN, inputIndex, width,
                              height, channel, number);
  return static_cast<NnRet>(ret);
}

NnRet ExynosOFI_GetOutputBufferShape(uint32_t modelId, int32_t outputIndex,
                                     int32_t* width, int32_t* height,
                                     int32_t* channel, int32_t* number) {
  int32_t ret =
      ExynosNN_getMemoryShape(modelId, EXYNOS_NN_BUF_DIR_OUT, outputIndex,
                              width, height, channel, number);
  return static_cast<NnRet>(ret);
}

NnRet ExynosOFI_FreeBuffers(uint32_t modelId, EdenBuffer* buffers) {
  exynos_nn_memory_t* mem = reinterpret_cast<exynos_nn_memory_t*>(buffers);
  int32_t ret = ExynosNN_releaseMemory(modelId, mem);
  return static_cast<NnRet>(ret);
}

NnRet ExynosOFI_CloseModel(uint32_t modelId) {
  int32_t ret = ExynosNN_unloadModel(modelId);
  return static_cast<NnRet>(ret);
}

NnRet ExynosOFI_Shutdown(void) {
  int32_t ret = ExynosNN_deinitialize();
  return static_cast<NnRet>(ret);
}
