#include "sbe2100.hpp"
#include "extern_ofi_stub.hpp"

namespace sbe {
	using namespace sbeID2100;
	static sbe2100 sbe_obj;

	void notify_f(addr_t *addr, addr_t value) {
		MLOGD("nodification of notify_f");
		*addr = value;
	}

	int32_t wait_f(addr_t *addr, uint32_t value, uint32_t timeout) {
		MLOGD("npu wait with addr[%x]", *addr);
		while (timeout--) {
			usleep(1);	//1
			if (*addr != INVALID_REQUEST_ID) {
				MLOGD("Done. npu callback done with addr[%d]", *addr);
				*addr = INVALID_REQUEST_ID;
				break;
			}
			MLOGD("npu wait updated result");
		}
		if (timeout == 0) {
			return -1;
		}
		return RET_OK;
	}

	int dsp_notify_f(int num_param, void** params) {
		MLOGD("nodification of dsp_notify_f");
		if (num_param != 1) {
			MLOGE("Wrong params. num_param(%d != 1)\n", num_param);
		}
		return 0;
	}

	int dsp_wait_f(exynos_nn_request_t* req) {
		MLOGD("dsp wait with exynos_id[%d]", req->exynos_id);
		while (req->result == (int32_t)0xffffffff) {
			usleep(1);	// 100
		}
		MLOGD("Done. dsp callback done with exynos_id[%d], result[%d]", req->exynos_id, req->result);
		if (req->result != RET_OK) {
			MLOGE("Execute done but failed.");
		}
		req->result = (int32_t)0xffffffff;
		return 0;
	}

	void sbe2100::impl_load_model(const char *model_path) {
		m_mdl_buf_len = m_mdl_buf_mix_len = 0;
		MLOGD("mdl_attr.m_accer_idx[%d]", mdl_attr.m_accer_idx);

		std::ifstream infile(model_path);
		if(mdl_attr.m_accer_idx == sbe::ACCER_IDX::NPUDSP) {
			int64_t file_sizes[NUM_NPUDSP_HEADER];
			MLOGD("file_sizes[%p, %p], sizeof(file_sizes)[%lu]", file_sizes, &file_sizes, sizeof(file_sizes));

			infile.read(reinterpret_cast<char*>(file_sizes), sizeof(int64_t)*NUM_NPUDSP_HEADER);

			for(int i=0;i<NUM_NPUDSP_HEADER;i++) {
				MLOGD("file size[%d]=[%lu]", i, file_sizes[i]);
			}
			m_mdl_buf_len = file_sizes[0];
			for (int i = 1; i < sizeof(file_sizes) / sizeof(file_sizes[0]); ++i) {
				m_mdl_buf_mix_len += file_sizes[i];
			}
		}
		else {
			infile.seekg(0, infile.end);
			m_mdl_buf_len = infile.tellg();
			infile.seekg(0, infile.beg);
		}
		MLOGD("m_mdl_buf_len[%zu], m_mdl_buf_mix_len[%zu]", m_mdl_buf_len, m_mdl_buf_mix_len);
		int mdl_len = m_mdl_buf_len + m_mdl_buf_mix_len;
		if (mdl_len > 0) {
			m_mdl_buf = allocate_buf(mdl_len);
			infile.read((char*)m_mdl_buf, mdl_len);
		}
	}

	bool sbe2100::task_deque(int mdl_idx, std::pair<void*, void*>&node) {
		std::unique_lock<std::mutex> lock(task_deque_mtx);
		if(task_pool.size()==0) {
			MLOGD("remained node size[%lu], mdl_idx[%d], return false", task_pool.size(), mdl_idx);
			return false;
		}
		MLOGD("remained node size[%lu]", task_pool.size());
		node = task_pool.front();
		MLOGD("mdl[%d] taken node[%p, %p]", mdl_idx, node.first, node.second);
		task_pool.pop();
		return true;
	}

