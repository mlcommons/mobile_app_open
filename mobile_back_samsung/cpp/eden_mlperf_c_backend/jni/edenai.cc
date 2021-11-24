/* Copyright (c) 2020-2021 Samsung Electronics Co., Ltd. All rights reserved.

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
#include "edenai.h"

#include <android/log.h>

#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#define LOG_TAG "EdenAI"

#include <dlfcn.h>
#include <fcntl.h>

#include "backend_c.h"
#include "exynos_ofi_wrapper.h"
#include "samsung_settings.h"

#define NUM_MIXED_MODEL_FILES 16
#define NUM_OF_INIT_DSP_BATCH 2
#define NUM_OF_DSP_BATCH 128

void* LoadFunction(void* dl_handle, const char* name) {
  auto func_pt = dlsym(dl_handle, name);
  return func_pt;
}

#define LOAD_FUNCTION(dl_handle, method_name)              \
  method_name##Func = reinterpret_cast<method_name##_fn*>( \
      LoadFunction(dl_handle, #method_name))

namespace EdenAI {

const uint32_t EDEN_NN_TIMEOUT = UINT32_MAX;

int makeBufferCount = 0;

uint32_t ModelId0 = INVALID_MODEL_ID;
EdenBuffer* inputBuffers0[NUM_OF_BUFF_MAX];
EdenBuffer* outputBuffers0[NUM_OF_BUFF_MAX];

uint32_t ModelId1 = INVALID_MODEL_ID;
EdenBuffer* inputBuffers1[NUM_OF_BUFF_MAX];
EdenBuffer* outputBuffers1[NUM_OF_BUFF_MAX];

uint32_t ModelId2 = INVALID_MODEL_ID;
EdenBuffer* inputBuffers2[NUM_OF_BUFF_MAX];
EdenBuffer* outputBuffers2[NUM_OF_BUFF_MAX];

uint32_t ModelIdDsp = INVALID_MODEL_ID;
EdenBuffer* inputBuffers3[NUM_OF_INIT_DSP_BATCH];
EdenBuffer* outputBuffers3[NUM_OF_INIT_DSP_BATCH];
int inputSize = 0;
int outputSize = 0;

bool MULTI_NPU_ENABLED = false;
EdenPreference preference;
HwPreference hwPreference = NPU_ONLY;
RequestMode requestMode = RequestMode::BLOCK;
ModePreference modePreference = BOOST_MODE;
bool gIsOfflineModel = false;
MLPERF_MODEL_TYPE gTestModelType = MLPERF_MODEL_TYPE_NONE;
const char* dspModelPath = "/sdcard/MLPerf_sideload/mobilenet_edgetpu_out.cgo";

std::vector<int32_t> vecOutputSize;

inline void _fillRequest(EdenRequest* request, uint32_t modelId,
                         EdenBuffer* inputBuffer, EdenBuffer* outputBuffer) {
  request->modelId = modelId;
  request->inputBuffers = inputBuffer;
  request->outputBuffers = outputBuffer;
}

inline int fillRequest(EdenRequest* request, uint32_t modelId,
                       EdenBuffer* inputBuffer, EdenBuffer* outputBuffer,
                       char* src, size_t size, IMAGE_FORMAT image_format) {
  memcpy(inputBuffer->addr, src, size);
  _fillRequest(request, modelId, inputBuffer, outputBuffer);
  return RET_OK;
}

inline int fillDspRequest(EdenRequest* request, uint32_t modelId,
                          EdenBuffer* inputBuffer, EdenBuffer* outputBuffer,
                          char* src, size_t size, IMAGE_FORMAT image_format) {
  if (ExynosOFI_CopyToBuffer(inputBuffer, 0, src, size, image_format) !=
      RET_OK) {
    MLOGE("load input failed");
    return RET_ERROR_ON_RT_LOAD_BUFFERS;
  }
  _fillRequest(request, modelId, inputBuffer, outputBuffer);
  return RET_OK;
}

bool dspOpenModel(uint32_t& modelId, const char* path,
                  EdenBuffer** inputBuffers, EdenBuffer** outputBuffers,
                  const int num_buffers) {
  EdenModelFile modelFile = makeModel(path);
  int ret = ExynosOFI_OpenModel(&modelFile, &modelId, preference);
  if (ret != RET_OK) {
    MLOGE("open dsp model failed");
    return false;
  }
  for (int i = 0; i < num_buffers; i++) {
    int32_t tempNumber = 0;
    ret =
        ExynosOFI_AllocateInputBuffers(modelId, &inputBuffers[i], &tempNumber);
    if (ret != RET_OK) {
      MLOGE("Fail to Allocate input buffer, ret = %d", ret);
      return false;
    }
    ret = ExynosOFI_AllocateOutputBuffers(modelId, &outputBuffers[i],
                                          &tempNumber);
    if (ret != RET_OK) {
      MLOGE("Fail to Allocate output buffer, ret = %d", ret);
      return false;
    }
  }
  return true;
}

bool dspOpenModelFromMemory(uint32_t& modelId, char* va, size_t size,
                            EdenBuffer** inputBuffers,
                            EdenBuffer** outputBuffers, const int num_buffers) {
  int ret;
  EdenModelOptions dummy_options;
  ret = ExynosOFI_OpenEdenModelFromMemory(MODEL_TYPE_IN_MEMORY_TFLITE,
                                          reinterpret_cast<int8_t*>(va), size,
                                          false, &modelId, dummy_options);
  if (ret != RET_OK) {
    MLOGE("Open DSP model from memory failed.");
    return false;
  }
  for (int i = 0; i < num_buffers; i++) {
    int32_t dummy = 0;
    ret = ExynosOFI_AllocateInputBuffers(modelId, &inputBuffers[i], &dummy);
    if (ret != RET_OK) {
      MLOGE("Fail to Allocate input buffer, ret = %d", ret);
      return false;
    }
    ret = ExynosOFI_AllocateOutputBuffers(modelId, &outputBuffers[i], &dummy);
    if (ret != RET_OK) {
      MLOGE("Fail to Allocate output buffer, ret = %d", ret);
      return false;
    }
  }
  return true;
}

void setHwPreference(int hp) { hwPreference = (HwPreference)hp; }

uint32_t getInputSize() {
  int32_t w, h, c, n;
  GetInputBufferShapeFunc(ModelId0, 0, &w, &h, &c, &n);
  return w * h * c * n;
}

uint32_t getOutputSize() {
  int32_t w, h, c, n;
  GetOutputBufferShapeFunc(ModelId0, 0, &w, &h, &c, &n);
  if (GetOutputBufferShapeFunc == ExynosOFI_GetOutputBufferShape) {
    w /= 4;
  }
  return w * h * c * n;
}

void setInputSize(int size) { inputSize = size; }

void setOutputSize(int size) { outputSize = size; }

void edenNotify(addr_t* addr, addr_t value) { *addr = value; }

int32_t edenWaitFor(addr_t* addr, uint32_t value, uint32_t timeout) {
  while (timeout--) {
    usleep(1);
    if (*addr != INVALID_REQUEST_ID) break;
  }
  if (timeout == 0) {
    return -1;
  }
  return RET_OK;
}

int32_t edenWaitForAll(EdenCallback* callbacks, uint32_t timeout,
                       uint32_t batch, char* output, EdenRequest* requests,
                       int oneFrameSize, int outputFrameSize, addr_t* requestId,
                       Backend* ptr) {
  bool all_done, all_dsp_done;
  const int COPIED = 1;
  const int NOT_COPIED = 0;
  const int dsp_batch = (ModelIdDsp != INVALID_MODEL_ID) ? NUM_OF_DSP_BATCH : 0;
  const int dsp_init_batch =
      (ModelIdDsp != INVALID_MODEL_ID) ? NUM_OF_INIT_DSP_BATCH : 0;
  int dsp_idx = batch + dsp_init_batch;
  int dsp_idx_end = batch + dsp_batch;

  int scoreBoard[batch + dsp_batch];
  for (int i = 0; i < batch + dsp_batch; i++) {
    scoreBoard[i] = NOT_COPIED;
  }

  int idx = NUM_OF_BUFF_MAX * 3;
  int offset;

  EdenPreference pref;
  pref.mode = modePreference;
  pref.hw = hwPreference;

  EdenRequestOptions options;
  options.userPreference = pref;
  options.requestMode = requestMode;
  options.reserved[0] = {
      0,
  };

  int i = 0;
  while (timeout--) {
    usleep(1);
    all_done = true;
    all_dsp_done = true;
    for (; i < batch; i++) {
      if (callbacks[i].requestId != INVALID_REQUEST_ID &&
          scoreBoard[i] == NOT_COPIED) {
        offset = i * outputFrameSize;
        vecOutputSize.push_back(requests[i].outputBuffers->size);
        memcpy(output + offset, requests[i].outputBuffers->addr,
               requests[i].outputBuffers->size);
        scoreBoard[i] = COPIED;
        if (idx < batch) {
          if (fillRequest(&requests[idx], requests[i].modelId,
                          requests[i].inputBuffers, requests[i].outputBuffers,
                          (char*)ptr->input_conv->at(idx), oneFrameSize,
                          IMAGE_FORMAT_BGRC) == RET_OK &&
              ExecuteEdenModel(&requests[idx], &requestId[idx], options) ==
                  RET_OK) {
            idx++;
          } else {
            MLOGE("edenWaitForAll ExecuteModel() failed ");
          }
        }
      }
      all_done = all_done && (callbacks[i].requestId != INVALID_REQUEST_ID);
    }
    for (; ModelIdDsp && i < dsp_idx_end; i++) {
      auto req = reinterpret_cast<exynos_nn_request_t*>(&requests[i]);
      if (scoreBoard[i] == NOT_COPIED && req->result != (int32_t)0xffffffff) {
        ExynosOFI_CopyFromBuffer(output + i * outputFrameSize,
                                 requests[i].outputBuffers, outputFrameSize);
        scoreBoard[i] = COPIED;
        if (dsp_idx < dsp_idx_end) {
          if (fillDspRequest(&requests[dsp_idx], requests[i].modelId,
                             requests[i].inputBuffers,
                             requests[i].outputBuffers,
                             (char*)ptr->input_conv->at(dsp_idx), oneFrameSize,
                             IMAGE_FORMAT_BGRC) == RET_OK &&
              ExynosOFI_ExecuteEdenModel(
                  &requests[dsp_idx], &requestId[dsp_idx], options) == RET_OK) {
            dsp_idx++;
          } else {
            MLOGE("execute dsp failed ");
          }
        }
      }
      all_dsp_done = all_dsp_done && (req->result != (int32_t)0xffffffff);
    }
    if (all_done && all_dsp_done) {
      break;
    }
    i = 0;
    if (all_done) {
      // skip NPU check in next iteration
      i = batch;
    }
    if (all_dsp_done) {
      // skip DSP check in next iteration
      dsp_idx_end = batch;
    }
  }
  if (timeout == 0) {
    MLOGE("edenWaitForAll timeout");
    return -1;
  }
  return RET_OK;
}

EdenModelFile makeModel(const char* pathToModelFile) {
  EdenModelFile modelFile;
  modelFile.modelFileFormat = TFLITE;
  modelFile.pathToModelFile = (int8_t*)pathToModelFile;
  modelFile.lengthOfPath = strlen(pathToModelFile) + 1;
  modelFile.pathToWeightBiasFile = nullptr;
  modelFile.lengthOfWeightBias = 0;
  return modelFile;
}

bool jni_Initialize() {
  int ret = 0;

  ret = InitializeFunc();
  if (ret != RET_OK) {
    MLOGE("jni_Initialize false");
    return false;
  }
  MLOGD("jni_Initialize true");
  ExynosOFI_Initialize();
  return true;
}

bool jni_OpenModelFromMemory(char* bufferMain, int lengthMain,
                             char* bufferOther, int lengthOther, int device,
                             int batch) {
  MLOGD("(+)");
  int ret = 0;

  EdenModelOptions options;
  options.modelPreference = {
      {(HwPreference)device, modePreference, {false, false}}, EDEN_NN_API};
  options.priority = P_DEFAULT;
  options.boundCore = NPU_UNBOUND;

  ModelId0 = INVALID_MODEL_ID;
  ModelId1 = INVALID_MODEL_ID;
  ModelId2 = INVALID_MODEL_ID;
  ModelIdDsp = INVALID_MODEL_ID;

  if (options.modelPreference.userPreference.hw == NPU_ONLY) {
    // OpenModel for 3 Core
    ret = OpenEdenModelFromMemory(MODEL_TYPE_IN_MEMORY_TFLITE,
                                  reinterpret_cast<int8_t*>(bufferMain),
                                  lengthMain, false, &ModelId0, options);

    if (ret != RET_OK) {
      MLOGE("Fail to OpenModel0!, ret = %d", ret);
      return false;
    }

    if (batch > 1) {
      ret = OpenEdenModelFromMemory(MODEL_TYPE_IN_MEMORY_TFLITE,
                                    reinterpret_cast<int8_t*>(bufferMain),
                                    lengthMain, false, &ModelId1, options);
      if (ret != RET_OK) {
        MLOGE("Fail to OpenModel1!, ret = %d", ret);
        return false;
      }

      ret = OpenEdenModelFromMemory(MODEL_TYPE_IN_MEMORY_TFLITE,
                                    reinterpret_cast<int8_t*>(bufferMain),
                                    lengthMain, false, &ModelId2, options);
      if (ret != RET_OK) {
        MLOGE("Fail to OpenModel2!, ret = %d", ret);
        return false;
      }

      if (bufferOther && lengthOther) {
        if (!dspOpenModelFromMemory(ModelIdDsp, bufferOther, lengthOther,
                                    inputBuffers3, outputBuffers3,
                                    NUM_OF_INIT_DSP_BATCH)) {
          MLOGE("Fail to OpenModel3!, ret = %d", ret);
          return false;
        }
      }

      MULTI_NPU_ENABLED = true;
    }

  } else if (options.modelPreference.userPreference.hw == GPU_ONLY) {
    // OpenModel for GPU
    ret = OpenEdenModelFromMemoryFunc(MODEL_TYPE_IN_MEMORY_TFLITE,
                                      reinterpret_cast<int8_t*>(bufferMain),
                                      lengthMain, false, &ModelId0, options);
    if (ret != RET_OK) {
      MLOGE("Fail to OpenModel0() GPU!, ret = %d", ret);
      return false;
    }
    MULTI_NPU_ENABLED = false;
  } else if (options.modelPreference.userPreference.hw == DSP_ONLY) {
    return false;
  }
  for (int32_t i = 0; i < NUM_OF_BUFF_MAX; i++) {
    int32_t tempNumber = 0;
    if (ModelId0 != INVALID_MODEL_ID) {
      ret = AllocateInputBuffersFunc(ModelId0, &inputBuffers0[i], &tempNumber);
      if (ret != RET_OK) {
        MLOGE("Fail to Allocate input buffer NPU0, ret = %d", ret);
        return false;
      }
      ret =
          AllocateOutputBuffersFunc(ModelId0, &outputBuffers0[i], &tempNumber);
      if (ret != RET_OK) {
        MLOGE("Fail to Allocate output buffer NPU0, ret = %d", ret);
        return false;
      }
    }
    if (ModelId1 != INVALID_MODEL_ID) {
      ret = AllocateInputBuffers(ModelId1, &inputBuffers1[i], &tempNumber);
      if (ret != RET_OK) {
        MLOGE("Fail to Allocate input buffer NPU1, ret = %d", ret);
        return false;
      }
      ret = AllocateOutputBuffers(ModelId1, &outputBuffers1[i], &tempNumber);
      if (ret != RET_OK) {
        MLOGE("Fail to Allocate output buffer NPU1, ret = %d", ret);
        return false;
      }
    }
    if (ModelId2 != INVALID_MODEL_ID) {
      ret = AllocateInputBuffers(ModelId2, &inputBuffers2[i], &tempNumber);
      if (ret != RET_OK) {
        MLOGE("Fail to Allocate input buffer NPU2, ret = %d", ret);
        return false;
      }
      ret = AllocateOutputBuffers(ModelId2, &outputBuffers2[i], &tempNumber);
      if (ret != RET_OK) {
        MLOGE("Fail to Allocate output buffer NPU2, ret = %d", ret);
        return false;
      }
    }
  }
  return true;
}

bool jni_OpenModel(const char* pathToModelFile, int device, int batch) {
  int ret = 0;

  preference.hw = (HwPreference)device;
  preference.mode = modePreference;

  preference.inputBufferMode.enable = false;
  preference.inputBufferMode.setInputAsFloat = false;

  EdenModelFile modelFile = makeModel(pathToModelFile);
  ModelId0 = INVALID_MODEL_ID;
  ModelId1 = INVALID_MODEL_ID;
  ModelId2 = INVALID_MODEL_ID;
  ModelIdDsp = INVALID_MODEL_ID;

  MLOGD("pathToModelFile=%s", pathToModelFile);
  if (preference.hw == NPU_ONLY) {
    ret = OpenModel(&modelFile, &ModelId0, preference);
    if (ret != RET_OK) {
      return false;
    }

    if (batch > 1) {
      ret = OpenModel(&modelFile, &ModelId1, preference);
      if (ret != RET_OK) {
        return false;
      }
      ret = OpenModel(&modelFile, &ModelId2, preference);
      if (ret != RET_OK) {
        return false;
      }

      if (!dspOpenModel(ModelIdDsp, dspModelPath, inputBuffers3, outputBuffers3,
                        NUM_OF_INIT_DSP_BATCH)) {
        return false;
      }

      MULTI_NPU_ENABLED = true;
    }

  } else if (preference.hw == GPU_ONLY) {
    ret = OpenModelFunc(&modelFile, &ModelId0, preference);
    if (ret != RET_OK) {
      return false;
    }
    MULTI_NPU_ENABLED = false;
  } else if (preference.hw == DSP_ONLY) {
    MULTI_NPU_ENABLED = false;
    return dspOpenModel(ModelId0, pathToModelFile, inputBuffers0,
                        outputBuffers0, NUM_OF_BUFF_MAX);
  }

  for (int32_t i = 0; i < NUM_OF_BUFF_MAX; i++) {
    int32_t tempNumber = 0;
    if (ModelId0 != INVALID_MODEL_ID) {
      ret = AllocateInputBuffersFunc(ModelId0, &inputBuffers0[i], &tempNumber);
      if (ret != RET_OK) {
        MLOGE("Fail to Allocate input buffer NPU0, ret = %d", ret);
        return false;
      }
      ret =
          AllocateOutputBuffersFunc(ModelId0, &outputBuffers0[i], &tempNumber);
      if (ret != RET_OK) {
        MLOGE("Fail to Allocate output buffer NPU0, ret = %d", ret);
        return false;
      }
    }
    if (ModelId1 != INVALID_MODEL_ID) {
      ret = AllocateInputBuffers(ModelId1, &inputBuffers1[i], &tempNumber);
      if (ret != RET_OK) {
        MLOGE("Fail to Allocate input buffer NPU1, ret = %d", ret);
        return false;
      }
      ret = AllocateOutputBuffers(ModelId1, &outputBuffers1[i], &tempNumber);
      if (ret != RET_OK) {
        MLOGE("Fail to Allocate output buffer NPU1, ret = %d", ret);
        return false;
      }
    }
    if (ModelId2 != INVALID_MODEL_ID) {
      ret = AllocateInputBuffers(ModelId2, &inputBuffers2[i], &tempNumber);
      if (ret != RET_OK) {
        MLOGE("Fail to Allocate input buffer NPU2, ret = %d", ret);
        return false;
      }
      ret = AllocateOutputBuffers(ModelId2, &outputBuffers2[i], &tempNumber);
      if (ret != RET_OK) {
        MLOGE("Fail to Allocate output buffer NPU2, ret = %d", ret);
        return false;
      }
    }
  }
  return true;
}

int sample_notify_callback(int num_param, void** params) {
  if (num_param != 1) {
    MLOGE("Wrong params. num_param(%d != 1)\n", num_param);
  }
  // MLOGD("User notify callback-ed! req=%p", params[0]);
  return 0;
}

/* OFI Test */
int sample_user_wait_for(exynos_nn_request_t* req) {
  while (req->result == (int32_t)0xffffffff) {
    // MLOGD("User waiting.. result(0x%x) addr:%p", req->result, req);
    usleep(1000);
  }
  if (req->result != RET_OK) {
    MLOGE("Execute done but failed.");
    return req->result;
  }
  return 0;
}

