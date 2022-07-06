#include "sbe2200.hpp"

namespace sbe {
  using namespace sbeID2200;
  static sbe2200 sbe_obj;

  std::condition_variable inferece_start_cond[MAX_INSTANCE];
  std::condition_variable inferece_done_cond;
  std::mutex inference_start_mtx[MAX_INSTANCE];
  std::mutex inference_done_mtx;
  std::mutex task_deque_mtx;

  std::atomic<bool> force_thread_done{true};
  std::atomic<int> inference_done_count{0};
  std::thread task_thread_executor[MAX_INSTANCE];
  EnnModelId mdl_id[MAX_INSTANCE] = {INVALID_MDL,};

  bool sbe2200::task_deque(int mdl_idx, std::pair<void*, void*>&node)
  {
    std::unique_lock<std::mutex> lock(task_deque_mtx);
    if(task_pool.size()==0) return false;
    MLOGD("remained node[%lu]", task_pool.size());
    node = task_pool.front();
    MLOGD("mld[%d] taken node[%p, %p]", mdl_idx, node.first, node.second);
    task_pool.pop();
    return true;
  }

  void sbe2200::impl_inference_thread(int mdl_idx)
  {
    if (EnnSetExecMsgAlwaysOff() != ENN_RET_SUCCESS) {
      MLOGD("fail to turn off execute log for thread");
    }
    int inbuf_size = mdl_container->m_inbuf_size;
    int outbuf_size = mdl_container->m_outbuf_size;

    while(true) {
      std::unique_lock<std::mutex> lock(inference_start_mtx[mdl_idx]);
      inferece_start_cond[mdl_idx].wait(lock, [this]{ return (!task_pool.empty() || force_thread_done);});
      if(force_thread_done) return;

      std::pair<void*, void*> buf;
      while(task_deque(mdl_idx, buf)) {
        memcpy(m_inbuf[mdl_idx], buf.first, inbuf_size);
        EnnReturn ret = EnnExecuteModel(mdl_id[mdl_idx]);
        if (ret != ENN_RET_SUCCESS) {
          MLOGE("fail to execute model");
        }
        memcpy(buf.second, m_batch_buf[mdl_idx], outbuf_size);
      }
      inference_done_count++;
      if(inference_done_count == MAX_INSTANCE) {
          inferece_done_cond.notify_one();
      }
    }
  }

  bool sbe2200::impl_closeModel()
  {
    EnnReturn ret = ENN_RET_SUCCESS;
    for (int i = 0; i < MAX_INSTANCE; i++) {
      if (mdl_id[i] == INVALID_MDL)
        continue;

      ret = EnnCloseModel(mdl_id[i]);
      if (ret != ENN_RET_SUCCESS) {
        MLOGE("fail to Close model #%d", i);
      }

      mdl_id[i] = INVALID_MDL;
    }

    if (force_thread_done == false) {
      force_thread_done = true;
      for(int i = 0; i < MAX_INSTANCE; i++) {
        inferece_start_cond[i].notify_one();
        task_thread_executor[i].join();  // wait thread done
      }
    }

    if(mdl_container->b_enable_fpc) {
      ret = EnnUnsetFastIpc();
    }
    return ret;
  }

  bool sbe2200::impl_shutdown()
  {
    EnnReturn ret = EnnDeinitialize();
    if (ret != ENN_RET_SUCCESS) {
      MLOGE("fail to Deinitialize");
    }
    return true;
  }
  
  void sbe2200::impl_set_buffer(int mdl_idx, void *buf, enn_buf_dir_e dir, int idx)
  {
    EnnBufferPtr p_memobj = impl_get_memobj(buf);
    EnnSetBufferByIndex(mdl_id[mdl_idx], dir, idx, p_memobj);
  }

  void sbe2200::impl_commit_buffer(int mdl_idx)
  {
    if(!mdl_container->b_enable_lazy) {
      EnnBufferCommit(mdl_id[mdl_idx]);
    }
  }

  EnnBufferPtr sbe2200::impl_get_memobj(void *p)
  {
    return mapped_mem[p];
  }

  bool sbe2200::initialize(mlperf_backend_configuration_t *configs) {
    EnnReturn ret = EnnInitialize();
    if (ret != ENN_RET_SUCCESS) {
      MLOGE("fail to EnnInitialize");
      return false;
    }

    impl_parse_ext_attribute(configs);
    return true;
  }

