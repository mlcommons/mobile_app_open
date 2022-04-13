#include "sbe1200.hpp"

namespace sbe {
	using namespace sbeID1200;
	static sbe1200 sbe_obj;

	void notify_f(addr_t *addr, addr_t value)
	{
		*addr = value;
	}

	int32_t wait_f(addr_t *addr, uint32_t value, uint32_t timeout)
	{
		MLOGD("wait done with addr[%d]", *addr);
		while (timeout--) {
			usleep(1);
			if (*addr != INVALID_REQUEST_ID) {
				MLOGD("changed callback done with addr[%d]", *addr);
				*addr = INVALID_REQUEST_ID;
				break;
			}
		}
		if (timeout == 0) {
			return -1;
		}
		return RET_OK;
	}

	void sbe1200::impl_load_model(const char *model_path)
	{
		std::ifstream infile(model_path);
		infile.seekg(0, infile.end);
		m_mdl_buf_len = infile.tellg();
		infile.seekg(0, infile.beg);
		if (m_mdl_buf_len > 0) {
			m_mdl_buf = allocate_buf(m_mdl_buf_len);;
			infile.read((char*)m_mdl_buf, m_mdl_buf_len);
		}
	}

	bool sbe1200::task_deque(int mdl_idx, std::pair<void*, void*>&node)
	{
		std::unique_lock<std::mutex> lock(task_deque_mtx);
		if(task_pool.size()==0) return false;
		MLOGD("remained node[%lu]", task_pool.size());
		node = task_pool.front();
		MLOGD("mdl[%d] taken node[%p, %p]", mdl_idx, node.first, node.second);
		task_pool.pop();
		return true;
	}

	void sbe1200::impl_inference_thread(int mdl_idx)
	{
		MLOGD("inference_thread with mdl[%d]", mdl_idx);
		int inbuf_size = mdl_container->m_inbuf_size;
		int outbuf_size = mdl_container->m_outbuf_size;

		while(true) {
			std::unique_lock<std::mutex> lock(inference_start_mtx[mdl_idx]);
			inferece_start_cond[mdl_idx].wait(lock, [this]{ return (!task_pool.empty() || force_thread_done);});
			MLOGD("thread unlock with mdl[%d]", mdl_idx);
			if(force_thread_done) return;

			std::pair<void*, void*> buf;
			while(task_deque(mdl_idx, buf)) {
				MLOGD("mdl_idx[%d], m_mdl_inbuf[%x], m_mdl_outbuf[%x]", mdl_idx, m_mdl_inbuf[mdl_idx], m_mdl_outbuf[mdl_idx]);
				MLOGD("m_mdl_inbuf[%d]->addr[%x], buf[%x]", mdl_idx, m_mdl_inbuf[mdl_idx]->addr, buf.first);
				memcpy(m_mdl_inbuf[mdl_idx]->addr, buf.first, inbuf_size);
				requests[mdl_idx].modelId = mdl_id[mdl_idx];
				requests[mdl_idx].inputBuffers = m_mdl_inbuf[mdl_idx];
				requests[mdl_idx].outputBuffers = m_mdl_outbuf[mdl_idx];

				int ret = ExecuteModel(&requests[mdl_idx], &requestId[mdl_idx], pref);
				if(ret != RET_OK) {
					MLOGE("request an inference fail ret[%d]", ret);
				}
				else {
					if(callbacks[mdl_idx].waitFor(&callbacks[mdl_idx].requestId, requestId[mdl_idx], EDEN_NN_TIMEOUT) < 0) {
						MLOGE("inference callback fail - timeout");
						break;
					}
				}
				memcpy(buf.second, m_mdl_outbuf[mdl_idx]->addr, outbuf_size);
			}
			inference_done_count++;
			if(inference_done_count == MAX_INSTANCE) {
					inferece_done_cond.notify_one();
			}
		}
	}