bool jni_ExecuteModelBatch(char* input, char* output, bool isMobileBert,
                           Backend* ptr) {
  int ret = RET_OK;
  vecOutputSize.clear();

  int batch = ptr->batch;
  int dsp_batch = 0;
  int dsp_init_batch = 0;
  std::unique_ptr<void*[]> cb_param_ptr;
  auto dsp_callbacks =
      std::make_unique<exynos_nn_callback_t[]>(batch + dsp_batch);

  if (ModelIdDsp != INVALID_MODEL_ID) {
    dsp_batch = NUM_OF_DSP_BATCH;
    dsp_init_batch = NUM_OF_INIT_DSP_BATCH;
    cb_param_ptr = std::make_unique<void*[]>(batch + dsp_batch);
  }
  batch -= dsp_batch;

  EdenRequest* requests = new EdenRequest[batch + dsp_batch];
  EdenCallback* callbacks = new EdenCallback[batch + dsp_batch];
  addr_t* requestId = new addr_t[batch + dsp_batch];

  EdenPreference pref;
  pref.mode = modePreference;
  pref.hw = hwPreference;

  EdenRequestOptions options;
  options.userPreference = pref;
  options.requestMode = requestMode;
  options.reserved[0] = {
      0,
  };

  if (hwPreference != DSP_ONLY) {
    for (int i = 0; i < batch; i++) {
      callbacks[i].notify = edenNotify;
      callbacks[i].waitFor = edenWaitFor;
      callbacks[i].requestId = INVALID_REQUEST_ID;
      callbacks[i].executionResult.inference.retCode = RET_OK;

      requestId[i] = INVALID_REQUEST_ID;
      requests[i].callback = &callbacks[i];
      requests[i].hw = hwPreference;
    }
  } else {
    for (int i = 0; i < batch; i++) {
      /* set callback_func param. */
      cb_param_ptr[i] = reinterpret_cast<void*>(&requests[i]);
      dsp_callbacks.get()[i].num_param = 1;
      dsp_callbacks.get()[i].param = &cb_param_ptr[i];
      dsp_callbacks.get()[i].notify = sample_notify_callback;

      requestId[i] = INVALID_REQUEST_ID;
      requests[i].callback =
          reinterpret_cast<EdenCallback*>(&dsp_callbacks.get()[i]);
      reinterpret_cast<exynos_nn_request_t*>(&requests[i])->result = 0xffffffff;
    }
  }
  for (int i = batch; i < batch + dsp_batch; i++) {
    cb_param_ptr[i] = reinterpret_cast<void*>(&requests[i]);
    dsp_callbacks.get()[i].num_param = 1;
    dsp_callbacks.get()[i].param = &cb_param_ptr[i];
    dsp_callbacks.get()[i].notify = sample_notify_callback;
    requestId[i] = INVALID_REQUEST_ID;
    requests[i].callback =
        reinterpret_cast<EdenCallback*>(&dsp_callbacks.get()[i]);
    reinterpret_cast<exynos_nn_request_t*>(&requests[i])->result = 0xffffffff;
  }

  const int OCCUPIED = 1;
  const int AVAILABLE = 0;

  int checkBufferBoard0[NUM_OF_BUFF_MAX] = {
      AVAILABLE,
  };

  int checkBufferBoard1[NUM_OF_BUFF_MAX] = {
      AVAILABLE,
  };

  int checkBufferBoard2[NUM_OF_BUFF_MAX] = {
      AVAILABLE,
  };

  int executedCount = 0;
  int oneFrameSize = inputSize / (batch + dsp_batch);
  int outputFrameSize = outputSize / (batch + dsp_batch);
  for (int i = 0; i < NUM_OF_BUFF_MAX; i++) {
    if (checkBufferBoard0[i] == AVAILABLE && ModelId0 != INVALID_MODEL_ID) {
      if (hwPreference == DSP_ONLY) {
        fillDspRequest(&requests[executedCount], ModelId0, inputBuffers0[i],
                       outputBuffers0[i],
                       (char*)ptr->input_conv->at(executedCount), oneFrameSize,
                       IMAGE_FORMAT_BGR);
      } else {
        if (isMobileBert) {
          memcpy((inputBuffers0[i] + 0)->addr, input + inputSize * 4 * 0,
                 inputSize * 4);
          memcpy((inputBuffers0[i] + 1)->addr, input + inputSize * 4 * 1,
                 inputSize * 4);
          memcpy((inputBuffers0[i] + 2)->addr, input + inputSize * 4 * 2,
                 inputSize * 4);
          requests[executedCount].modelId = ModelId0;
          requests[executedCount].inputBuffers = inputBuffers0[i];
          requests[executedCount].outputBuffers = outputBuffers0[i];
          GPUBoost::apply_GPU_boost();
        } else {
          fillRequest(&requests[executedCount], ModelId0, inputBuffers0[i],
                      outputBuffers0[i],
                      (char*)ptr->input_conv->at(executedCount), oneFrameSize,
                      IMAGE_FORMAT_BGRC);
        }
      }
      if (ExecuteEdenModelFunc(&requests[executedCount],
                               &requestId[executedCount], options) != RET_OK) {
        MLOGE("execute model failed");
      } else {
        executedCount++;
      }
      checkBufferBoard0[i] = OCCUPIED;
      if (executedCount >= batch) {
        break;
      }
    }

    if (MULTI_NPU_ENABLED) {
      if (checkBufferBoard1[i] == AVAILABLE && ModelId1 != INVALID_MODEL_ID) {
        memcpy(inputBuffers1[i]->addr,
               (char*)ptr->input_conv->at(executedCount), oneFrameSize);
        requests[executedCount].modelId = ModelId1;
        requests[executedCount].inputBuffers = inputBuffers1[i];
        requests[executedCount].outputBuffers = outputBuffers1[i];

        ret = ExecuteEdenModel(&requests[executedCount],
                               &requestId[executedCount], options);
        if (ret != RET_OK) {
          MLOGE("execute model failed");
        } else {
          executedCount++;
        }
        checkBufferBoard1[i] = OCCUPIED;
        if (executedCount >= batch) {
          break;
        }
      }

      if (checkBufferBoard2[i] == AVAILABLE && ModelId2 != INVALID_MODEL_ID) {
        memcpy(inputBuffers2[i]->addr,
               (char*)ptr->input_conv->at(executedCount), oneFrameSize);
        requests[executedCount].modelId = ModelId2;
        requests[executedCount].inputBuffers = inputBuffers2[i];
        requests[executedCount].outputBuffers = outputBuffers2[i];

        ret = ExecuteEdenModel(&requests[executedCount],
                               &requestId[executedCount], options);
        if (ret != RET_OK) {
        } else {
          executedCount++;
        }

        checkBufferBoard2[i] = OCCUPIED;
        if (executedCount >= batch) {
          break;
        }
      }
    }
  }
  if (ModelIdDsp != INVALID_MODEL_ID) {
    for (int i = 0; i < dsp_init_batch; ++i) {
      if (fillDspRequest(&requests[batch + i], ModelIdDsp, inputBuffers3[i],
                         outputBuffers3[i],
                         (char*)ptr->input_conv->at(batch + i), oneFrameSize,
                         IMAGE_FORMAT_BGRC) != RET_OK ||
          ExynosOFI_ExecuteEdenModel(
              &requests[batch + i], &requestId[batch + i], options) != RET_OK) {
        MLOGE("dsp model execution failed");
      }
    }
  }

  if (batch == 1) {
    do {
      if (hwPreference == DSP_ONLY && requestMode == RequestMode::NONBLOCK) {
        for (int i = 0; i < batch; i++) {
          if (sample_user_wait_for(reinterpret_cast<exynos_nn_request_t*>(
                  &requests[i])) != RET_OK) {
            MLOGE("callback error");
          }
        }
        ExynosOFI_CopyFromBuffer(output, requests[0].outputBuffers,
                                 outputFrameSize);
      } else if (requestMode == RequestMode::NONBLOCK &&
                 callbacks[0].waitFor(&callbacks[0].requestId, requestId[0],
                                      EDEN_NN_TIMEOUT) < 0) {
        ret = RET_ERROR_ON_RT_EXECUTE_MODEL;
        MLOGE("callbackThread RET_ERROR_ON_RT_EXECUTE_MODEL");
        break;
      }
      vecOutputSize.push_back(requests[0].outputBuffers->size);

      if (isMobileBert) {
        memcpy(output + outputSize * 0, (requests[0].outputBuffers + 0)->addr,
               outputSize * 4);
        memcpy(output + outputSize * 4, (requests[0].outputBuffers + 1)->addr,
               outputSize * 4);
      } else {
        memcpy(output, requests[0].outputBuffers->addr,
               requests[0].outputBuffers->size);
      }

      if (requests[executedCount].modelId == ModelId0 &&
          ModelId0 != INVALID_MODEL_ID) {
        checkBufferBoard0[0] = AVAILABLE;
      }
    } while (0);
  } else {
    do {
      if (hwPreference == DSP_ONLY && requestMode == RequestMode::NONBLOCK) {
        int idx = NUM_OF_BUFF_MAX;
        for (int i = 0; i < batch; i++) {
          if (sample_user_wait_for(reinterpret_cast<exynos_nn_request_t*>(
                  &requests[i])) != RET_OK) {
            MLOGE("callback error");
          }
          int offset = i * outputFrameSize;
          auto mem =
              reinterpret_cast<exynos_nn_memory_t*>(requests[i].outputBuffers);
          memcpy(output + offset, mem->address, mem->size);
          if (idx < batch) {
            if (fillDspRequest(&requests[idx], requests[i].modelId,
                               requests[i].inputBuffers,
                               requests[i].outputBuffers,
                               (char*)ptr->input_conv->at(idx), oneFrameSize,
                               IMAGE_FORMAT_BGR) == RET_OK &&
                ExecuteEdenModelFunc(&requests[idx], &requestId[idx],
                                     options) == RET_OK) {
              idx++;
            } else {
              MLOGE("edenWaitForAll ExecuteModel() failed");
            }
          }
        }
      } else if (requestMode == RequestMode::NONBLOCK &&
                 edenWaitForAll(callbacks, EDEN_NN_TIMEOUT, batch, output,
                                requests, oneFrameSize, outputFrameSize,
                                requestId, ptr) < 0) {
        ret = RET_ERROR_ON_RT_EXECUTE_MODEL;
        MLOGE("callbackThread RET_ERROR_ON_RT_EXECUTE_MODEL");
        break;
      }
      for (int i = 0; i < NUM_OF_BUFF_MAX; i++) {
        checkBufferBoard0[i] = AVAILABLE;
        checkBufferBoard1[i] = AVAILABLE;
        checkBufferBoard2[i] = AVAILABLE;
      }
    } while (0);
  }

  delete[] requests;
  delete[] callbacks;
  delete[] requestId;
  return true;
}

