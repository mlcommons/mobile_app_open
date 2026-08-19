/* Copyright (c) 2020-2025 Qualcomm Innovation Center, Inc. All rights reserved.

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

#include "qtiBackendHelper.h"

#include "tensorflow/core/platform/logging.h"

void qtiBackendHelper::setBackend() {
  switch (backend_type_) {
    case QTI_BACKEND_TYPE_SNPE:
      setExecutor(BackendFactory::createBackend(Backends::SNPE));
      break;
    case QTI_BACKEND_TYPE_PSNPE:
      setExecutor(BackendFactory::createBackend(Backends::PSNPE));
      break;
    case QTI_BACKEND_TYPE_STABLE_DIFFUSION:
      setExecutor(BackendFactory::createBackend(Backends::SD));
      break;
#ifdef GENIE_FLAG
    case QTI_BACKEND_TYPE_GENIE:
      setExecutor(BackendFactory::createBackend(Backends::GENIE));
      break;
#endif
    default:
      LOG(FATAL) << "Invalid Backend type";
  }
  // TODO: Add support for other backends (e.g., SNPE, etc.
}

void qtiBackendHelper::setExecutor(std::unique_ptr<Executor> execute_handler) {
  if (execute_handler == nullptr) LOG(FATAL) << "Delegate handler is NULL";
  m_executor = std::move(execute_handler);
}

void qtiBackendHelper::flush() { m_executor->flush(); }

void qtiBackendHelper::create(const char *model_path,
                              const char *native_lib_path) {
  m_executor->create(model_path, native_lib_path);
}

mlperf_status_t qtiBackendHelper::set_input(int32_t batchIndex, int32_t i,
                                            void *data) {
  return m_executor->set_input(batchIndex, i, data);
}

mlperf_status_t qtiBackendHelper::get_output(uint32_t batchIndex,
                                             int32_t outputIndex, void **data) {
  return m_executor->get_output(batchIndex, outputIndex, data);
}

mlperf_status_t qtiBackendHelper::execute(ft_callback callback, void *context) {
  return m_executor->execute(callback, context);
}

void *qtiBackendHelper::getBuffer(size_t n) { return m_executor->getBuffer(n); }

void qtiBackendHelper::deregister(void *p) { m_executor->deregister(p); }

const char *qtiBackendHelper::get_name_() const {
  return m_executor->get_name_();
}

bool qtiBackendHelper::getUseIonBuffers_() const {
  return m_executor->getUseIonBuffers_();
}

std::vector<mlperf_data_t> qtiBackendHelper::getInputFormat_() const {
  return m_executor->getInputFormat_();
}

std::vector<mlperf_data_t> qtiBackendHelper::getOutputFormat_() const {
  return m_executor->getOutputFormat_();
}

// Set functions

void qtiBackendHelper::setConfigs(
    const mlperf_backend_configuration_t *configs) {
  // Set the corresponding executor's settings
  m_executor->setConfigs(configs);
  acceleratorName_ = configs->accelerator_desc;

  // Applying config settings to backend which is at top level
  for (int i = 0; i < configs->count; ++i) {
    if (strcmp(configs->keys[i], "bg_load") == 0) {
      if (strcmp(configs->values[i], "true") == 0) {
        bgLoad_ = true;
      } else {
        bgLoad_ = false;
      }
    } else if (strcmp(configs->keys[i], "load_off_time") == 0) {
      loadOffTime_ = atoi(configs->values[i]);
    } else if (strcmp(configs->keys[i], "load_on_time") == 0) {
      loadOnTime_ = atoi(configs->values[i]);
    }
  }

  LOG(INFO) << " | bg_load: " << bgLoad_ << " | loadOffTime: " << loadOffTime_
            << " | loadOnTime: " << loadOnTime_;
}