	bool sbe1200::impl_closeModel()
	{
		int ret = RET_OK;
		for(int mdl_idx=0; mdl_idx < MAX_INSTANCE && mdl_idx < m_batch_size; mdl_idx++) {
			for(int idx=0; idx<mdl_container->m_in_cnt; idx++) {
				ret = FreeBuffers(mdl_id[mdl_idx], m_mdl_inbuf[idx]);
				MLOGD("free batch_inbuf mdl_idx[%d], ret[%d]", mdl_idx, ret);
			}

			for(int idx=0; idx<mdl_container->m_out_cnt; idx++) {
				ret = FreeBuffers(mdl_id[mdl_idx], m_mdl_outbuf[idx]);
				MLOGD("free batch_outbuf mdl_idx[%d], ret[%d]", mdl_idx, ret);
			}

			ret = CloseModel(mdl_id[mdl_idx]);
			mdl_id[mdl_idx] = INVALID_MODEL_ID;
			MLOGD("close mdl_idx[%d], ret[%d]", mdl_idx, ret);
		}

		if (force_thread_done == false) {
      force_thread_done = true;
      for(int i = 0; i < MAX_INSTANCE; i++) {
        inferece_start_cond[i].notify_one();
        task_thread_executor[i].join();
      }
    }

		MLOGD("Close done");
		return true;
	}

	bool sbe1200::impl_shutdown()
	{
		int ret = Shutdown();
		if (ret != RET_OK) {
			MLOGE("fail to Shutdown");
		}
		MLOGD("Shutdown done");
		return true;
	}

	bool sbe1200::initialize(mlperf_backend_configuration_t *configs)
	{
		int ret = Initialize();
		if (ret != RET_OK) {
			return false;
		}

		impl_parse_ext_attribute(configs);
		return true;
	}

	bool sbe1200::open_model(const char* model_path, const char* accelerator)
	{
		int ret = 0;
		pref_hw = (HwPreference)NPU_ONLY;
		options.modelPreference = {{pref_hw, BOOST_MODE, {false, false}}, EDEN_NN_API};
		options.priority = P_DEFAULT;
		options.boundCore = NPU_UNBOUND;
		/* for execution */
		pref.mode = BOOST_MODE;
		pref.hw = pref_hw;
		for (int i = 0; i < MAX_INSTANCE; i++) {
			mdl_id[i] = INVALID_MODEL_ID;
		}

		impl_load_model(model_path);
		for(int mdl_idx=0; mdl_idx < MAX_INSTANCE && mdl_idx < m_batch_size; mdl_idx++) {
			MLOGD("mdl_idx[%d], m_batch_size[%d]", mdl_idx, m_batch_size);

			ret = OpenEdenModelFromMemory(MODEL_TYPE_IN_MEMORY_TFLITE,
								reinterpret_cast<int8_t *>(m_mdl_buf), m_mdl_buf_len,
								false, &mdl_id[mdl_idx], options);
			if (ret != RET_OK) {
				MLOGE("fail to open_model. ret = %d", ret);
				return false;
			}
		}
		return true;
	}

	bool sbe1200::set_model_buf()
	{
		int32_t buf_cnt = 0;
		int ret = 0;
		int inbuf_size = m_batch_size>1?MAX_INSTANCE:mdl_container->m_in_cnt;
		int outbuf_size = m_batch_size>1?MAX_INSTANCE:mdl_container->m_out_cnt;

		MLOGD("inbuf size[%d], outbuf size[%d]", inbuf_size, outbuf_size);

		m_mdl_inbuf.resize(inbuf_size);
		m_mdl_outbuf.resize(outbuf_size);

		for (int32_t mdl_idx = 0; mdl_idx < MAX_INSTANCE && mdl_idx < m_batch_size; mdl_idx++) {
			for(int idx=0; idx<mdl_container->m_in_cnt; idx++) {
				ret = AllocateInputBuffers(mdl_id[mdl_idx], &m_mdl_inbuf[mdl_container->m_in_cnt*mdl_idx + idx], &buf_cnt);
				if (ret != RET_OK) {
					MLOGE("fail to alloc inbuf mdl_idx[%d], ret = %d", 0, ret);
					return false;
				}
				MLOGD("m_mdl_inbuf[idx]->addr[%x], size[%d]", m_mdl_inbuf[mdl_container->m_in_cnt*mdl_idx + idx]->addr, m_mdl_inbuf[mdl_container->m_in_cnt*mdl_idx + idx]->size);
			}
			for(int idx=0; idx<mdl_container->m_out_cnt; idx++) {
				ret = AllocateOutputBuffers(mdl_id[mdl_idx], &m_mdl_outbuf[mdl_container->m_out_cnt*mdl_idx + idx], &buf_cnt);
				if (ret != RET_OK) {
					MLOGE("fail to alloc outbuf mdl_idx[%d], ret = %d", 0, ret);
					return false;
				}
				MLOGD("m_mdl_outbuf[idx]->addr[%x], size[%d]", m_mdl_outbuf[mdl_container->m_out_cnt*mdl_idx + idx]->addr, m_mdl_outbuf[mdl_container->m_out_cnt*mdl_idx + idx]->size);
			}
		}

		MLOGD("m_mdl_inbuf.size[%d], m_mdl_outbuf.size[%d]", m_mdl_inbuf.size(), m_mdl_outbuf.size());
		return true;
	}