bool jni_CloseModel() {
  int ret = RET_OK;
  for (int32_t i = 0; i < NUM_OF_BUFF_MAX; i++) {
    if (ModelId0 != INVALID_MODEL_ID) {
      ret = FreeBuffersFunc(ModelId0, inputBuffers0[i]);
      ret = FreeBuffersFunc(ModelId0, outputBuffers0[i]);
      if (ret != RET_OK) {
        MLOGE("FreeBuffers failed ModelId0");
      }
      MLOGD("FreeBuffers ModelId0");
    }

    if (ModelId1 != INVALID_MODEL_ID) {
      ret = FreeBuffers(ModelId1, inputBuffers1[i]);
      ret = FreeBuffers(ModelId1, outputBuffers1[i]);
      if (ret != RET_OK) {
        MLOGE("FreeBuffers failed ModelId1");
      }
      MLOGD("FreeBuffers ModelId1");
    }

    if (ModelId2 != INVALID_MODEL_ID) {
      ret = FreeBuffers(ModelId2, inputBuffers2[i]);
      ret = FreeBuffers(ModelId2, outputBuffers2[i]);
      if (ret != RET_OK) {
        MLOGE("FreeBuffers failed ModelId2");
      }
      MLOGD("FreeBuffers ModelId2");
    }
  }
  if (ModelIdDsp != INVALID_MODEL_ID) {
    for (int32_t i = 0; i < NUM_OF_INIT_DSP_BATCH; i++) {
      ExynosOFI_FreeBuffers(ModelIdDsp, inputBuffers3[i]);
      ExynosOFI_FreeBuffers(ModelIdDsp, outputBuffers3[i]);
    }
  }

  if (ModelId0 != INVALID_MODEL_ID) {
    ret = CloseModelFunc(ModelId0);
    if (ret != RET_OK) {
      MLOGE("CloseModel failed ModelId0");
    }
    ModelId0 = INVALID_MODEL_ID;
    MLOGD("CloseModel ModelId0");
  }
  if (ModelId1 != INVALID_MODEL_ID) {
    ret = CloseModel(ModelId1);
    if (ret != RET_OK) {
      MLOGE("CloseModel failed ModelId1");
    }
    ModelId1 = INVALID_MODEL_ID;
    MLOGD("CloseModel ModelId1");
  }

  if (ModelId2 != INVALID_MODEL_ID) {
    ret = CloseModel(ModelId2);
    if (ret != RET_OK) {
      MLOGE("CloseModel failed ModelId2");
    }
    ModelId2 = INVALID_MODEL_ID;
    MLOGD("CloseModel ModelId2");
  }

  if (ModelIdDsp != INVALID_MODEL_ID) {
    ret = ExynosOFI_CloseModel(ModelIdDsp);
    if (ret != RET_OK) {
      MLOGE("CloseModel failed ModelIdDsp");
    }
    ModelIdDsp = INVALID_MODEL_ID;
  }
  return true;
}

