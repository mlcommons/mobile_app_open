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

#include "GenieExecutor.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "tensorflow/core/platform/logging.h"

// Static output buffer
std::vector<uint32_t> GenieExecutor::outputTokens_;

// First-token callback statics
ft_callback GenieExecutor::ftCallback_ = nullptr;
void* GenieExecutor::ftContext_ = nullptr;
bool GenieExecutor::ftFired_ = false;

// ---------------------------------------------------------------------------
// Genie status name helper
// ---------------------------------------------------------------------------

#ifdef GENIE_FLAG
static const char* genieStatusName(Genie_Status_t s) {
  switch (s) {
    case GENIE_STATUS_SUCCESS:
      return "SUCCESS";
    case GENIE_STATUS_WARNING_ABORTED:
      return "WARNING_ABORTED";
    case GENIE_STATUS_WARNING_BOUND_HANDLE:
      return "WARNING_BOUND_HANDLE";
    case GENIE_STATUS_WARNING_PAUSED:
      return "WARNING_PAUSED";
    case GENIE_STATUS_WARNING_CONTEXT_EXCEEDED:
      return "WARNING_CONTEXT_EXCEEDED";
    case GENIE_STATUS_ERROR_GENERAL:
      return "ERROR_GENERAL";
    case GENIE_STATUS_ERROR_INVALID_ARGUMENT:
      return "ERROR_INVALID_ARGUMENT";
    case GENIE_STATUS_ERROR_MEM_ALLOC:
      return "ERROR_MEM_ALLOC";
    case GENIE_STATUS_ERROR_INVALID_CONFIG:
      return "ERROR_INVALID_CONFIG";
    case GENIE_STATUS_ERROR_INVALID_HANDLE:
      return "ERROR_INVALID_HANDLE";
    case GENIE_STATUS_ERROR_QUERY_FAILED:
      return "ERROR_QUERY_FAILED";
    case GENIE_STATUS_ERROR_JSON_FORMAT:
      return "ERROR_JSON_FORMAT";
    case GENIE_STATUS_ERROR_JSON_SCHEMA:
      return "ERROR_JSON_SCHEMA";
    case GENIE_STATUS_ERROR_JSON_VALUE:
      return "ERROR_JSON_VALUE";
    case GENIE_STATUS_ERROR_GENERATE_FAILED:
      return "ERROR_GENERATE_FAILED";
    case GENIE_STATUS_ERROR_GET_HANDLE_FAILED:
      return "ERROR_GET_HANDLE_FAILED";
    case GENIE_STATUS_ERROR_APPLY_CONFIG_FAILED:
      return "ERROR_APPLY_CONFIG_FAILED";
    case GENIE_STATUS_ERROR_SET_PARAMS_FAILED:
      return "ERROR_SET_PARAMS_FAILED";
    case GENIE_STATUS_ERROR_BOUND_HANDLE:
      return "ERROR_BOUND_HANDLE";
    default:
      return "UNKNOWN";
  }
}
#endif

// ---------------------------------------------------------------------------

// Load Genie dialog JSON config (NPU only).
// Replaces {{MODEL_DIR}} placeholder with the actual model folder path.
std::string GenieExecutor::buildConfigJson(const std::string& modelFolder,
                                           const std::string& /*deviceType*/,
                                           uint32_t /*maxTokens*/,
                                           uint32_t /*contextSize*/) {
#ifdef GENIE_FLAG
  std::string configPath = modelFolder + "/genie_config_npu.json";
  std::ifstream f(configPath);
  if (!f) {
    LOG(FATAL) << "Genie: NPU config not found at " << configPath;
    return "";
  }
  LOG(INFO) << "Genie: using config " << configPath;
  std::string json((std::istreambuf_iterator<char>(f)),
                   std::istreambuf_iterator<char>());
  const std::string placeholder = "{{MODEL_DIR}}";
  for (size_t p = 0; (p = json.find(placeholder, p)) != std::string::npos;
       p += modelFolder.size())
    json.replace(p, placeholder.size(), modelFolder);
  return json;
#else
  return "";
#endif
}

// ---------------------------------------------------------------------------
// Static token callback
// ---------------------------------------------------------------------------