	bool sbe1200::inference()
	{
		int ret = RET_OK;
		if(m_batch_size == 1) {
			requests[0].modelId = mdl_id[0];
			requests[0].inputBuffers = m_mdl_inbuf[0];
			requests[0].outputBuffers = m_mdl_outbuf[0];

			ret = ExecuteModel(&requests[0], &requestId[0], pref);
			if (callbacks[0].waitFor(&callbacks[0].requestId, requestId[0], EDEN_NN_TIMEOUT) < 0) {
				ret = RET_ERROR_ON_RT_EXECUTE_MODEL;
				MLOGE("fail to inference with ret[%d]", ret);
			}
		}
		else {
			for (int mdl_idx = 0; mdl_idx < MAX_INSTANCE; mdl_idx++) {
					inferece_start_cond[mdl_idx].notify_one();
				}

				std::unique_lock<std::mutex> lock(inference_done_mtx);
				inferece_done_cond.wait(lock);
				inference_done_count = 0;
		}
		return true;
	}

  void sbe1200::impl_config_batch()
  {
    force_thread_done = false;
    for(int mdl_idx = 0; mdl_idx < MAX_INSTANCE && mdl_idx < m_batch_size; mdl_idx++) {
      task_thread_executor[mdl_idx] = std::thread(&sbe1200::impl_inference_thread, this, mdl_idx);
    }
		m_batch_buf = allocate_buf(mdl_container->m_outbuf_size * m_batch_size);
  }

	void sbe1200::config_request()
	{
		requests = new EdenRequest[MAX_INSTANCE];
		callbacks = new EdenCallback[MAX_INSTANCE];
		requestId = new addr_t[MAX_INSTANCE];

		for (int idx = 0; idx < MAX_INSTANCE && idx < m_batch_size; idx++) {
			callbacks[idx].notify = notify_f;
			callbacks[idx].waitFor = wait_f;
			callbacks[idx].requestId = INVALID_REQUEST_ID;
			callbacks[idx].executionResult.inference.retCode = RET_OK;
			requestId[idx] = INVALID_REQUEST_ID;
			requests[idx].callback = &callbacks[idx];
			requests[idx].hw = pref_hw;
		}

		if(m_batch_size > 1) {
			impl_config_batch();
		}
	}

	void sbe1200::set_inbuf(void* p, int batch_idx, int idx)
	{
		if(m_batch_size == 1) {
			memcpy(m_mdl_inbuf[idx]->addr, p, mdl_container->m_inbuf_size);
		}
		else {
			task_pool.push({p, (char*)(m_batch_buf) + batch_idx * mdl_container->m_outbuf_size});
		}
	}

	void *sbe1200::allocate_buf(size_t size)
	{
		if(!size)
			return nullptr;

		void *ptr = std::malloc(size);
		heap_mem[ptr]=ptr;
		return ptr;
	}

	void sbe1200::release_buf(void *p)
	{
		if(heap_mem.find(p) != heap_mem.end()) {
			heap_mem.erase(p);
		}
		std::free(p);
		p = nullptr;
	}

	void sbe1200::clear()
	{
		impl_closeModel();
		impl_shutdown();

		m_mdl_buf_len=0;
		det_lbl_boxes.clear();
		det_lbl_indices.clear();
    det_lbl_prob.clear();
    det_num.clear();
    det_lbl_boxes.shrink_to_fit();
    det_lbl_indices.shrink_to_fit();
    det_lbl_prob.shrink_to_fit();
    det_num.shrink_to_fit();

		m_batch_size = 0;
		m_created = false;

		while(!task_pool.empty()) {
			task_pool.pop();
		}

		for(auto elem : heap_mem) {
      free(elem.second);
    }
		heap_mem.clear();

		m_mdl_inbuf.clear();
		m_mdl_inbuf.shrink_to_fit();

		m_mdl_outbuf.clear();
		m_mdl_outbuf.shrink_to_fit();

		mdl_container->deinit();
		mdl_container = nullptr;

		delete[] requests;
		delete[] callbacks;
		delete[] requestId;
	}