bool jni_Shutdown() {
  int ret = ShutdownFunc();
  if (ret != RET_OK) {
    MLOGE("Shutdown failed");
  }

  if (ShutdownFunc != ExynosOFI_Shutdown) {
    ExynosOFI_Shutdown();
  }

  MLOGV("Shutdown");
  return true;
}

bool jni_GetEdenVersion(uint32_t modelId, int32_t* versions) {
  int ret = GetEdenVersion(modelId, versions);
  if (ret != RET_OK) {
    MLOGE("GetEdenVersion failed");
  }
  return true;
}

}  // namespace EdenAI

// Should return true if current hardware is supported.
bool mlperf_backend_matches_hardware(const char** not_allowed_message,
                                     const char** settings,
                                     mlperf_device_info_t* device_info) {
  MLOGD("Check for support  manufacturer: %s  model: %s",
        device_info->manufacturer, device_info->model);

  *not_allowed_message = nullptr;
  *settings = samsung_settings.c_str();

  char* isSamsung1 = NULL;
  isSamsung1 = strstr((char*)device_info->manufacturer, "samsung");
  char* isSamsung2 = NULL;
  isSamsung2 = strstr((char*)device_info->manufacturer, "Samsung");
  if (isSamsung1 || isSamsung2) {  // Samsung device
    MLOGD("This is Samsung device");

    char* isG998N = NULL;
    isG998N = strstr((char*)device_info->model, "SM-G998N");
    char* isG998B = NULL;
    isG998B = strstr((char*)device_info->model, "SM-G998B");
    char* isG996N = NULL;
    isG996N = strstr((char*)device_info->model, "SM-G996N");
    char* isG996B = NULL;
    isG996B = strstr((char*)device_info->model, "SM-G996B");
    char* isG991N = NULL;
    isG991N = strstr((char*)device_info->model, "SM-G991N");
    char* isG991B = NULL;
    isG991B = strstr((char*)device_info->model, "SM-G991B");
    if (isG998N || isG998B || isG996N || isG996B || isG991N ||
        isG991B) {  // S21, S21 ultra or Olympus engineering device
      MLOGD("This is S21 device");
      std::array<char, 128> buffer;
      std::string result;
      std::unique_ptr<FILE, decltype(&pclose)> pipe(
          popen("getprop | grep ro.hardware.chipname", "r"), pclose);
      if (!pipe) {
        throw std::runtime_error("popen() failed!");
      }
      while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
      }
      if (result.find("exynos2100") != std::string::npos) {
        MLOGD("This hardware is supported");
        return true;
      } else {  // S21 but not using exynos2100, unsuported, should stop search
                // for next backend, report unsupported
        MLOGD("This hardware is unsupported");
        *not_allowed_message = "Unsupported";
        return true;
      }
    } else {  // Samsung but not s21
      MLOGD("This hardware is unsupported");
      *not_allowed_message = "Unsupported";
      return true;
    }
  } else {  // not Samsung
    MLOGD("Soc Not supported. Trying next backend");
    *not_allowed_message = "UnsupportedSoc";
    return true;
  }
}
void mlperf_backend_clear(mlperf_backend_ptr_t backend_ptr) {
  MLOGD("mlperf_backend_clear");
  Backend* samsung_backend = (Backend*)backend_ptr;
  samsung_backend->input_format_.clear();
  samsung_backend->input_format_.shrink_to_fit();

  samsung_backend->output_format_.clear();
  samsung_backend->output_format_.shrink_to_fit();

  if (samsung_backend->outputs_buffer != nullptr) {
    samsung_backend->outputs_buffer->clear();
    samsung_backend->outputs_buffer->shrink_to_fit();
    delete samsung_backend->outputs_buffer;
    samsung_backend->outputs_buffer = nullptr;
  }

  if (samsung_backend->model_buffer != nullptr) {
    samsung_backend->model_buffer->clear();
    samsung_backend->model_buffer->shrink_to_fit();
    delete samsung_backend->model_buffer;
    samsung_backend->model_buffer = nullptr;
  }

  if (samsung_backend->input_conv != nullptr) {
    samsung_backend->input_conv->clear();
    samsung_backend->input_conv->shrink_to_fit();
    delete samsung_backend->input_conv;
    samsung_backend->input_conv = nullptr;
  }

  if (samsung_backend->detected_label_boxes != nullptr) {
    samsung_backend->detected_label_boxes->clear();
    samsung_backend->detected_label_boxes->shrink_to_fit();
    delete samsung_backend->detected_label_boxes;
    samsung_backend->detected_label_boxes = nullptr;
  }

  if (samsung_backend->detected_label_indices != nullptr) {
    samsung_backend->detected_label_indices->clear();
    samsung_backend->detected_label_indices->shrink_to_fit();
    delete samsung_backend->detected_label_indices;
    samsung_backend->detected_label_indices = nullptr;
  }

  if (samsung_backend->detected_label_probabilities != nullptr) {
    samsung_backend->detected_label_probabilities->clear();
    samsung_backend->detected_label_probabilities->shrink_to_fit();
    delete samsung_backend->detected_label_probabilities;
    samsung_backend->detected_label_probabilities = nullptr;
  }

  if (samsung_backend->num_detections != nullptr) {
    samsung_backend->num_detections->clear();
    samsung_backend->num_detections->shrink_to_fit();
    delete samsung_backend->num_detections;
    samsung_backend->num_detections = nullptr;
  }

  samsung_backend->input_size = 0;
  samsung_backend->output_size = 0;
  samsung_backend->batch = 0;

  // for mobile bert
  samsung_backend->isMobileBertModel_ = false;
  samsung_backend->m_inputs_.clear();
  samsung_backend->m_inputs_.shrink_to_fit();
  samsung_backend->m_outputs_.clear();
  samsung_backend->m_outputs_.shrink_to_fit();
  samsung_backend->out0_.clear();
  samsung_backend->out0_.shrink_to_fit();
  samsung_backend->out1_.clear();
  samsung_backend->out1_.shrink_to_fit();

  EdenAI::jni_CloseModel();
  EdenAI::jni_Shutdown();
  libeden_nn_on_direct = nullptr;
}