	void sbe2100::impl_inference_thread(int mdl_idx) {
		MLOGD("init deploy thread with mdl[%d]", mdl_idx);
		int inbuf_size = mdl_container->m_inbuf_size;
		int outbuf_size = mdl_container->m_outbuf_size;

		MLOGD("impl_inference_thread::inbuf_size[%d], outbuf_size[%d]", inbuf_size, outbuf_size);
		while(true) {
			std::unique_lock<std::mutex> lock(inference_start_mtx[mdl_idx]);
			inferece_start_cond[mdl_idx].wait(lock, [this]{ return (!task_pool.empty() || force_thread_done);});
			MLOGD("thread unlock with mdl[%d]", mdl_idx);
			if(force_thread_done) return;

			std::pair<void*, void*> buf;
			while(task_deque(mdl_idx, buf)) {
				MLOGD("mdl_idx[%d], m_mdl_inbuf[%d]->addr[%x], buf[%x]", mdl_idx, mdl_idx, m_mdl_inbuf[mdl_idx]->addr, buf.first);
				if(mdl_idx < NPU_INSTANCE) {	// for NPU
					memcpy(requests[mdl_idx].inputBuffers->addr, buf.first, inbuf_size);
					int ret = ExecuteEdenModel(&requests[mdl_idx], &requestId[mdl_idx], req_opts);
					if(ret != RET_OK) {
						MLOGE("request an NPU inference fail, ret[%d]", ret);
					}
					else {
						if(npu_callbacks[mdl_idx].waitFor(&npu_callbacks[mdl_idx].requestId, requestId[mdl_idx], EDEN_NN_TIMEOUT) < 0) {
							MLOGE("npu inference callback fail - timeout");
							break;
						}
						else {
							MLOGD("npu inference done of mdl_idx[%d]", mdl_idx);
						}
					}
					memcpy(buf.second, m_mdl_outbuf[mdl_idx]->addr, outbuf_size);
				}
				else {	// for DSP
					MLOGD("dsp req_mdl_idx[%d]=[%d]", mdl_idx, mdl_ids[mdl_idx]);
					ofi_stub::CopyToBuffer(requests[mdl_idx].inputBuffers, 0/*offset*/, buf.first, inbuf_size, ofi_stub::IMAGE_FORMAT_BGRC);
					int ret = ofi_stub::ExecuteEdenModel(&requests[mdl_idx]);
					MLOGD("ret of an DSP inference[%d]", ret);
					if(ret != RET_OK) {
						MLOGE("request an DSP inference fail, mdl_idx[%d], ret[%d]", mdl_idx, ret);
					}
					else {
						if (dsp_wait_f(reinterpret_cast<exynos_nn_request_t*>(&requests[mdl_idx])) != RET_OK) {
							MLOGE("dsp inference callback fail - timeout");
							break;
						}
						else {
							MLOGD("dsp inference done of mdl_idx[%d]", mdl_idx);
						}
					}
					ofi_stub::CopyFromBuffer(buf.second, m_mdl_outbuf[mdl_idx], outbuf_size);					
				}
			}
			inference_done_count++;
			MLOGD("mdl_idx[%d], inference_done_count[%d]", mdl_idx, inference_done_count.load());
			if(inference_done_count == m_max_instance) {
				MLOGD("triggered inferece_done_cond");
				inferece_done_cond.notify_one();
			}
		}
	}