	void sbe1200::impl_parse_ext_attribute(mlperf_backend_configuration_t *configs)
  {
		if (configs->batch_size > 1) {
			m_batch_size = configs->batch_size;
		}

    for (int i = 0; i < configs->count; ++i) {
      if(strcmp(configs->keys[i], "i_type") == 0) {
        if(strcmp(configs->values[i], "Int32") == 0) {
          mdl_attr.m_inbuf_type = mlperf_data_t::Int32;
        }
        else {
          mdl_attr.m_inbuf_type = mlperf_data_t::Uint8;
        }
      }
      else if(strcmp(configs->keys[i], "o_type") == 0) {
        if(strcmp(configs->values[i], "Float32") == 0) {
          mdl_attr.m_outbuf_type = mlperf_data_t::Float32;
        }
        else {
          mdl_attr.m_outbuf_type = mlperf_data_t::Uint8;
        }
      }
    }
  }

	void sbe1200::impl_parse_mdl_attribute()
  {
		int32_t in_w, in_h, in_c, in_n;
		GetInputBufferShape(mdl_id[0], 0, &in_w, &in_h, &in_c, &in_n);

		int32_t out_w, out_h, out_c, out_n;
		GetOutputBufferShape(mdl_id[0], 0, &out_w, &out_h, &out_c, &out_n);

    mdl_attr.m_channel = in_c;
    mdl_attr.m_width = in_w;
    mdl_attr.m_height = in_h;

    mdl_attr.m_inbuf_size = in_w*in_h*in_c*in_n;
    mdl_attr.m_outbuf_size = out_w*out_h*out_c*out_n*mdl_attr.get_byte(mdl_attr.m_outbuf_type);
  }

	void sbe1200::attach_model_container()
	{
		impl_parse_mdl_attribute();

		if(mdl_attr.m_inbuf_size == obj_od.get_buf_size()) {
      mdl_container = &obj_od;
			det_lbl_boxes = std::vector<float>(mdl_attr.m_outbuf_size/7);
      det_lbl_indices = std::vector<float>(mdl_attr.m_outbuf_size/28);
      det_lbl_prob = std::vector<float>(mdl_attr.m_outbuf_size/28);
      det_num = std::vector<float>(1);
    }
    else if(mdl_attr.m_inbuf_size == obj_ic.get_buf_size()) {
      mdl_container = &obj_ic;
    }
    else {
      mdl_container = &obj_is;
    }

		mdl_attr.m_in_cnt = mdl_container->m_in_cnt;
		mdl_attr.m_out_cnt = mdl_container->m_out_cnt;

		mdl_attr.update(mdl_container);
		mdl_attr.show();

		mdl_container->init();
	}
} // sbe

using namespace sbe;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

bool backend_create(const char *model_path, mlperf_backend_configuration_t *configs,
                                        const char *native_lib_path)
{
  if (sbe_obj.m_created) {
    sbe_obj.clear();
  }
	sbe_obj.m_mdl_buf_len = 0;
  sbe_obj.m_created = true;
    sbe_obj.m_batch_size = 1;

	if (sbe_obj.initialize(configs) != true) {
		return false;
	}
	if (sbe_obj.open_model(model_path, configs->accelerator) != true) {
		return false;
	}

	sbe_obj.attach_model_container();
	sbe_obj.set_model_buf();
	sbe_obj.config_request();
	return true;
}

int32_t backend_get_input_count()
{
  return sbe_obj.mdl_container->get_input_size();
}

mlperf_data_t backend_get_input_type(int32_t i)
{
  return sbe_obj.mdl_container->get_input_type(i);
}

mlperf_status_t backend_set_input(int32_t batchIndex, int32_t i, void* data)
{
  sbe_obj.set_inbuf(data, batchIndex, i);
  return MLPERF_SUCCESS;
}

int32_t backend_get_output_count()
{
  return sbe_obj.mdl_container->get_output_size();
}