Backend ss_backend;

// Create a new backend and return the pointer to it.
mlperf_backend_ptr_t mlperf_backend_create(
    const char* model_path, mlperf_backend_configuration_t* configs,
    const char* native_lib_path) {
  Backend* samsung_backend = &ss_backend;
  if (samsung_backend->created) {
    mlperf_backend_clear(samsung_backend);
  }

  std::array<char, 128> buffer_kill;
  std::string result_kill;
  std::unique_ptr<FILE, decltype(&pclose)> pipe_kill(
      popen("killall android.hardware.neuralnetworks@1.3-service.eden-drv",
            "r"),
      pclose);
  if (!pipe_kill) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer_kill.data(), buffer_kill.size(), pipe_kill.get()) !=
         nullptr) {
    result_kill += buffer_kill.data();
  }
  MLOGD("killed eden drv");

  std::array<char, 128> buffer_kill2;
  std::string result_kill2;
  std::unique_ptr<FILE, decltype(&pclose)> pipe_kill2(
      popen("killall vendor.samsung_slsi.hardware.eden_runtime@1.0-service",
            "r"),
      pclose);
  if (!pipe_kill2) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer_kill2.data(), buffer_kill2.size(), pipe_kill2.get()) !=
         nullptr) {
    result_kill2 += buffer_kill2.data();
  }
  MLOGD("killed eden service");
  cpu_set_t mask;
  long nproc, i;
  pid_t tid = gettid();
  CPU_ZERO(&mask);
  CPU_SET(4, &mask);
  CPU_SET(5, &mask);
  CPU_SET(6, &mask);
  if (sched_setaffinity(tid, sizeof(mask), &mask) ||
      sched_getaffinity(tid, sizeof(cpu_set_t), &mask)) {
    MLOGE("Error setting up cpu affinity. tid: %d", tid);
  } else {
    nproc = sysconf(_SC_NPROCESSORS_ONLN);
    MLOGD("sched_getaffinity tid: %d", tid);
    for (i = 0; i < nproc; i++) {
      MLOGD("CPU(%ld): %d", i, CPU_ISSET(i, &mask));
    }
  }

  samsung_backend->created = true;

  mlperf_data_t v;
  bool isMixedModel = false;
  samsung_backend->model_path = model_path;
  EdenAI::requestMode = RequestMode::BLOCK;
  EdenAI::modePreference = BOOST_MODE;
  samsung_backend->batch = 1;
  EdenAI::gIsOfflineModel = false;
  if (configs->batch_size > 1) {
    samsung_backend->batch = configs->batch_size;
    MLOGD("mlperf_backend_create samsung_backend->batch: %d",
          samsung_backend->batch);
    EdenAI::requestMode = RequestMode::NONBLOCK;
    EdenAI::modePreference = BOOST_MODE;
    EdenAI::gIsOfflineModel = true;
  }

  libeden_nn_on_direct = nullptr;
  if (strstr(model_path, "lu") != NULL) {
    samsung_backend->isMobileBertModel_ = true;
    std::string direct_lib_path =
        std::string(native_lib_path) + std::string("/libeden_nn.so");
    libeden_nn_on_direct =
        dlopen(direct_lib_path.c_str(), RTLD_LAZY | RTLD_LOCAL);
    if (libeden_nn_on_direct != nullptr) {
      MLOGD("libeden_nn_on_direct != nullptr");
    } else {
      MLOGD("libeden_nn_on_direct == nullptr");
    }
    MLOGD("EDEN Direct NN lib path = %s; dlerror = %s\n",
          direct_lib_path.c_str(), dlerror());
  } else {
    samsung_backend->isMobileBertModel_ = false;
  }

  if (libeden_nn_on_direct != nullptr) {
    LOAD_FUNCTION(libeden_nn_on_direct, Initialize);
    LOAD_FUNCTION(libeden_nn_on_direct, OpenModel);
    LOAD_FUNCTION(libeden_nn_on_direct, OpenEdenModelFromMemory);
    LOAD_FUNCTION(libeden_nn_on_direct, AllocateInputBuffers);
    LOAD_FUNCTION(libeden_nn_on_direct, AllocateOutputBuffers);
    LOAD_FUNCTION(libeden_nn_on_direct, ExecuteModel);
    LOAD_FUNCTION(libeden_nn_on_direct, ExecuteEdenModel);
    LOAD_FUNCTION(libeden_nn_on_direct, FreeBuffers);
    LOAD_FUNCTION(libeden_nn_on_direct, CloseModel);
    LOAD_FUNCTION(libeden_nn_on_direct, Shutdown);
    GetInputBufferShapeFunc = GetInputBufferShape;
    GetOutputBufferShapeFunc = GetOutputBufferShape;
    EdenAI::setHwPreference(GPU_ONLY);
  } else if (strstr(model_path, "mobilenet_edgetpu_out.cgo") != NULL) {
    InitializeFunc = ExynosOFI_Initialize;
    OpenModelFunc = ExynosOFI_OpenModel;
    OpenEdenModelFromMemoryFunc = ExynosOFI_OpenEdenModelFromMemory;
    AllocateInputBuffersFunc = ExynosOFI_AllocateInputBuffers;
    AllocateOutputBuffersFunc = ExynosOFI_AllocateOutputBuffers;
    ExecuteModelFunc = ExynosOFI_ExecuteModel;
    ExecuteEdenModelFunc = ExynosOFI_ExecuteEdenModel;
    GetInputBufferShapeFunc = ExynosOFI_GetInputBufferShape;
    GetOutputBufferShapeFunc = ExynosOFI_GetOutputBufferShape;
    FreeBuffersFunc = ExynosOFI_FreeBuffers;
    CloseModelFunc = ExynosOFI_CloseModel;
    ShutdownFunc = ExynosOFI_Shutdown;
    EdenAI::requestMode = RequestMode::NONBLOCK;
    EdenAI::setHwPreference(DSP_ONLY);
  } else {
    InitializeFunc = Initialize;
    OpenModelFunc = OpenModel;
    OpenEdenModelFromMemoryFunc = OpenEdenModelFromMemory;
    AllocateInputBuffersFunc = AllocateInputBuffers;
    AllocateOutputBuffersFunc = AllocateOutputBuffers;
    ExecuteModelFunc = ExecuteModel;
    ExecuteEdenModelFunc = ExecuteEdenModel;
    GetInputBufferShapeFunc = GetInputBufferShape;
    GetOutputBufferShapeFunc = GetOutputBufferShape;
    FreeBuffersFunc = FreeBuffers;
    CloseModelFunc = CloseModel;
    ShutdownFunc = Shutdown;
    EdenAI::setHwPreference(NPU_ONLY);
  }

  if (EdenAI::jni_Initialize() != true) {
    MLOGE("init fail\n");
  } else {
    MLOGD("jni_Initialize() succeed\n");
  }
  if (strstr(model_path, ".nncgo")) {
    isMixedModel = true;
  }

  size_t length = 0;
  size_t length_others = 0;
  std::ifstream infile;
  if (isMixedModel && EdenAI::gIsOfflineModel) {
    /* TODO: need sophisticated header to better describe mixed models */
    struct MixedModelHeader {
      int64_t file_sizes[NUM_MIXED_MODEL_FILES];
    } header;
    infile.open(model_path);
    infile.read(reinterpret_cast<char*>(&header), sizeof(header));
    length = header.file_sizes[0];
    for (int i = 1;
         i < sizeof(header.file_sizes) / sizeof(header.file_sizes[0]); ++i) {
      length_others += header.file_sizes[i];
    }
  } else {
    // open file
    infile.open(model_path);
    infile.seekg(0, infile.end);
    length = infile.tellg();
    infile.seekg(0, infile.beg);
  }

  // read file
  if (length + length_others) {
    samsung_backend->model_buffer =
        new std::vector<char>(length + length_others);
    infile.read(samsung_backend->model_buffer->data(), length + length_others);
  } else {
    MLOGE("error reading model file: %s", model_path);
  }

  if (samsung_backend->isMobileBertModel_) {
    samsung_backend->batch = 1;
    if (EdenAI::jni_OpenModelFromMemory(samsung_backend->model_buffer->data(),
                                        length, nullptr, 0, GPU_ONLY,
                                        samsung_backend->batch) != true) {
      MLOGE("jni_OpenModelFromMemory GPU failed");
    } else {
      MLOGD("jni_OpenModelFromMemory GPU success");
    }
  } else if (EdenAI::hwPreference == DSP_ONLY) {
    if (EdenAI::jni_OpenModel(samsung_backend->model_path.c_str(), DSP_ONLY,
                              samsung_backend->batch) != true) {
      MLOGE("jni_OpenModel() failed\n");
      return nullptr;
    }
  } else {
    if (EdenAI::jni_OpenModelFromMemory(
            samsung_backend->model_buffer->data(), length,
            samsung_backend->model_buffer->data() + length, length_others,
            NPU_ONLY, samsung_backend->batch) != true) {
      MLOGE("jni_OpenModelFromMemory failed");
    } else {
      MLOGD("jni_OpenModelFromMemory success");
    }
  }

  MLOGD("requestMode = %d", EdenAI::requestMode);
  if (samsung_backend->isMobileBertModel_) {
    samsung_backend->m_inputs_.resize(384 * 3);
    samsung_backend->m_outputs_.resize(384 * 2 * 4);

    samsung_backend->out0_.resize(384);
    samsung_backend->out1_.resize(384);

    v.type = mlperf_data_t::Int32;
    v.size = 384;
    samsung_backend->input_format_.push_back(v);
    samsung_backend->input_format_.push_back(v);
    samsung_backend->input_format_.push_back(v);
    v.type = mlperf_data_t::Float32;
    v.size = 384;
    samsung_backend->output_format_.push_back(v);
    samsung_backend->output_format_.push_back(v);

    samsung_backend->output_size = 384;
    samsung_backend->input_size = 384;
  } else {
    samsung_backend->output_size = EdenAI::getOutputSize() * 4;
    samsung_backend->input_size = EdenAI::getInputSize();

    if (samsung_backend->input_size == 150528) {
      samsung_backend->model = "classification";
    } else if (samsung_backend->input_size == 270000 ||
               samsung_backend->input_size == 307200) {
      samsung_backend->model = "object_detection";
      MLOGD(
          "object_detection  samsung_backend->output_size %d  "
          "samsung_backend->input_size %d",
          samsung_backend->output_size, samsung_backend->input_size);
    } else {
      samsung_backend->model = "segmentation";
      samsung_backend->output_size = EdenAI::getOutputSize();
      MLOGD("output_size %d", samsung_backend->output_size);
    }

    if (samsung_backend->model == "object_detection") {
      samsung_backend->outputs_buffer = new std::vector<uint8_t>(
          samsung_backend->output_size * samsung_backend->batch);
      samsung_backend->input_conv =
          new std::vector<uint8_t*>(samsung_backend->batch);
      samsung_backend->detected_label_boxes =
          new std::vector<float>(samsung_backend->output_size / 7);
      samsung_backend->detected_label_indices =
          new std::vector<float>(samsung_backend->output_size / 28);
      samsung_backend->detected_label_probabilities =
          new std::vector<float>(samsung_backend->output_size / 28);
      samsung_backend->num_detections = new std::vector<float>(1);

      v.type = mlperf_data_t::Uint8;
      v.size = samsung_backend->input_size;
      samsung_backend->input_format_.push_back(v);

      v.type = mlperf_data_t::Float32;
      v.size = samsung_backend->output_size / 7;
      samsung_backend->output_format_.push_back(v);

      v.type = mlperf_data_t::Float32;
      v.size = samsung_backend->output_size / 28;
      samsung_backend->output_format_.push_back(v);

      v.type = mlperf_data_t::Float32;
      v.size = samsung_backend->output_size / 28;
      samsung_backend->output_format_.push_back(v);

      v.type = mlperf_data_t::Float32;
      v.size = 1;
      samsung_backend->output_format_.push_back(v);

    } else {
      samsung_backend->outputs_buffer = new std::vector<uint8_t>(
          samsung_backend->output_size * samsung_backend->batch);
      samsung_backend->input_conv =
          new std::vector<uint8_t*>(samsung_backend->batch);

      v.type = mlperf_data_t::Uint8;
      v.size = samsung_backend->input_size;
      samsung_backend->input_format_.push_back(v);
      if (samsung_backend->model == "classification") {
        v.type = mlperf_data_t::Float32;
        v.size = samsung_backend->output_size / 4;
        samsung_backend->output_format_.push_back(v);
      } else {
        v.type = mlperf_data_t::Uint8;
        v.size = samsung_backend->output_size;
        samsung_backend->output_format_.push_back(v);
      }
    }
  }
  MLOGD("mlperf_backend_create backend_ptr: %p", samsung_backend);
  return (mlperf_backend_ptr_t)(samsung_backend);
}