  bool sbe2200::open_model(const char* model_path) {
    MLOGD("model_path : %s", model_path);
    EnnReturn ret = ENN_RET_SUCCESS;
    for (int i = 0; i < MAX_INSTANCE; i++) {
      mdl_id[i] = INVALID_MDL;
    }
    ret = EnnSetPreferencePerfMode(ENN_PREF_MODE_CUSTOM);
    if (ret != ENN_RET_SUCCESS) {
      MLOGE("fail to set perf mode. But test continues");
    }
    ret = EnnSetPreferencePresetId(mdl_attr.m_preset_id);
    if (ret != ENN_RET_SUCCESS) {
      MLOGE("fail to set preset id. But test continues");
    }
    ret = EnnSetExecMsgAlwaysOff();
    if (ret != ENN_RET_SUCCESS) {
      MLOGE("fail to turn of execute log. But test continues");
    }
    for(int mdl_idx = 0; mdl_idx < MAX_INSTANCE && mdl_idx < m_batch_size; mdl_idx++) {
      ret = EnnOpenModel(model_path, &mdl_id[mdl_idx]);
      if (ret != ENN_RET_SUCCESS) {
        MLOGE("fail to Open model, model_id [%d]", mdl_idx);
        return false;
      }
    }
    return true;
  }

