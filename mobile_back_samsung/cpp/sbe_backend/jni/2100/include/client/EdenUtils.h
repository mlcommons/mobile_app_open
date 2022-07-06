/* Copyright 2018 The MLPerf Authors. All Rights Reserved.
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
#include <unistd.h>

#include <cstdio>   // fopen, perror
#include <cstdlib>  // exit
#include <cstring>  // strlen
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <queue>
#include <random>
#include <sstream>
#include <utility>

#include "eden_nn_api.h"
#include "eden_types.h"

#define JNI_LOGD(...) \
  __android_log_print(ANDROID_LOG_DEBUG, JNI_LOG_TAG, __VA_ARGS__)
#define JNI_LOGE(...) \
  __android_log_print(ANDROID_LOG_ERROR, JNI_LOG_TAG, __VA_ARGS__)

#define LOG_TAG "EdenAI_JNI"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

//#define DEBUG_DUMP
#define NUM_OF_BUFF_MAX 16

namespace EdenUtils {
static const char* ExynosEDEN = "[Exynos][EDEN] ";

template <typename T>
void printTopN(char* buffer, int32_t len, int32_t topN, int32_t iterCnt) {
  // Check output result
  std::priority_queue<std::pair<T, int32_t>> pqueue;

  T* pProb = reinterpret_cast<T*>(buffer);
  for (uint32_t idx = 0; idx < (len / sizeof(T)); idx++) {
    T prob = pProb[idx];
    pqueue.push(std::make_pair(prob, idx));
  }

  // Print top-n
  std::cout << ExynosEDEN << "====== Top-" << topN << " ======"
            << " Iteration Count:" << iterCnt + 1 << std::endl;
  for (int32_t idx = 0; idx < topN; idx++) {
    auto topData = pqueue.top();
    std::cout << ExynosEDEN << "== Top-" << (idx + 1) << " :: ";
    std::cout << "probability:[" << topData.first << "], id:[" << topData.second
              << "]" << std::endl;
    pqueue.pop();
  }
  std::cout << ExynosEDEN << "===================" << std::endl;
}

void printDetection(void* outputBufferAddr, int32_t outputBufferSize,
                    int32_t iterCnt) {
  float* outputRawData = reinterpret_cast<float*>(outputBufferAddr);
  // Print on LOG
  std::cout << ExynosEDEN
            << "================= Detection Output ================="
            << " Iteration Count:" << (iterCnt + 1) << std::endl;
  std::cout << ExynosEDEN
            << "Index\tLabel\t\tScore\t\tX-start\t\tY-start\t\tX-end\t\tY-end"
            << std::endl;
  int count = outputBufferSize / sizeof(float);
  std::string buffstr;
  int outCnt = 0;
  for (int i = 0; i < count; i++) {
    if (outCnt < 3) {
      buffstr += std::to_string(outputRawData[i]);
    } else {
      buffstr += std::to_string(outputRawData[i] * 300);
    }
    buffstr += "\t";
    outCnt++;
    if (outCnt % 7 == 0) {
      std::cout << ExynosEDEN << buffstr.c_str() << std::endl;
      buffstr = "";
      outCnt = 0;
    }
  }
  if (buffstr != "") {
    std::cout << ExynosEDEN << buffstr.c_str() << std::endl;
  }
  std::cout << ExynosEDEN
            << "================================================="
               "==================================="
            << std::endl;
}

bool loadFileOnMemory(const char* pathToModel, int8_t** addr,
                      int32_t* sizeInByte) {
  // File open
  FILE* modelFp = std::fopen(pathToModel, "rb");
  if (modelFp == nullptr) {
    LOGE("Fail to open a given file!\n");
    return false;
  }

  // Calculate a file size and load file into buffer
  std::fseek(modelFp, 0, SEEK_END);  // seek to end
  int32_t filesize = std::ftell(modelFp);
  std::fseek(modelFp, 0, SEEK_SET);  // seek to start
  LOGD("open a given file [%d]!\n", filesize);
  if (filesize < 0) {
    LOGE("Fail to fread from a given file!\n");
    std::fclose(modelFp);
    return false;
  }

  // Load data on buffer
  int8_t* buffer = new int8_t[filesize];
  size_t numOfBytes = std::fread(buffer, sizeof(int8_t), filesize, modelFp);
  std::fclose(modelFp);
  if (numOfBytes < filesize) {
    LOGE("Fail to fread from a given file!\n");
    delete[] buffer;
    return false;
  }

  LOGD("buffer=[%p], filesize=[%d]\n", buffer, filesize);
  *addr = buffer;
  *sizeInByte = filesize;

  return true;
}

EdenModelFile makeModel(const char* pathToModelFile) {
  EdenModelFile modelFile;
  modelFile.modelFileFormat = TFLITE;
  modelFile.pathToModelFile = (int8_t*)pathToModelFile;
  modelFile.lengthOfPath = strlen(pathToModelFile) + 1;  // including NULL
  modelFile.pathToWeightBiasFile = nullptr;
  modelFile.lengthOfWeightBias = 0;  // include NULL
  return modelFile;
}
}  // namespace EdenUtils