// Vendor name who create this backend.
const char* mlperf_backend_vendor_name(mlperf_backend_ptr_t backend_ptr) {
  static const char name[] = "samsung";
  return name;
}

const char* mlperf_backend_name(mlperf_backend_ptr_t backend_ptr) {
  static const char name[] = "samsung";
  return name;
}

int32_t mlperf_backend_get_input_count(mlperf_backend_ptr_t backend_ptr) {
  return ((Backend*)backend_ptr)->input_format_.size();
}

// Return the type of the ith input.
mlperf_data_t mlperf_backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                            int32_t i) {
  return ((Backend*)backend_ptr)->input_format_.at(i);
}

mlperf_status_t mlperf_backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                         int32_t batchIndex, int32_t i,
                                         void* data) {
  Backend* ptr = (Backend*)backend_ptr;

  if (ptr->isMobileBertModel_) {
    int32_t* pInputs = ptr->m_inputs_.data();
    int32_t* input_data = (int32_t*)data;
    for (int j = 0; j < 384; ++j) {
      *(pInputs + 384 * i + j) = input_data[j];
    }
  } else {
    (ptr->input_conv)->at(batchIndex) = static_cast<uint8_t*>(data);
  }
  return MLPERF_SUCCESS;
}

// Return the number of outputs from the model.
int32_t mlperf_backend_get_output_count(mlperf_backend_ptr_t backend_ptr) {
  return ((Backend*)backend_ptr)->output_format_.size();
}
// Return the type of ith output.
mlperf_data_t mlperf_backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                             int32_t i) {
  return ((Backend*)backend_ptr)->output_format_.at(i);
}