mlperf_data_t backend_get_output_type(int32_t i)
{
  return sbe_obj.mdl_container->get_output_type(i);
}

mlperf_status_t backend_issue_query()
{
  if (sbe_obj.inference()) {
    return MLPERF_SUCCESS;
  }
  return MLPERF_FAILURE;
}

mlperf_status_t backend_get_output(uint32_t batch_idx, int32_t idx, void **data)
{
  if (sbe_obj.mdl_container->m_model_id == MOBILE_BERT) {
    if (idx == 1) {
      *data = sbe_obj.m_mdl_outbuf[0]->addr;
    }
    else if (idx == 0) {
      *data = sbe_obj.m_mdl_outbuf[1]->addr;
    }
  }
  else {
    if (sbe_obj.mdl_container->m_model_id == OBJECT_DETECTION) {
			float* buf = (float*)(sbe_obj.m_mdl_outbuf[0]->addr);
			float det_idx = 0.0;
			int det_cnt = 0;
			int block_cnt = ((detection*)(sbe_obj.mdl_container))->det_block_cnt;
			int block_size = ((detection*)(sbe_obj.mdl_container))->det_block_size;

			for (int i=0, j=0; j<block_cnt; j++) {
				det_idx = buf[j*block_size+1];
				if(det_idx > 0) {
					switch (idx) {
						case 0:
								sbe_obj.det_lbl_boxes[i++] = buf[j*block_size+4];
								sbe_obj.det_lbl_boxes[i++] = buf[j*block_size+3];
								sbe_obj.det_lbl_boxes[i++] = buf[j*block_size+6];
								sbe_obj.det_lbl_boxes[i++] = buf[j*block_size+5];
							break;
						case 1:
							sbe_obj.det_lbl_indices[j] = det_idx-1;
						case 2:
							sbe_obj.det_lbl_prob[j] = buf[j*block_size+2];
						case 3:
							det_cnt++;
						default:
							break;
					}
				}
			}

			switch (idx) {
				case 0:
					*data = (void *)(sbe_obj.det_lbl_boxes.data());
					break;
				case 1:
					*data = (void *)(sbe_obj.det_lbl_indices.data());
					break;
				case 2:
					*data = (void *)(sbe_obj.det_lbl_prob.data());
					break;
				case 3:
					sbe_obj.det_num[0] = det_cnt;
					*data = (void *)(sbe_obj.det_num.data());
					memset(buf, 0, sizeof(float) * block_size * block_cnt);
					break;
				default:
					break;
			}
    }
    else if (sbe_obj.mdl_container->m_model_id == IMAGE_SEGMENTATION) {
      *data = (void*)(sbe_obj.m_mdl_outbuf[0]->addr);
    }
    else if (sbe_obj.mdl_container->m_model_id == IMAGE_CLASSIFICATION) {
      uint8_t *buf;
      if(sbe_obj.m_batch_size > 1)
        buf = (uint8_t*)(sbe_obj.m_batch_buf);
      else
        buf = (uint8_t*)(sbe_obj.m_mdl_outbuf[0]->addr);
			*data = (void*)(buf + sbe_obj.mdl_container->m_outbuf_size * batch_idx);
    }
  }
  return MLPERF_SUCCESS;
}

void backend_convert_inputs(int bytes, int width, int height, uint8_t* data)
{
  std::vector<uint8_t>* data_uint8 = new std::vector<uint8_t>(bytes);
  int blueOffset = 0;
  int greenOffset = width * height;
  int redOffset = width * height * 2;
  int idx = 0;

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      (*data_uint8)[redOffset] = data[idx];
      (*data_uint8)[greenOffset] = data[idx + 1];
      (*data_uint8)[blueOffset] = data[idx + 2];
      redOffset++;
      greenOffset++;
      blueOffset++;
      idx = idx + 3;
    }
  }
  memcpy(data, data_uint8->data(), sizeof(uint8_t) * bytes);
  data_uint8->clear();
  data_uint8->shrink_to_fit();
  delete data_uint8;
}

void backend_delete()
{
  sbe_obj.clear();
}

void* backend_get_buffer(size_t size)
{
  return sbe_obj.allocate_buf(size);
}

void backend_release_buffer(void *p)
{
  sbe_obj.release_buf(p);
}

#ifdef __cplusplus
}
#endif  // __cplusplus