#ifdef GENIE_FLAG
void GenieExecutor::tokenToTokenCallback(
    const uint32_t* token, uint32_t tokensLength,
    GenieDialog_SentenceCode_t sentenceCode, const void* /*userData*/) {
  switch (sentenceCode) {
    case GENIE_DIALOG_SENTENCE_COMPLETE:
      break;
    case GENIE_DIALOG_SENTENCE_BEGIN:
      break;
    case GENIE_DIALOG_SENTENCE_CONTINUE:
      break;
    case GENIE_DIALOG_SENTENCE_END:
      break;
    case GENIE_DIALOG_SENTENCE_ABORT:
      return;  // stop on abort
    default:
      break;
  }
  if (token) {
    // Fire first-token callback before accumulating — avoids checking ftFired_
    // on every token
    if (!ftFired_ && ftCallback_) {
      ftCallback_(ftContext_);
      ftFired_ = true;
    }

    for (uint32_t i = 0; i < tokensLength; ++i) {
      outputTokens_.push_back(token[i]);
    }
  }
}
#endif

// ---------------------------------------------------------------------------
// Executor interface
// ---------------------------------------------------------------------------

void GenieExecutor::create(const char* model_path,
                           const char* /*native_lib_path*/) {
#ifdef GENIE_FLAG
  modelFolder_ = std::string(model_path);

  // Pre-flight: verify model folder and NPU config exist
  LOG(INFO) << "Genie: model_path=" << modelFolder_;
  {
    namespace fs = std::filesystem;
    if (!fs::exists(modelFolder_)) {
      LOG(FATAL) << "Genie: model path does not exist: " << modelFolder_;
      return;
    }
    std::string cfgPath = modelFolder_ + "/genie_config_npu.json";
    if (!fs::exists(cfgPath)) {
      LOG(FATAL) << "Genie: NPU config not found: " << cfgPath;
      return;
    }
  }

  std::string configJson =
      buildConfigJson(modelFolder_, deviceType_, maxNewTokens_, contextSize_);
  if (configJson.empty()) {
    LOG(FATAL) << "Genie: failed to build config JSON";
    return;
  }

  LOG(INFO) << "Genie: config JSON built, length=" << configJson.size();
  LOG(INFO) << "Genie: config JSON: " << configJson;

  GenieDialogConfig_Handle_t configHandle = nullptr;
  Genie_Status_t status =
      GenieDialogConfig_createFromJson(configJson.c_str(), &configHandle);
  if (GENIE_STATUS_SUCCESS != status || configHandle == nullptr) {
    LOG(FATAL) << "Genie: GenieDialogConfig_createFromJson failed, status="
               << status << " (" << genieStatusName(status) << ")";
    return;
  }

  status = GenieDialog_create(configHandle, &dialogHandle_);
  if (GENIE_STATUS_SUCCESS != status || dialogHandle_ == nullptr) {
    GenieDialogConfig_free(configHandle);
    LOG(FATAL) << "Genie: GenieDialog_create failed, status=" << status << " ("
               << genieStatusName(status) << ")";
    return;
  }

  GenieDialogConfig_free(configHandle);

  // Set I/O formats
  inputFormat_.clear();
  outputFormat_.clear();

  mlperf_data_t input0;
  input0.type = mlperf_data_t::Int32;
  input0.size = contextSize_;
  inputFormat_.push_back(input0);

  // Second input: token limit (single int32)
  mlperf_data_t input1;
  input1.type = mlperf_data_t::Int32;
  input1.size = 1;
  inputFormat_.push_back(input1);

  mlperf_data_t output;
  output.type = mlperf_data_t::Int32;
  output.size = maxNewTokens_;
  outputFormat_.push_back(output);

  // Pre-reserve output buffer to avoid reallocation during first query's token
  // generation
  outputTokens_.reserve(maxNewTokens_);

  LOG(INFO) << "Genie Dialog Initialized successfully";
#else
  LOG(ERROR) << "Genie support not compiled in";
#endif
}

mlperf_status_t GenieExecutor::set_input(int32_t /*batchIndex*/,
                                         int32_t inputIndex, void* data) {
#ifdef GENIE_FLAG
  if (!data) {
    LOG(ERROR) << "Genie: set_input called with null data";
    return MLPERF_FAILURE;
  }
  if (inputIndex == 0) {
    // Input 0: pointer to std::vector<int> of token IDs — store pointer
    // directly, avoiding a copy. Caller guarantees lifetime for the duration of
    // the query.
    const auto* tokenVec = static_cast<const std::vector<int>*>(data);
    if (tokenVec && !tokenVec->empty()) {
      inputTokens_ = tokenVec;
    }
  } else if (inputIndex == 1) {
    // Input 1: token limit (single int32) — only update if value changed
    int32_t limit = *static_cast<const int32_t*>(data);
    if (limit > 0 && static_cast<uint32_t>(limit) != maxNewTokens_) {
      maxNewTokens_ = static_cast<uint32_t>(limit);
      if (dialogHandle_) {
        GenieDialog_setMaxNumTokens(dialogHandle_, maxNewTokens_);
      }
    }
  }
  return MLPERF_SUCCESS;
#else
  LOG(ERROR) << "Genie support not compiled in";
  return MLPERF_FAILURE;
#endif
}