	bool sbe2100::impl_closeModel() {
		int ret = RET_OK;
		for(int mdl_idx=0; mdl_idx < NPU_INSTANCE && mdl_idx < m_batch_size; mdl_idx++) {
			ret = FreeBuffers(mdl_ids[mdl_idx], m_mdl_inbuf[mdl_idx]);
			MLOGD("free inbuf mdl_idx[%d], ret[%d]", mdl_idx, ret);

			ret = FreeBuffers(mdl_ids[mdl_idx], m_mdl_outbuf[mdl_idx]);
			MLOGD("free outbuf mdl_idx[%d], ret[%d]", mdl_idx, ret);

			ret = CloseModel(mdl_ids[mdl_idx]);
			mdl_ids[mdl_idx] = INVALID_MODEL_ID;
			MLOGD("npu close mdl_idx[%d], ret[%d]", mdl_idx, ret);
		}

		if(mdl_attr.m_accer_idx == sbe::ACCER_IDX::NPUDSP) {
			for (int mdl_idx = 0; mdl_idx < DSP_INSTANCE; mdl_idx++) {
				int dsp_mdl_idx = NPU_INSTANCE+mdl_idx;
				ret = ofi_stub::FreeBuffers(mdl_ids[dsp_mdl_idx], m_mdl_inbuf[dsp_mdl_idx]);
				MLOGD("free inbuf dsp_mdl_idx[%d], ret[%d]", dsp_mdl_idx, ret);

				ret = ofi_stub::FreeBuffers(mdl_ids[dsp_mdl_idx], m_mdl_outbuf[dsp_mdl_idx]);
				MLOGD("free outbuf dsp_mdl_idx[%d], ret[%d]", dsp_mdl_idx, ret);

				ret = ofi_stub::CloseModel(mdl_ids[dsp_mdl_idx]);
				mdl_ids[dsp_mdl_idx] = INVALID_MODEL_ID;
				MLOGD("dsp close model, dsp_mdl_idx[%d], ret[%d]", dsp_mdl_idx, ret);
			}
		}

		if (force_thread_done == false) {
			force_thread_done = true;
			for(int i = 0; i < m_max_instance; i++) {
				inferece_start_cond[i].notify_one();
				task_thread_executor[i].join();
			}
    	}

		MLOGD("Close done");
		return true;
	}

	bool sbe2100::impl_shutdown() {
		int ret = Shutdown();
		if (ret != RET_OK) {
			MLOGE("fail to Shutdown");
		}
		MLOGD("done of npu shutdown");
		if(mdl_attr.m_accer_idx == sbe::ACCER_IDX::NPUDSP) {
			ret = ofi_stub::Shutdown();
			MLOGD("shut down ret[%d]", ret);
		}

		MLOGD("Shutdown done");
		return true;
	}

	bool sbe2100::initialize(mlperf_backend_configuration_t *configs) {
		int ret = Initialize();
		if (ret != RET_OK) {
			return false;
		}

		impl_parse_ext_attribute(configs);

		if(mdl_attr.m_accer_idx == sbe::ACCER_IDX::NPUDSP) {
			ofi_stub::Initialize();
		}

		req_mode = RequestMode::BLOCK;
		mode_pref = BOOST_MODE;

		// TODO : Do it need ?
		if (m_batch_size > 1) {
			req_mode = RequestMode::NONBLOCK;
		}

		m_max_instance = NPU_INSTANCE;
		if(mdl_attr.m_accer_idx == sbe::ACCER_IDX::NPUDSP) {
			m_max_instance += DSP_INSTANCE;
		}

		/* configuration of option */
		if(mdl_attr.m_accer_idx == sbe::ACCER_IDX::GPU) {
			hw_pref = GPU_ONLY;
		}
		else {
			hw_pref = NPU_ONLY;
		}

		/* for open model */
		open_opts.modelPreference = {{hw_pref, mode_pref, {false, false}}, EDEN_NN_API};
		open_opts.priority = P_DEFAULT;
		open_opts.boundCore = NPU_UNBOUND;

		/* for execution */
		req_opts.userPreference = {hw_pref, mode_pref};
		req_opts.requestMode = req_mode;
		req_opts.reserved[0] = {0, };
		return true;
	}

