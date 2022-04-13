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
#ifndef SBE1200_H_
#define SBE1200_H_

/**
 * @file sbe1200.hpp
 * @brief samsung backend for 1200.
 * @date 2022-01-04
 * @author soobong Huh (soobong.huh@samsung.com)
 */

#include <iostream>
#include <queue>
#include <unordered_map>
#include <thread>
#include <unistd.h>
#include <fstream>
#include "client/eden_nn_api.h"
#include "client/eden_types.h"
#include "sbe_model_container.hpp"
#include "sbe_utils.hpp"
#include "type.h"
namespace sbe {
class sbe1200 {
  public:
  const std::string name_ = "sbe1200";
	int m_batch_size;

  /* common buf */
	void* m_mdl_buf;
	size_t m_mdl_buf_len;
	HwPreference pref_hw = NPU_ONLY;

  std::vector<float> det_lbl_boxes;
  std::vector<float> det_lbl_indices;
  std::vector<float> det_lbl_prob;
  std::vector<float> det_num;

  uint32_t mdl_id[MAX_INSTANCE];
  std::vector<EdenBuffer *> m_mdl_inbuf;
  std::vector<EdenBuffer *> m_mdl_outbuf;

  void * m_batch_buf;

  /* create request */
  EdenRequest *requests;
  EdenCallback *callbacks;
  addr_t *requestId;
  EdenPreference pref;
  EdenModelOptions options;

  bool m_created;

  model_container *mdl_container;

  std::queue<std::pair<void*, void*>> task_pool;
  std::unordered_map<void*, void*> heap_mem;

  std::condition_variable inferece_start_cond[MAX_INSTANCE];
  std::condition_variable inferece_done_cond;
  std::mutex inference_start_mtx[MAX_INSTANCE];
  std::mutex inference_done_mtx;
  std::mutex task_deque_mtx;

  std::atomic<bool> force_thread_done{true};
  std::atomic<int> inference_done_count{0};
  std::thread task_thread_executor[MAX_INSTANCE];

  /* common API */
  bool impl_closeModel();
  bool impl_shutdown();
  void clear();
  bool inference();
  void *allocate_buf(size_t);
  void release_buf(void *);

  void set_inbuf(void*, int, int);
  void config_request();
  void impl_config_batch();
  bool initialize(mlperf_backend_configuration_t *);

  /* common external api */
  void attach_model_container();
  void impl_parse_mdl_attribute();
  void impl_parse_ext_attribute(mlperf_backend_configuration_t *);

  /*  batch execution */
  void impl_inference_thread(int);
  bool task_deque(int mdl_idx, std::pair<void*, void*>&node);

  /* target specific */
  bool open_model(const char*, const char*);
  void impl_load_model(const char *);
  bool set_model_buf();

	sbe1200() : m_batch_size(0), m_mdl_buf(nullptr), m_created(false) {}
};	// sbe1200

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

  bool backend_create(const char *, mlperf_backend_configuration_t *, const char *);
  mlperf_status_t backend_get_output(uint32_t, int32_t, void **);
  int32_t backend_get_input_count();
  mlperf_data_t backend_get_input_type(int32_t);
  mlperf_status_t backend_set_input(int32_t, int32_t, void*);
  int32_t backend_get_output_count();
  mlperf_data_t backend_get_output_type(int32_t);
  mlperf_status_t backend_issue_query();
  void backend_convert_inputs(int, int, int, uint8_t*);
  void backend_delete();
  void *backend_get_buffer(size_t);
  void backend_release_buffer(void*);

#ifdef __cplusplus
}
#endif  // __cplusplus

} // namespace sbe;
#endif