// Run the inference for a sample.
mlperf_status_t mlperf_backend_issue_query(mlperf_backend_ptr_t backend_ptr) {
  Backend* ptr = (Backend*)backend_ptr;
  if (ptr->isMobileBertModel_) {
    EdenAI::setInputSize(ptr->input_size);
    EdenAI::setOutputSize(ptr->output_size);
    if (EdenAI::jni_ExecuteModelBatch((char*)(ptr->m_inputs_.data()),
                                      (char*)(ptr->m_outputs_.data()), true,
                                      ptr)) {
      return MLPERF_SUCCESS;
    } else {
      return MLPERF_FAILURE;
    }
  } else {
    EdenAI::setInputSize(ptr->input_size * ptr->batch);
    EdenAI::setOutputSize(ptr->output_size * ptr->batch);
    if (EdenAI::jni_ExecuteModelBatch((char*)((ptr->input_conv)->at(0)),
                                      (char*)(ptr->outputs_buffer->data()),
                                      false, ptr)) {
      return MLPERF_SUCCESS;
    } else {
      return MLPERF_FAILURE;
    }
  }
}

mlperf_status_t mlperf_backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                          uint32_t batchIndex, int32_t i,
                                          void** data) {
  Backend* ptr = (Backend*)backend_ptr;
  if (ptr->isMobileBertModel_ == true) {
    int8_t* pOut = ptr->m_outputs_.data();

    if (i == 1) {
      float* pOut0 = ptr->out0_.data();
      for (int i = 0; i < 384; ++i) {
        memcpy(pOut0 + i, pOut + 384 * 0 + i * 4, sizeof(float));
      }
      *data = (void*)(pOut0);
    } else if (i == 0) {
      float* pOut1 = ptr->out1_.data();
      for (int i = 0; i < 384; ++i) {
        memcpy(pOut1 + i, pOut + 384 * 4 + i * 4, sizeof(float));
      }
      *data = (void*)(pOut1);
    }

  } else {
    if (ptr->model == "object_detection") {
      uint8_t* b = 0;
      b = ptr->outputs_buffer->data() + ptr->output_size * batchIndex;
      float num_d = 0.0;
      float* detected_label_probabilities_p =
          ptr->detected_label_probabilities->data();
      float* detected_label_boxes_p = ptr->detected_label_boxes->data();
      float* detected_label_indices_p = ptr->detected_label_indices->data();
      float* num_detections_p = ptr->num_detections->data();
      float curr_detected_idx = 0.0;
      for (int j = 0; j < 11; j++) {
        curr_detected_idx = (*((float*)(b + j * 28 + 4)));
        if (curr_detected_idx > 0) {
          switch (i) {
            case 0:
              memcpy(detected_label_boxes_p, b + j * 28 + 16, sizeof(float));
              detected_label_boxes_p++;
              memcpy(detected_label_boxes_p, b + j * 28 + 12, sizeof(float));
              detected_label_boxes_p++;
              memcpy(detected_label_boxes_p, b + j * 28 + 24, sizeof(float));
              detected_label_boxes_p++;
              memcpy(detected_label_boxes_p, b + j * 28 + 20, sizeof(float));
              detected_label_boxes_p++;
              break;
            case 1:
              curr_detected_idx = curr_detected_idx - 1;
              memcpy(detected_label_indices_p, &curr_detected_idx,
                     sizeof(float));
              detected_label_indices_p++;
              break;
            case 2:
              memcpy(detected_label_probabilities_p, b + j * 28 + 8,
                     sizeof(float));
              detected_label_probabilities_p++;
              break;
            case 3:
              num_d++;
              break;
            default:
              break;
          }
        }
      }

      switch (i) {
        case 0:
          *data = (void*)(ptr->detected_label_boxes->data());
          break;
        case 1:
          *data = (void*)(ptr->detected_label_indices->data());
          break;
        case 2:
          *data = (void*)(ptr->detected_label_probabilities->data());
          break;
        case 3:
          memcpy(num_detections_p, &num_d, sizeof(float));
          *data = (void*)(num_detections_p);
          memset(b, 0, sizeof(uint8_t) * 28 * num_d);
          break;
        default:
          break;
      }
    } else if (ptr->model == "classification" || ptr->model == "segmentation") {
      *data =
          (void*)(ptr->outputs_buffer->data() + ptr->output_size * batchIndex);
    }
  }
  return MLPERF_SUCCESS;
}