mlperf_status_t GenieExecutor::execute(ft_callback callback, void* context) {
#ifdef GENIE_FLAG
  if (!dialogHandle_) {
    LOG(ERROR) << "Genie: execute called but dialog not initialized";
    return MLPERF_FAILURE;
  }
  if (!inputTokens_ || inputTokens_->empty()) {
    LOG(ERROR) << "Genie: execute called with no input tokens";
    return MLPERF_FAILURE;
  }

  outputTokens_.clear();

  // Arm first-token callback
  ftCallback_ = callback;
  ftContext_ = context;
  ftFired_ = false;

  Genie_Status_t status = GenieDialog_tokenQuery(
      dialogHandle_, reinterpret_cast<const uint32_t*>(inputTokens_->data()),
      static_cast<uint32_t>(inputTokens_->size()),
      GENIE_DIALOG_SENTENCE_COMPLETE, tokenToTokenCallback, nullptr);

  LOG(INFO) << "Genie: tokenQuery done output_tokens=" << outputTokens_.size();

  if (GENIE_STATUS_SUCCESS != status &&
      GENIE_STATUS_WARNING_CONTEXT_EXCEEDED != status) {
    LOG(ERROR) << "Genie: GenieDialog_tokenQuery failed, status=" << status;
    return MLPERF_FAILURE;
  }

  return MLPERF_SUCCESS;
#else
  LOG(ERROR) << "Genie support not compiled in";
  return MLPERF_FAILURE;
#endif
}

mlperf_status_t GenieExecutor::get_output(uint32_t /*batchIndex*/,
                                          int32_t outputIndex, void** data) {
#ifdef GENIE_FLAG
  if (outputIndex >= static_cast<int32_t>(outputFormat_.size())) {
    LOG(ERROR) << "Genie: invalid output index " << outputIndex;
    return MLPERF_FAILURE;
  }
  *data = &outputTokens_;
  return MLPERF_SUCCESS;
#else
  LOG(ERROR) << "Genie support not compiled in";
  return MLPERF_FAILURE;
#endif
}

void* GenieExecutor::getBuffer(size_t /*n*/) {
  LOG(WARNING) << "Genie: getBuffer not used";
  return nullptr;
}

void GenieExecutor::deregister(void* /*p*/) {
  LOG(WARNING) << "Genie: deregister not used";
}

const char* GenieExecutor::get_name_() const { return name_; }

bool GenieExecutor::getUseIonBuffers_() const { return false; }

std::vector<mlperf_data_t> GenieExecutor::getInputFormat_() const {
  return inputFormat_;
}

std::vector<mlperf_data_t> GenieExecutor::getOutputFormat_() const {
  return outputFormat_;
}

void GenieExecutor::setConfigs(const mlperf_backend_configuration_t* configs) {
  for (int i = 0; i < configs->count; ++i) {
    if (strcmp(configs->keys[i], "device_type") == 0) {
      deviceType_ = std::string(configs->values[i]);
    } else if (strcmp(configs->keys[i], "max_new_tokens") == 0) {
      maxNewTokens_ = static_cast<uint32_t>(atoi(configs->values[i]));
    } else if (strcmp(configs->keys[i], "context_size") == 0) {
      contextSize_ = static_cast<uint32_t>(atoi(configs->values[i]));
    }
  }
  LOG(INFO) << "Genie | device_type: " << deviceType_
            << " | max_new_tokens: " << maxNewTokens_
            << " | context_size: " << contextSize_;
}

void GenieExecutor::flush() {
#ifdef GENIE_FLAG
  if (dialogHandle_) {
    GenieDialog_reset(dialogHandle_);
  }
#endif
  inputTokens_ = nullptr;  // release pointer after query completes
}

GenieExecutor::~GenieExecutor() {
#ifdef GENIE_FLAG
  if (dialogHandle_) {
    Genie_Status_t status = GenieDialog_free(dialogHandle_);
    if (GENIE_STATUS_SUCCESS != status) {
      LOG(ERROR) << "Genie: GenieDialog_free failed, status=" << status;
    }
    dialogHandle_ = nullptr;
  }
  outputTokens_.clear();
#endif
}