	bool sbe2100::open_model(const char* model_path) {
		MLOGD("model_path : %s", model_path);
		int ret = 0;
		for (int idx = 0; idx < m_max_instance; idx++) {
			mdl_ids[idx] = INVALID_MODEL_ID;
		}

		impl_load_model(model_path);
		MLOGD("model load done : %s", model_path);

		for(int mdl_idx=0; mdl_idx < NPU_INSTANCE && mdl_idx < m_batch_size; mdl_idx++) {
			MLOGD("npu mdl_idx[%d], m_batch_size[%d]", mdl_idx, m_batch_size);
			ret = OpenEdenModelFromMemory(MODEL_TYPE_IN_MEMORY_TFLITE,
								reinterpret_cast<int8_t *>(m_mdl_buf), m_mdl_buf_len,
								false, &mdl_ids[mdl_idx], open_opts);
			if (ret != RET_OK) {
				MLOGE("fail to open_model. ret = %d", ret);
				return false;
			}
		}

		if(mdl_attr.m_accer_idx == sbe::ACCER_IDX::NPUDSP) {
			for(int mdl_idx=0; mdl_idx < DSP_INSTANCE && m_max_instance > NPU_INSTANCE; mdl_idx++) {
				int dsp_mdl_idx = NPU_INSTANCE+mdl_idx;
				MLOGD("dsp dsp_mdl_idx[%d], m_batch_size[%d]", dsp_mdl_idx, m_batch_size);
				ret = ofi_stub::OpenEdenModelFromMemory(reinterpret_cast<int8_t*>(m_mdl_buf) + m_mdl_buf_len, m_mdl_buf_mix_len,
														&mdl_ids[dsp_mdl_idx]);
				if (ret != RET_OK) {
					MLOGE("Open DSP model from memory failed.");
					return false;
				}
			}
		}
		/* for debug */
		for(int idx=0;idx<m_max_instance;idx++) {
			MLOGD("debug_mdl_ids[%d]=[%d]", idx, mdl_ids[idx]);
		}
		return true;
	}
	
	int sbe2100::get_io_buf_size() {
		int ret_cnt = 1;
		if(m_batch_size > 1) {
			ret_cnt = m_max_instance;
		}
		return ret_cnt;
	}

	bool sbe2100::set_model_buf() {
		int32_t buf_cnt = 0;
		int ret = 0;
		int inbuf_cnt = get_io_buf_size();
		int outbuf_cnt = get_io_buf_size();

		m_mdl_inbuf.resize(inbuf_cnt);
		m_mdl_outbuf.resize(outbuf_cnt);

		for (int32_t mdl_idx = 0; mdl_idx < NPU_INSTANCE && mdl_idx < m_batch_size; mdl_idx++) {
			ret = AllocateInputBuffers(mdl_ids[mdl_idx], &m_mdl_inbuf[mdl_idx], &buf_cnt);
			if (ret != RET_OK) {
				MLOGE("fail to alloc inbuf mdl_idx[%d], ret = %d", 0, ret);
				return false;
			}
			MLOGD("m_mdl_inbuf[%d]->addr[%x], size[%d], buf_cnt[%d]", mdl_idx, m_mdl_inbuf[mdl_idx]->addr, m_mdl_inbuf[mdl_idx]->size, buf_cnt);

			ret = AllocateOutputBuffers(mdl_ids[mdl_idx], &m_mdl_outbuf[mdl_idx], &buf_cnt);
			if (ret != RET_OK) {
				MLOGE("fail to alloc outbuf mdl_idx[%d], ret = %d", 0, ret);
				return false;
			}
			MLOGD("m_mdl_outbuf[%d]->addr[%x], size[%d], buf_cnt[%d]", mdl_idx, m_mdl_outbuf[mdl_idx]->addr, m_mdl_outbuf[mdl_idx]->size, buf_cnt);
		}

		if(mdl_attr.m_accer_idx == sbe::ACCER_IDX::NPUDSP) {
			for (int mdl_idx = 0; mdl_idx < DSP_INSTANCE; mdl_idx++) {
				int dsp_mdl_idx = NPU_INSTANCE+mdl_idx;
				ret = ofi_stub::AllocateBuffers(mdl_ids[dsp_mdl_idx], &m_mdl_inbuf[dsp_mdl_idx], EXYNOS_NN_BUF_DIR_IN);
				if (ret != RET_OK) {
					MLOGE("Fail to Allocate input buffer, ret = %d", ret);
					return false;
				}

				ret = ofi_stub::AllocateBuffers(mdl_ids[dsp_mdl_idx], &m_mdl_outbuf[dsp_mdl_idx], EXYNOS_NN_BUF_DIR_OUT);
				if (ret != RET_OK) {
					MLOGE("Fail to Allocate output buffer, ret = %d", ret);
					return false;
				}
			}
		}
		return true;
	}