  bool sbe2200::inference() {
    if(mdl_container->b_enable_lazy) {
      for(int mdl_idx = 0; mdl_idx < MAX_INSTANCE && mdl_idx < m_batch_size; mdl_idx++) {
        EnnBufferCommit(mdl_id[mdl_idx]);
      }
      mdl_container->unset_lazy();
    }

    EnnReturn ret = ENN_RET_SUCCESS;
    if (m_batch_size == 1) {
      if(mdl_container->b_enable_fpc) {
        ret = EnnExecuteModelFastIpc(mdl_id[0], mdl_container->m_freeze);
      }
      else {
        ret = EnnExecuteModel(mdl_id[0]);
      }
      MLOGD("Enn execute model with ret[%d]", ret);
      if (ret != ENN_RET_SUCCESS) {
        MLOGE("fail to Execute model for single batch");
        return false;
      }
      return true;
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

  void sbe2200::impl_config_batch()
  {
    force_thread_done = false;
    for(int mdl_idx = 0; mdl_idx < MAX_INSTANCE && mdl_idx < m_batch_size; mdl_idx++) {
      task_thread_executor[mdl_idx] = std::thread(&sbe2200::impl_inference_thread, this, mdl_idx);
    }
    void *ptr = allocate_buf(mdl_container->m_outbuf_size * m_batch_size);
    m_outbuf.push_back(ptr);
  }

  void sbe2200::config_instance()
  {
    void *ptr = nullptr;
    for (int32_t mdl_idx = 0; mdl_idx < MAX_INSTANCE && mdl_idx < m_batch_size; mdl_idx++) {
      for(int idx=0; idx<mdl_container->m_in_cnt; idx++) {
        ptr = allocate_buf(mdl_container->m_inbuf_size);
        m_inbuf.push_back(ptr);
        impl_set_buffer(mdl_idx, ptr, ENN_DIR_IN, idx);
      }
      for(int idx=0; idx<mdl_container->m_out_cnt; idx++) {
        ptr = allocate_buf(mdl_container->m_outbuf_size);
        if(m_batch_size > 1) m_batch_buf.push_back(ptr);
        else m_outbuf.push_back(ptr);
        impl_set_buffer(mdl_idx, ptr, ENN_DIR_OUT, idx);
      }
      impl_commit_buffer(mdl_idx);
    }
    if(m_batch_size > 1) {
      impl_config_batch();
    }
  }

  void sbe2200::set_inbuf(void* p, int batch_idx, int idx)
  {
    if(m_batch_size == 1) {
      memcpy(m_inbuf[idx], p, mdl_container->m_inbuf_size);
    }
    else {
      task_pool.push({p, (char*)m_outbuf[0] + batch_idx * mdl_container->m_outbuf_size});
    }
  }

  void *sbe2200::allocate_buf(size_t size)
  {
    if(!size)
      return nullptr;

    EnnBufferPtr ptr;
    EnnReturn ret = EnnCreateBuffer(&ptr, size);
    if(ret != ENN_RET_SUCCESS) {
      MLOGE("fail to alloc buf");
    }
    mapped_mem[ptr->va] = ptr;
    return ptr->va;
  }

  void sbe2200::release_buf(void *p)
  {
    /* release heap buffer */
    if(mapped_mem.find(p) == mapped_mem.end()) {
        std::free(p);
        p = nullptr;
    }
    else {
      /* release ion buffer */
      EnnReleaseBuffer(mapped_mem[p]);
      mapped_mem.erase(p);
    }
  }

  void sbe2200::clear()
  {
    impl_closeModel();
    impl_shutdown();

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

    for(auto elem : mapped_mem) {
      EnnReleaseBuffer(elem.second);
    }
    mapped_mem.clear();

    m_inbuf.clear();
    m_inbuf.shrink_to_fit();
    m_outbuf.clear();
    m_outbuf.shrink_to_fit();
    m_batch_buf.clear();
    m_batch_buf.shrink_to_fit();

    mdl_container->deinit();
    mdl_container = nullptr;
  }

  void sbe2200::impl_parse_ext_attribute(mlperf_backend_configuration_t *configs)
  {
    if (configs->batch_size > 1) {
      m_batch_size = configs->batch_size;
    }
    for (int i = 0; i < configs->count; ++i) {
      if (strcmp(configs->keys[i], "preset") == 0) {
        mdl_attr.m_preset_id = std::stoul(configs->values[i]);
      }
      else if(strcmp(configs->keys[i], "i_type") == 0) {
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
      else if(strcmp(configs->keys[i], "fpc_mode") == 0) {
        if(strcmp(configs->values[i], "true") == 0) {
          mdl_attr.b_enable_fpc = true;
        }
        else {
          mdl_attr.b_enable_fpc = false;
        }
      }
      else if(strcmp(configs->keys[i], "freezing") == 0) {
        mdl_attr.m_freeze = std::stoul(configs->values[i]);
      }
      else if(strcmp(configs->keys[i], "lazy_mode") == 0) {
        if(strcmp(configs->values[i], "true") == 0) {
          mdl_attr.b_enable_lazy = true;
        }
        else {
          mdl_attr.b_enable_lazy = false;
        }
      }
    }
  }

  void sbe2200::impl_parse_mdl_attribute()
  {
    NumberOfBuffersInfo buf_info;
    EnnGetBuffersInfo(&buf_info, mdl_id[0]);

    mdl_attr.m_in_cnt = buf_info.n_in_buf;
    mdl_attr.m_out_cnt = buf_info.n_out_buf;

    EnnBufferInfo in_buf_info, out_buf_info;
    EnnGetBufferInfoByIndex(&in_buf_info, mdl_id[0], ENN_DIR_IN, 0);
    EnnGetBufferInfoByIndex(&out_buf_info, mdl_id[0], ENN_DIR_OUT, 0);

    mdl_attr.m_channel = in_buf_info.channel;
    mdl_attr.m_width = in_buf_info.width;
    mdl_attr.m_height = in_buf_info.height;

    mdl_attr.m_inbuf_size = in_buf_info.size;
    mdl_attr.m_outbuf_size = out_buf_info.size;
  }

  void sbe2200::attach_model_container()
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

    mdl_attr.update(mdl_container);
    mdl_attr.show();

    mdl_container->init();
    if(mdl_container->b_enable_fpc) {
      EnnReturn ret = EnnSetFastIpc();
      MLOGD("Enable Fast IPC mode with ret[%d]", ret);
    }
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
  sbe_obj.m_created = true;
  sbe_obj.m_batch_size = 1;

  if (sbe_obj.initialize(configs) != true) {
    return false;
  }

  if (sbe_obj.open_model(model_path) != true) {
    return false;
  }

  sbe_obj.attach_model_container();
  sbe_obj.config_instance();
  usleep(1000000);
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

mlperf_status_t backend_get_output(uint32_t batch_idx, int32_t idx, void** data)
{  
  if(sbe_obj.mdl_container->m_model_id == MOBILE_BERT) {
    if (idx == 1) {
      *data = (void*)(sbe_obj.m_outbuf[0]);
    }
    else if (idx == 0) {
      *data = (void*)(sbe_obj.m_outbuf[1]);
    }
  }
  else if (sbe_obj.mdl_container->m_model_id == OBJECT_DETECTION) {
    float* buf = (float*)(sbe_obj.m_outbuf[0]);
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
  } else if (sbe_obj.mdl_container->m_model_id == IMAGE_SEGMENTATION) {
    *data = (void *)(sbe_obj.m_outbuf[0]);
  } else if (sbe_obj.mdl_container->m_model_id == IMAGE_CLASSIFICATION) {
    uint8_t* buf = (uint8_t*)(sbe_obj.m_outbuf[0]);
    *data = (void*)(buf + sbe_obj.mdl_container->m_outbuf_size * batch_idx);
  }
  return MLPERF_SUCCESS;
}

void backend_convert_inputs(int bytes, int width, int height, uint8_t* data)
{
  std::vector<uint8_t>* data_uint8 = new std::vector<uint8_t>(bytes);
  int blueOffset = 0;
  int greenOffset = 0;
  int redOffset = 0;
  int idx = 0;

  if(sbe_obj.mdl_container->m_model_id == IMAGE_SEGMENTATION) {
    redOffset = 0;
    greenOffset = width * height;
    blueOffset = width * height * 2;
  }
  else {
    blueOffset = 0;
    greenOffset = width * height;
    redOffset = width * height * 2;
  }

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

void *backend_get_buffer(size_t size)
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