// Flush the staged queries immediately.
mlperf_status_t mlperf_backend_flush_queries(mlperf_backend_ptr_t backend_ptr) {
  return MLPERF_SUCCESS;
}

// Destroy the backend pointer and its data.
void mlperf_backend_delete(mlperf_backend_ptr_t backend_ptr) {
  mlperf_backend_clear(backend_ptr);
}

void mlperf_backend_convert_inputs(mlperf_backend_ptr_t backend_ptr, int bytes,
                                   int width, int height, uint8_t* data) {
  // MLOGE("mlperf_backend_convert_inputs called width: %d height: %d", width,
  // height);
  std::vector<uint8_t>* data_uint8_conv = new std::vector<uint8_t>(bytes);
  int blueOffset = 0;
  int greenOffset = width * height;
  int redOffset = width * height * 2;
  int idx = 0;
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      (*data_uint8_conv)[redOffset] = data[idx];
      (*data_uint8_conv)[greenOffset] = data[idx + 1];
      (*data_uint8_conv)[blueOffset] = data[idx + 2];
      redOffset++;
      greenOffset++;
      blueOffset++;
      idx = idx + 3;
    }
  }
  memcpy(data, data_uint8_conv->data(), sizeof(uint8_t) * bytes);
  data_uint8_conv->clear();
  data_uint8_conv->shrink_to_fit();
  delete data_uint8_conv;
}