	bool sbe2100::inference() {
		int ret = RET_OK;
		if(m_batch_size == 1) {
			ret = ExecuteEdenModel(&requests[0], &requestId[0], req_opts);
			if (npu_callbacks[0].waitFor(&npu_callbacks[0].requestId, requestId[0], EDEN_NN_TIMEOUT) < 0) {
				ret = RET_ERROR_ON_RT_EXECUTE_MODEL;
				MLOGE("fail to inference with ret[%d]", ret);
			}
		}
		else {
			for (int mdl_idx = 0; mdl_idx < m_max_instance; mdl_idx++) {
				inferece_start_cond[mdl_idx].notify_one();
			}

			std::unique_lock<std::mutex> lock(inference_done_mtx);
			inferece_done_cond.wait(lock);
			MLOGD("waken inferece_done_cond");
			inference_done_count = 0;
		}
		return true;
	}

	void sbe2100::impl_config_batch() {
		force_thread_done = false;
		MLOGD("m_max_instance [%d]", m_max_instance);
		for(int thread_idx = 0; thread_idx < m_max_instance && thread_idx < m_batch_size; thread_idx++) {
			task_thread_executor[thread_idx] = std::thread(&sbe2100::impl_inference_thread, this, thread_idx);
		}
		m_batch_buf = allocate_buf(mdl_container->m_outbuf_size * m_batch_size);
	}

	void sbe2100::config_request() {
		requests = new EdenRequest[m_max_instance];	// npu, npudsp
		requestId = new addr_t[m_max_instance];

		npu_callbacks = new EdenCallback[NPU_INSTANCE];
		for (int npu_idx = 0; npu_idx < NPU_INSTANCE && npu_idx < m_batch_size; npu_idx++) {
			npu_callbacks[npu_idx].notify = notify_f;
			npu_callbacks[npu_idx].waitFor = wait_f;
			npu_callbacks[npu_idx].requestId = INVALID_REQUEST_ID;
			npu_callbacks[npu_idx].executionResult.inference.retCode = RET_OK;
			requestId[npu_idx] = INVALID_REQUEST_ID;
			requests[npu_idx].callback = &npu_callbacks[npu_idx];
			requests[npu_idx].hw = hw_pref;
			requests[npu_idx].modelId = mdl_ids[npu_idx];
			requests[npu_idx].inputBuffers = m_mdl_inbuf[npu_idx];
			requests[npu_idx].outputBuffers = m_mdl_outbuf[npu_idx];
			MLOGD("requests[%p], id[%d]", &requests[npu_idx], npu_idx);
		}

		if(mdl_attr.m_accer_idx == sbe::ACCER_IDX::NPUDSP) {
			dsp_callbacks.resize(DSP_INSTANCE);
			cb_param_ptr.resize(DSP_INSTANCE);
			for (int dsp_idx = 0; dsp_idx < DSP_INSTANCE && m_max_instance > NPU_INSTANCE; dsp_idx++) {
				int dsp_mdl_idx = NPU_INSTANCE+dsp_idx;
				cb_param_ptr[dsp_idx] = reinterpret_cast<void*>(&requests[dsp_mdl_idx]);
				dsp_callbacks[dsp_idx].num_param = 1;
				dsp_callbacks[dsp_idx].param = &cb_param_ptr[dsp_idx];
				dsp_callbacks[dsp_idx].notify = dsp_notify_f;
				requests[dsp_mdl_idx].callback = reinterpret_cast<EdenCallback*>(&dsp_callbacks[dsp_idx]);
				requests[dsp_mdl_idx].modelId = mdl_ids[dsp_mdl_idx];
				requests[dsp_mdl_idx].inputBuffers = m_mdl_inbuf[dsp_mdl_idx];
				requests[dsp_mdl_idx].outputBuffers = m_mdl_outbuf[dsp_mdl_idx];
				reinterpret_cast<exynos_nn_request_t*>(&requests[dsp_mdl_idx])->result = 0xffffffff;
				MLOGD("requests[%p], id[%d]", &requests[dsp_mdl_idx], dsp_mdl_idx);
			}
		}

		if(m_batch_size > 1) {
			impl_config_batch();
		}
	}

