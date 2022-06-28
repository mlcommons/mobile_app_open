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

#ifndef SBE2200_H_
#define SBE2200_H_

/**
 * @file sbe2200.hpp
 * @brief samsung backend for exynos 2200.
 * @date 2022-01-04
 * @author soobong Huh (soobong.huh@samsung.com)
 */

#include <memory>
#include <string>
#include <vector>
#include <unistd.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <sstream>
#include <utility>
#include <cstdio>
#include <cstdlib>
#include <android/log.h>
#include <thread>
#include <unordered_map>
#include <stdint.h>
#include "client/enn_api-public.hpp"
#include "client/enn_api-type.h"
#include "sbe_model_container.hpp"
#include "type.h"
#include "sbe_utils.hpp"

namespace sbe {
using namespace enn::api;
class sbe2200 {
  public:
  const std::string name = "sbe2200";
  int m_batch_size;

  /* declaration of buffers for model */
  std::vector<void*> m_inbuf;
  std::vector<void*> m_outbuf;
  std::vector<void*> m_batch_buf;

  std::vector<float> det_lbl_boxes;
  std::vector<float> det_lbl_indices;
  std::vector<float> det_lbl_prob;
  std::vector<float> det_num;

  bool m_created;
  model_container *mdl_container;

  std::queue<std::pair<void*, void*>> task_pool;
  std::unordered_map<void*, EnnBufferPtr> mapped_mem;

  void impl_set_buffer(int, void *, enn_buf_dir_e, int);
  void impl_commit_buffer(int);

  EnnBufferPtr impl_get_memobj(void *);  
  bool impl_closeModel();
  bool impl_shutdown();

  /* external method */
  void attach_model_container();
  void impl_parse_mdl_attribute();
  void impl_parse_ext_attribute(mlperf_backend_configuration_t *);

  bool initialize(mlperf_backend_configuration_t *);
  bool open_model(const char*);
  bool inference();

  void clear();
  void *allocate_buf(size_t);
  void release_buf(void *);

  void config_instance();
  void impl_config_batch();
  void set_inbuf(void *, int , int);

  bool task_deque(int idx, std::pair<void*, void*>&);
  void impl_inference_thread(int);

  sbe2200() : m_batch_size(0), m_created(false) {}

};  // sbe2200

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
