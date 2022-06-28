/* Copyright 2020 The MLPerf Authors. All Rights Reserved.

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
#include <android/log.h>
#include <stdint.h>
#include <unistd.h>
#include <map>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "extern_ofi_stub.hpp"

namespace ofi_stub {
	static void convert_bgrc_to_bgr(char* dst, void* src, size_t size) {
		char *ptr = reinterpret_cast<char*>(src);
		size_t channel_size = size / 3;
		int blueOffset = 0;
		int greenOffset = channel_size;
		int redOffset = channel_size * 2;
		for (size_t i = 0; i < channel_size; i++) {
			*dst++ = ptr[blueOffset++];
			*dst++ = ptr[greenOffset++];
			*dst++ = ptr[redOffset++];
		}
	}

	uint32_t getUniqueModelId(void) {
		static uint32_t UID = 0;
		if (UID == (INVALID_MODEL_ID - 1)) {
			UID = 1;
			return UID;
		} else {
			return ++UID;
		}
	}

	NnRet update_ofi_mdl_id(ofi_model_id ofi_mdl_id, uint32_t* mdl_id) {
		int ret = RET_OK;
		if (ofi_mdl_id != 0) {
			exynos_info* info = new exynos_info;
			info->mdl_id = ofi_mdl_id;
			info->type = EXYNOS_NN_TYPE_OFI;
			info->exynos_id = *mdl_id = getUniqueModelId();
			m_info_map.insert(std::make_pair(info->exynos_id, info));
		} else {
			*mdl_id = INVALID_MODEL_ID;
			ret = RET_ERROR_ON_RT_OPEN_MODEL;
		}

		ret |= ofi_execute_pre_model(ofi_mdl_id);
		ret |= ofi_verify_model(ofi_mdl_id);
		return static_cast<NnRet>(ret);
	}

	NnRet Initialize() {
		int ret = RET_OK;
		m_info_map.clear();
		ret |= ofi_initialize();
		return static_cast<NnRet>(ret);
	}

	NnRet OpenModel(EdenModelFile* mdl_file, uint32_t* mdl_id) {
		NnRet ret = RET_OK;
		const char* graph_path = reinterpret_cast<const char*>(mdl_file->pathToModelFile); 
		ofi_model_id ofi_mdl_id = ofi_load_model(graph_path);
		ret = update_ofi_mdl_id(ofi_mdl_id, mdl_id);
		return ret;
	}

	NnRet OpenEdenModelFromMemory(int8_t* addr, int32_t size, uint32_t* mdl_id) {
		NnRet ret = RET_OK;
		ofi_model_id ofi_mdl_id = ofi_load_model_from_memory(reinterpret_cast<char*>(addr), size);
		MLOGD("ofi_load_model_from_memory with ofi_model_id[%d]", ofi_mdl_id);
		ret = update_ofi_mdl_id(ofi_mdl_id, mdl_id);
		return ret;
	}

	//dir : EXYNOS_NN_BUF_DIR_IN, EXYNOS_NN_BUF_DIR_OUT
	NnRet AllocateBuffers(uint32_t mdl_id, EdenBuffer** bufs, exynos_nn_buf_dir_e dir) {
		//std::lock_guard<std::mutex> lock(m_lock_mtx);
		int buf_cnt=0;
		exynos_nn_memory_t** mem = reinterpret_cast<exynos_nn_memory_t**>(bufs);
		
		int ret = RET_OK;
		if (m_info_map.count(mdl_id) == 0) {
			return RET_PARAM_INVALID;
		}

		exynos_info* info = m_info_map.at(mdl_id);
		if(info->type != EXYNOS_NN_TYPE_OFI) {
			return RET_PARAM_INVALID;
		}

		uint32_t ofi_mdl_id = info->mdl_id;
		ofi_memory* ofi_buf;
		if (dir == EXYNOS_NN_BUF_DIR_IN) {
			ret = ofi_allocateInputs(ofi_mdl_id, &buf_cnt, &ofi_buf);
		} else if (dir == EXYNOS_NN_BUF_DIR_OUT) {
			ret = ofi_allocateOutputs(ofi_mdl_id, &buf_cnt, &ofi_buf);
		} else {
			return RET_PARAM_INVALID;
		}
		if (ret != RET_OK) {
			ret = (dir == EXYNOS_NN_BUF_DIR_IN)
					? (RET_ERROR_ON_RT_ALLOCATE_INPUT_BUFFERS)
					: (RET_ERROR_ON_RT_ALLOCATE_OUTPUT_BUFFERS);
			return static_cast<NnRet>(ret);
		}
		if (buf_cnt < 1) {
			if (dir == EXYNOS_NN_BUF_DIR_IN) {
				ofi_freeInputs(ofi_mdl_id, ofi_buf);
			} else if (dir == EXYNOS_NN_BUF_DIR_OUT) {
				ofi_freeOutputs(ofi_mdl_id, ofi_buf);
			}
			return RET_PARAM_INVALID;
		}

		exynos_nn_memory_t* exynos_nn_mem_set = new exynos_nn_memory_t[buf_cnt];

		// Insert OFI buffer_set to vector and copy all buffer's addr and size.
		for (int32_t buf_idx = 0; buf_idx < buf_cnt; buf_idx++) {
			exynos_nn_memory_t* tmp_mem = &(exynos_nn_mem_set[buf_idx]);
			tmp_mem->address = ofi_buf[buf_idx]->va;
			tmp_mem->size = ofi_buf[buf_idx]->size;
		}
		/* return mem_set to user. */
		*mem = exynos_nn_mem_set;

		/* create and save ofi_buf_set for execution. */
		ofi_buf_set buf_set = {	.dir = dir, .num_buf = buf_cnt, .base = ofi_buf};
		info->map_ofi_buf_set.insert(std::make_pair(ofi_buf[0]->va, buf_set));
		return static_cast<NnRet>(ret);
	}

	NnRet CopyToBuffer(EdenBuffer* buf, int32_t offset,
								void* src, size_t size,
								IMAGE_FORMAT img_fmt) {
		exynos_nn_memory_t* mem = reinterpret_cast<exynos_nn_memory_t*>(buf);

		MLOGD("mem->size[%d], addr[%p], src[%p], size[%d]", mem->size, buf->addr, src, size);
		if (mem->size < (size + offset)) {
			MLOGE("OOM: buf size: %d, input size = %zu", mem->size, size);
			return RET_ERROR_ON_RT_LOAD_BUFFERS;
		}

		switch (img_fmt) {
			case IMAGE_FORMAT_BGRC:
				convert_bgrc_to_bgr((char*)mem->address + offset, src, size);
			break;
			case IMAGE_FORMAT_BGR:
				memcpy((char*)mem->address + offset, src, size);
			break;
			default:
			return RET_PARAM_INVALID;
		}
		return RET_OK;
	}

	NnRet CopyFromBuffer(void* dst, EdenBuffer* buf, size_t size) {
		exynos_nn_memory_t* mem = reinterpret_cast<exynos_nn_memory_t*>(buf);
		if (mem->size < size) {
			MLOGE("OOM: buf size: %d, dst size = %zu", mem->size, size);
			return RET_ERROR_ON_RT_LOAD_BUFFERS;
		}
		memcpy(dst, mem->address, size);
		return RET_OK;
	}

	NnRet ExecuteModel(EdenRequest* req) {
		int ret = RET_OK;
		exynos_nn_request_t* ofi_req = reinterpret_cast<exynos_nn_request_t*>(req);
		if (m_info_map.count(req->modelId) == 0 || ofi_req == nullptr) {
			return RET_PARAM_INVALID;
		}
		exynos_info* info = m_info_map.at(req->modelId);
		if(info->type != EXYNOS_NN_TYPE_OFI) {
			return RET_PARAM_INVALID;
		}

		/* attr represent OFI request. */
		ofi_model_exe_attr attr;
		auto in_buf_set = info->map_ofi_buf_set.find(ofi_req->inputs->address);
		if (in_buf_set == info->map_ofi_buf_set.end()) {
			MLOGE("No input tensor for Model");
			return RET_NO_INPUT_TENSOR_FOR_MODEL;
		}
		MLOGD("ofi_req->inputs->address[%p]", ofi_req->inputs->address);

		/* Find out buffer_set. */
		auto out_buf_set = info->map_ofi_buf_set.find(ofi_req->outputs->address);
		if (out_buf_set == info->map_ofi_buf_set.end()) {
			MLOGE("No output tensor for Model");
			return RET_NO_OUTPUT_TENSOR_FOR_MODEL;
		}
		MLOGD("ofi_req->outputs->address[%p]", ofi_req->outputs->address);

		attr.in_buffers = in_buf_set->second.base;
		attr.numOfIn = in_buf_set->second.num_buf;

		attr.out_buffers = out_buf_set->second.base;
		attr.numOfOut = out_buf_set->second.num_buf;

		attr.n_skip = 0;
		attr.is_wait = true;
		attr.is_async = true;

		MLOGD("ofi_req->callback->num_param=[%d]", ofi_req->callback->num_param);
		MLOGD("ofi_req->callback->param=[%p]", ofi_req->callback->param);

		if (ofi_req->callback) {
			// ExynosNN user-callback is directly used.
			attr.function_desc.func_p = ofi_req->callback->notify;
			attr.param.num_param = ofi_req->callback->num_param;
			attr.param.params = ofi_req->callback->param;
			// pointer to return result of execution to ExynosNN user
			attr.result = &ofi_req->result;
		} else {
			/* OFI SYNC session execution mode */
			attr.is_async = false;
			attr.function_desc.func_p = nullptr;
			attr.result = nullptr;
			attr.param.params = nullptr;
			attr.param.num_param = 0;
		}

/*
struct ofi_model_exe_attr {
  bool is_wait = true;
  int n_skip = 0;
  uint8_t skip_id[N_SKIP_ID_MAX] = {
      0,
  };
  uint64_t request_id = 0;
  // variables are used for async.
  int *result;
  bool is_async = false;
  int numOfIn = 0;
  int numOfOut = 0;
  int numOfUsrparam = 0;
  ofi_memory *in_buffers = nullptr;
  ofi_memory *out_buffers = nullptr;
  ofi_memory *usrparam_buffers = nullptr;  // array
  ofi_shape_info *in_buffers_shape = nullptr;
  ofi_shape_info *out_buffers_shape = nullptr;
  ofi_usr_callback_desc function_desc;
  ofi_usr_callback_param param;
};
*/

		MLOGD("print attribute");
		MLOGD("request_id=%d", attr.request_id);
		MLOGD("result[%p] = %d", attr.result, *(attr.result));
		MLOGD("is_async = %d", attr.is_async);
		MLOGD("numOfIn = %d", attr.numOfIn);
		MLOGD("numOfOut = %d", attr.numOfOut);
		MLOGD("numOfUsrparam = %d", attr.numOfUsrparam);
		MLOGD("in_buffers = %p", attr.in_buffers);
		MLOGD("out_buffers = %p", attr.out_buffers);

		MLOGD("usrparam_buffers = %p", attr.usrparam_buffers);
		MLOGD("in_buffers_shape = %p", attr.in_buffers_shape);
		MLOGD("out_buffers_shape = %p", attr.out_buffers_shape);
		//ofi_usr_callback_desc function_desc;

		MLOGD("ofi_usr_callback_param.num_param = %d", attr.param.num_param);
		MLOGD("ofi_usr_callback_param.params = %p", *(attr.param.params));

		int* addr = (int*)ofi_req->outputs->address;
		*addr = 0;

		MLOGD("call ofi_execute_model_attr");
		ret = ofi_execute_model_attr(info->mdl_id, &attr);
		return static_cast<NnRet>(ret);
	}

	NnRet ExecuteEdenModel(EdenRequest* req) {
		int ret = ExecuteModel(req);
		return static_cast<NnRet>(ret);
	}

	#if 0
	// EXYNOS_NN_BUF_DIR_IN, EXYNOS_NN_BUF_DIR_OUT
	NnRet GetBufferShape(uint32_t modelId, int32_t index,
										int32_t* width, int32_t* height,
										int32_t* channel, int32_t* number,
										exynos_nn_buf_dir_e dir) {
	NnRet ret = RET_OK;
	if (m_info_map.count(modelId) == 0) {
		return RET_PARAM_INVALID;
	}
	exynos_info* info = m_info_map.at(modelId);

		if(info->type != EXYNOS_NN_TYPE_OFI)
		{
			return RET_PARAM_INVALID;
		}

		*number = 1;
		if (dir == EXYNOS_NN_BUF_DIR_IN) {
		ofi_get_inbuf_shape_info_by_index(info->mdl_id, (uint32_t)index,
											(uint32_t*)width, (uint32_t*)height,
											(uint32_t*)channel);
		}
		else if (dir == EXYNOS_NN_BUF_DIR_OUT) {
		ofi_get_outbuf_shape_info_by_index(info->mdl_id, (uint32_t)index,
											(uint32_t*)width, (uint32_t*)height,
											(uint32_t*)channel);
		}
		else {
			//
		}
		return ret;
	}
	#endif

	NnRet FreeBuffers(uint32_t mdl_id, EdenBuffer* bufs) {
		exynos_nn_memory_t* mem = reinterpret_cast<exynos_nn_memory_t*>(bufs);

		//std::lock_guard<std::mutex> lock(m_lock_mtx);
		if (m_info_map.count(mdl_id) == 0) {
			return RET_PARAM_INVALID;
		}
		int ret = RET_OK;
		exynos_info* info = m_info_map.at(mdl_id);
		if(info->type != EXYNOS_NN_TYPE_OFI) {
			return RET_PARAM_INVALID;
		}

		auto itr = info->map_ofi_buf_set.find(mem->address);
		if (itr == info->map_ofi_buf_set.end()) {
			return RET_ERROR_ON_MEM_FREE;
		}
		ofi_buf_set* buf_set = &itr->second;
		if (buf_set->dir == EXYNOS_NN_BUF_DIR_IN) {
			ofi_freeInputs(info->mdl_id, buf_set->base);
		} else if (buf_set->dir == EXYNOS_NN_BUF_DIR_OUT) {
			ofi_freeOutputs(info->mdl_id, buf_set->base);
		}
		info->map_ofi_buf_set.erase(mem->address);
		return static_cast<NnRet>(ret);
	}

	NnRet CloseModel(uint32_t mdl_id) {
		//std::lock_guard<std::mutex> lock(m_lock_mtx);

		int ret = RET_OK;
		if (m_info_map.count(mdl_id) == 0) {
			return RET_PARAM_INVALID;
		}
		exynos_info* info = m_info_map.at(mdl_id);
		if(info->type != EXYNOS_NN_TYPE_OFI) {
			return RET_PARAM_INVALID;
		}
		ret = ofi_unload_model(info->mdl_id);
		delete info;
		m_info_map.erase(mdl_id);
		return static_cast<NnRet>(ret);
	}

	NnRet Shutdown(void) {
		MLOGD("ready of dsp shutdown");
		int ret = RET_OK;
		ret |= ofi_deinitialize();
		m_info_map.clear();
		return static_cast<NnRet>(ret);
	}
}