	void sbe2100::set_inbuf(void* p, int batch_idx, int idx) {
		MLOGD("set_inbuf with ptr[%p], batch_idx[%d], idx[%d]", p, batch_idx, idx);
		if(m_batch_size == 1) {
			if(mdl_container->m_accer_idx == sbe::ACCER_IDX::GPU) {
				MLOGD("m_inbuf mem size [%d]", mdl_container->m_inbuf_size);
				memcpy((m_mdl_inbuf[0] + idx)->addr, p, mdl_container->m_inbuf_size);
			}
			else {
				MLOGD("m_inbuf mem size [%d], size[%d]", mdl_container->m_inbuf_size, m_mdl_inbuf[idx]->size);
				memcpy(m_mdl_inbuf[idx]->addr, p, mdl_container->m_inbuf_size);
			}
		}
		else {
			task_pool.push({p, (char*)(m_batch_buf) + batch_idx * mdl_container->m_outbuf_size});
		}
	}

	void *sbe2100::allocate_buf(size_t size) {
		if(!size)
			return nullptr;

		void *ptr = std::malloc(size);
		heap_mem[ptr]=ptr;
		return ptr;
	}

	void sbe2100::release_buf(void *p) {
		if(heap_mem.find(p) != heap_mem.end()) {
			heap_mem.erase(p);
		}
		std::free(p);
		p = nullptr;
	}

	void sbe2100::clear() {
		impl_closeModel();
		impl_shutdown();

		m_mdl_buf_len=0;
		m_mdl_buf_mix_len=0;
		det_lbl_boxes.clear();
		det_lbl_indices.clear();
		det_lbl_prob.clear();
		det_num.clear();
		det_lbl_boxes.shrink_to_fit();
		det_lbl_indices.shrink_to_fit();
		det_lbl_prob.shrink_to_fit();
		det_num.shrink_to_fit();

		m_batch_size = 0;
		m_max_instance = 0;
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

		dsp_callbacks.clear();
		dsp_callbacks.shrink_to_fit();

  		cb_param_ptr.clear();
		cb_param_ptr.shrink_to_fit();

		delete[] requests;
		delete[] requestId;
		delete[] npu_callbacks;
	}

	void sbe2100::impl_parse_ext_attribute(mlperf_backend_configuration_t *configs) {
		if (configs->batch_size > 1) {
			m_batch_size = configs->batch_size;
		}

		if(strcmp(configs->accelerator, "npu") == 0) {
			mdl_attr.m_accer_idx = sbe::ACCER_IDX::NPU;
		}
		else if(strcmp(configs->accelerator, "gpu") == 0) {
			mdl_attr.m_accer_idx = sbe::ACCER_IDX::GPU;
		}
		else if(strcmp(configs->accelerator, "dsp") == 0) {
			mdl_attr.m_accer_idx = sbe::ACCER_IDX::DSP;
		}
		else if(strcmp(configs->accelerator, "npudsp") == 0) {
			mdl_attr.m_accer_idx = sbe::ACCER_IDX::NPUDSP;
		}
		else {
			mdl_attr.m_accer_idx = sbe::ACCER_IDX::NPU;
		}

		for (int i = 0; i < configs->count; ++i) {
			if(strcmp(configs->keys[i], "i_type") == 0) {
				if(strcmp(configs->values[i], "Int32") == 0) {
					mdl_attr.m_inbuf_type = mlperf_data_t::Int32;
				}
				else if(strcmp(configs->values[i], "Uint8") == 0) {
					mdl_attr.m_inbuf_type = mlperf_data_t::Uint8;
				}
				else {
					mdl_attr.m_inbuf_type = mlperf_data_t::Int8;
				}
			}
			else if(strcmp(configs->keys[i], "o_type") == 0) {
				if(strcmp(configs->values[i], "Float32") == 0) {
					mdl_attr.m_outbuf_type = mlperf_data_t::Float32;
				}
				else if(strcmp(configs->values[i], "Int32") == 0) {
					mdl_attr.m_outbuf_type = mlperf_data_t::Int32;
				}
				else if(strcmp(configs->values[i], "Uint8") == 0) {
					mdl_attr.m_outbuf_type = mlperf_data_t::Uint8;
				}
				else {
					mdl_attr.m_outbuf_type = mlperf_data_t::Int8;
				}
			}
		}
	}

	void sbe2100::impl_parse_mdl_attribute() {
		int32_t in_w, in_h, in_c, in_n;
		GetInputBufferShape(mdl_ids[0], 0, &in_w, &in_h, &in_c, &in_n);

		int32_t out_w, out_h, out_c, out_n;
		GetOutputBufferShape(mdl_ids[0], 0, &out_w, &out_h, &out_c, &out_n);

		mdl_attr.m_channel = in_c;
		mdl_attr.m_width = in_w;
		mdl_attr.m_height = in_h;
		
		mdl_attr.m_inbuf_size = in_w*in_h*in_c*in_n*mdl_attr.get_byte(mdl_attr.m_inbuf_type);
		mdl_attr.m_outbuf_size = out_w*out_h*out_c*out_n*mdl_attr.get_byte(mdl_attr.m_outbuf_type);
	}

	void sbe2100::attach_model_container() {
		impl_parse_mdl_attribute();

		MLOGV("mdl_attr.m_inbuf_size[%d], mdl_attr.m_outbuf_size[%d]", mdl_attr.m_inbuf_size, mdl_attr.m_outbuf_size);

		if(mdl_attr.m_inbuf_size == obj_od.get_buf_size()) {
			mdl_container = &obj_od;
			det_lbl_boxes = std::vector<float>(mdl_attr.m_outbuf_size/7);
			det_lbl_indices = std::vector<float>(mdl_attr.m_outbuf_size/28);
			det_lbl_prob = std::vector<float>(mdl_attr.m_outbuf_size/28);
			det_num = std::vector<float>(1);
		}
		else if(mdl_attr.m_inbuf_size == obj_ic.get_buf_size()) {
			if(m_batch_size > 1) {
				mdl_container = &obj_ic_offline;
			}
			else {
				mdl_container = &obj_ic;
			}
		}
		else if(mdl_attr.m_inbuf_size == obj_is.get_buf_size()) {
			mdl_container = &obj_is;
		}
		else {
			mdl_container = &obj_bert;
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
	sbe_obj.m_mdl_buf_mix_len = 0;
	sbe_obj.m_created = true;
    sbe_obj.m_batch_size = 1;

	if (sbe_obj.initialize(configs) != true) {
		MLOGD("init failed");
		return false;
	}
	MLOGD("init done, ready to open [%s]", model_path);
	if (sbe_obj.open_model(model_path) != true) {
		MLOGD("open model failed");
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
		MLOGD("get_output of MobileBert");
		if (idx == 0) {
			*data = (sbe_obj.m_mdl_outbuf[0]+1)->addr;
		}
		else if (idx == 1) {
			*data = (sbe_obj.m_mdl_outbuf[0])->addr;
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