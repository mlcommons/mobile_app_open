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

#ifndef EDENMODEL_INCLUDE_EDEN_MEMORY_MANAGER_H_
#define EDENMODEL_INCLUDE_EDEN_MEMORY_MANAGER_H_

#include <cstdint>  // int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t
#include <map>      // map
#include <vector>   // vector

#include "Clang_ThreadSafety.h"
#include "ThreadSafeMap.h"
#include "eden_memory.h"       // eden_memory_t
#include "eden_model_types.h"  // EdenDataType
#include "eden_nn_types.h"     // NnRet
#include "eden_types.h"  // EdenBuffer, EdenMemType, ModelPreference, NnApiType

namespace eden {
namespace nn {

typedef struct __EdenMemoryInfo {
  int32_t numOfBuffers;
  EdenBuffer* buffers;
  int32_t* sizeOfBuffers;
  int32_t* operandIds;
  int32_t* allocSizeOfBuffers;
  eden_memory_t* emaBuffers;
} EdenMemoryInfo;

class EdenModel;

/**
 *  EdenMemoryManager class. This class implements EDEN Memory Manager.
 */
class EdenMemoryManager {
 public:
  /**
   * @brief EdenMemoryManager constructor
   * @details Initialize internal variables and resources
   * @param void
   */
  EdenMemoryManager(void);

  /**
   * @brief EdenMemoryManager destructor
   * @details Release internal resourses
   * @param void
   */
  virtual ~EdenMemoryManager(void);

  /**
   * @brief Register new model
   * @details This function allocates resources for a given model instance.
   * @param[in] modelId EdenModel instance id.
   * @returns return code
   */
  NnRet RegisterModel(EdenModel* model, ModelPreference preference);

  /**
   * @brief Allocate buffers for input operands
   * @details This function allocates memory for input operands.
   *          Allocated memory can be different based on a given memType.
   *          Allocated memory is returned via an array of EdenBuffer refering
   * to
   *          input operands on model.
   * @param[in] modelId EdenModel instance id.
   * @param[in] memType Memory type for input operands.
   * @param[out] buffers Array of EdenBuffer*.
   * @param[out] numOfBuffers Number of item on buffers.
   * @returns return code
   */
  NnRet AllocateBuffersForInputOperands(uint32_t modelId, EdenMemType memType,
                                        EdenBuffer** buffers,
                                        int32_t* numOfBuffers);

  /**
   * @brief Allocate buffers for output operands
   * @details This function allocates memory for output operands.
   *          Allocated memory can be different based on a given memType.
   *          Allocated memory is returned via an array of EdenBuffer refering
   * to
   *          output operands on model.
   * @param[in] modelId EdenModel instance id.
   * @param[in] memType Memory type for output operands.
   * @param[out] buffers Array of EdenBuffer*.
   * @param[out] numOfBuffers Number of item on buffers.
   * @returns return code
   */
  NnRet AllocateBuffersForOutputOperands(uint32_t modelId, EdenMemType memType,
                                         EdenBuffer** buffers,
                                         int32_t* numOfBuffers);

  /**
   * @brief Allocate buffers for input operands
   * @details This function allocates memory for input operands.
   *          Allocated memory can be different based on a given memType.
   *          Allocated memory is returned via an array of EdenBuffer refering
   * to
   *          input operands on model.
   * @todo Proper error handling should be added in future to avoid memory leak
   * @param[in] modelId EdenModel instance id.
   * @param[in] memType Memory type for input operands.
   * @param[in] buffers Array of UserBuffer*.
   * @param[in] numOfBuffers Number of item on buffers.
   * @param[out] buffers Array of EdenBuffers for input
   * @returns return code
   */
  NnRet LoadBuffersForInputOperands(uint32_t modelId, EdenMemType memType,
                                    UserBuffer* userBuffers,
                                    int32_t numOfBuffers,
                                    EdenBuffer** edenBuffers);

  /**
   * @brief Allocate buffers for output operands
   * @details This function allocates memory for output operands.
   *          Allocated memory can be different based on a given memType.
   *          Allocated memory is returned via an array of EdenBuffer refering
   * to
   *          output operands on model.
   * @todo Proper error handling should be added in future to avoid memory leak
   * @param[in] modelId EdenModel instance id.
   * @param[in] memType Memory type for output operands.
   * @param[in] buffers Array of UserBuffer*.
   * @param[in] numOfBuffers Number of item on buffers.
   * @param[out] buffers Array of EdenBuffers for output
   * @returns return code
   */
  NnRet LoadBuffersForOutputOperands(uint32_t modelId, EdenMemType memType,
                                     UserBuffer* userBuffers,
                                     int32_t numOfBuffers,
                                     EdenBuffer** edenBuffers);

  /**
   * @brief Allocate buffers for operands by referring to given operandIds
   * @details This function allocates memory for operands matched to a given
   * operandIds.
   *          Allocated memory can be different based on a given memType.
   *          Allocated memory is returned via an array of EdenBuffer refering
   * to operandIds.
   * @param[in] modelId EdenModel instance id.
   * @param[in] memType Memory type for output operands.
   * @param[in] numOfOperand Number of operand id on operandIds.
   * @param[in] operandIds Array of operand id.
   * @param[out] buffers Array of EdenBuffer*.
   * @returns return code
   */
  NnRet AllocateBuffersForOperands(uint32_t modelId, EdenMemType memType,
                                   int32_t numOfOperand, int32_t* operandIds,
                                   EdenBuffer** buffers);

  /**
   * @brief Allocate buffers for an operand by referring to given operandId
   * @details This function allocates memory for an operand matched to a given
   * operandId.
   *          Allocated memory can be different based on a given memType.
   *          Allocated memory is returned via an array of EdenBuffer refering
   * to an operandId.
   * @param[in] modelId EdenModel instance id.
   * @param[in] memType Memory type for output operands.
   * @param[in] operandId Operand id on model.
   * @param[out] buffers Array of EdenBuffer*.
   * @returns return code
   */
  NnRet AllocateBuffersForOperand(uint32_t modelId, EdenMemType memType,
                                  int32_t operandId, EdenBuffer** buffers);

  NnRet AllocateBuffersForBridgeOperand(uint32_t modelId, addr_t refer,
                                        EdenMemType memType, int32_t operandId,
                                        EdenBuffer** buffers);
  NnRet GetBuffersForBridgeOperand(uint32_t modelId, addr_t refer,
                                   int32_t operandId, EdenBuffer** buffers,
                                   bool check = false);
#ifdef DSP_USERDRIVER
  // Allocate Bridge buffers for custom operations
  NnRet IsAlreadyAllocatedBridgeOperand(uint32_t modelId, addr_t refer,
                                        int32_t operandId);
#endif

  /**
   * @brief Reallocate buffers for a given oldBuffers
   * @details This function finds a matched EdenBuffers allocated before,
   *          and releases previously allocated one.
   *          Then it allocates new EdenBuffers with a given parameters,
   *          and replace it to old one.
   * @param[in] modelId EdenModel instance id.
   * @param[in] oldBuffers Array of EdenBuffer*.
   * @param[in] numOfBuffers Number of items on buffers.
   * @param[in] sizeOfBuffers Array of EdenBuffer*.
   * @param[out] newBuffers Array of EdenBuffer* allocated.
   * @returns return code
   */
  NnRet ReallocateBuffersForOperands(uint32_t modelId, addr_t requestId,
                                     EdenBuffer* oldBuffers,
                                     int32_t newNumOfBuffers,
                                     int32_t* newSizeOfBuffers);

  /**
   * @brief Get rellocated buffer information for a given modelId
   * @details This function gets reallocated buffer information for a given
   * modelId,
   *          which is called via ReallocateBuffersForOperands().
   * @param[in] modelId EdenModel instance id
   * @param[out] bufferInfo Map between operandId to EdenBuffer*
   * @returns return code
   */
  NnRet GetReallocatedBufferInfo(uint32_t modelId, addr_t requestId,
                                 std::map<int32_t, EdenBuffer*>& bufferInfo);

  /**
   * @brief Clear rellocated buffer information for a given modelId
   * @details This function clear reallocated buffer information for a given
   * modelId,
   *          which is called via ReallocateBuffersForOperands().
   * @param[in] modelId EdenModel instance id
   * @returns return code
   */
  NnRet ClearReallocatedBufferInfo(uint32_t modelId, addr_t requestId);

  /**
   * @brief Free resources allocated for buffers of a given modelId
   * @details This function releases a resources allocated for buffers of a
   * given modelId.
   *          It first searches it on input resources, then output resources.
   * @param[in] modelId EdenModel instance id
   * @param[in] buffers Array of EdenBuffer*.
   * @returns return code
   */
  NnRet FreeBuffersForOperands(uint32_t modelId, EdenBuffer* buffers);

  /**
   * @brief Free temporal resources allocated for operandIds of a given modelId
   * @details This function releases a temporal resources allocated for
   * operandIds of a given modelId.
   *          It only searches on temporal resources.
   * @param[in] modelId EdenModel instance id
   * @param[in] numOfOperand Number of operand id on operandIds.
   * @param[in] operandIds Array of operand id.
   * @returns return code
   */
  NnRet FreeTempBuffersForOperands(uint32_t modelId, int32_t numOfOperand,
                                   int32_t* operandIds);

  /**
   * @brief Free temporal resources allocated for an operandId of a given
   * modelId
   * @details This function releases a temporal resources allocated for an
   * operandId of a given modelId.
   *          It only searches on temporal resources.
   * @param[in] modelId EdenModel instance id
   * @param[in] operandId Operand id on model.
   * @returns return code
   */
  NnRet FreeTempBuffersForOperand(uint32_t modelId, int32_t operandId);

  /**
   * @brief Unregister new model id
   * @details This function releases resources for a given model id.
   * @param[in] modelId EdenModel instance id.
   * @returns return code
   */
  NnRet UnregisterModel(uint32_t modelId);

  /**
   * @brief Return EMABuffer pointer match to a given EdenBuffer
   * @details This function returns a pointer of EMABuffer which is pair of a
   * given EdenBuffer.
   * @param[in] modelId EdenModel instance id.
   * @param[in] buffers pointer of EdenBuffer
   * @param[out] emaBuffers pointer of eden_memory_t
   * @returns pointer of eden_memory_t pair of a given EdenBuffer*
   */
  NnRet GetMatchedEMABuffers(uint32_t modelId, EdenBuffer* buffers,
                             eden_memory_t** emaBuffers);

  /**
   * @brief Find a matched EMABuffer in terms of operandId and update a given
   * emaBuffers
   * @details This function updates a given emaBuffers's values matched to a
   * given operandId.
   *          Therefore, caller should make sure emaBuffers is not null.
   * @param[in] modelId EdenModel instance id.
   * @param[in] operandId Operand id on model.
   * @param[out] emaBuffers pointer of eden_memory_t
   * @returns return code
   */
  NnRet GetMatchedEMABuffers(uint32_t modelId, int32_t operandId,
                             eden_memory_t* emaBuffers);

  /**
   * @brief Find a matched EMABuffer in terms of addr/size and update a given
   * emaBuffers
   * @details This function updates a given emaBuffers's values matched to a
   * given addr/size.
   *          Therefore, caller should make sure emaBuffers is not null.
   * @param[in] modelId EdenModel instance id.
   * @param[in] addr Virtual address of buffer, returned by EMM.
   * @param[in] size size of buffer, returned by EMM.
   * @param[out] emaBuffers pointer of eden_memory_t
   * @returns return code
   */
  NnRet GetMatchedEMABuffers(uint32_t modelId, void* addr, int32_t size,
                             eden_memory_t* emaBuffers);

  /**
   * @brief Calculate required buffer size to store input based on Z-order
   * @details This function calculates required size for storing contents on
   * buffer based on Z-order.
   * @param[in] width buffer width
   * @param[in] height buffer height
   * @param[in] channel number of channel
   * @param[in] numOfBuffers number of buffers
   * @param[in] width for align
   * @param[in] height for align
   * @param[in] channel for align
   * @returns Size for Z-order data
   */
  int32_t SizeForZOrder(int32_t width, int32_t height, int32_t channel,
                        int32_t numOfBuffers, uint32_t alignWidth,
                        uint32_t alignHeight, uint32_t alignChannel);

  /**
   * @brief Dump internal memory info mapping
   * @details This function shows internal memory info mappings for a given
   * modelId.
   * @param[in] modelId Model ID for EdenModel.
   * @returns void
   */
  void DumpInternalMemoryInfos(uint32_t modelId);

 private:
  /**
   * @brief Create EdenMemoryInfo for input operand
   * @details This function creates EdenMemoryInfo reflecting input operands.
   * @param[in] modelId EdenModel instance id.
   * @param[out] memInfo Created instance of EdenMemoryInfo
   * @returns return code
   */
  NnRet CreateMemoryInfoForInput(uint32_t modelId, EdenMemoryInfo** memInfo);

  /**
   * @brief Create EdenMemoryInfo for output operand
   * @details This function creates EdenMemoryInfo reflecting output operands.
   * @param[in] modelId EdenModel instance id.
   * @param[out] memInfo Created instance of EdenMemoryInfo
   * @returns return code
   */
  NnRet CreateMemoryInfoForOutput(uint32_t modelId, EdenMemoryInfo** memInfo);

  /**
   * @brief Create a new EdenMemoryInfo with given parameters.
   * @details This function creates a EdenMemoryInfo with a given parameters.
   * @param[in] numOfBuffers EdenMemoryInfo to be released
   * @param[in] sizeOfBuffers EdenMemoryInfo to be released
   * @param[out] memInfo EdenMemoryInfo to be released
   * @returns return code
   */
  NnRet CreateMemoryInfo(int32_t numOfBuffers, int32_t* sizeOfBuffers,
                         int32_t* operandIds, EdenMemoryInfo** memInfo);

  /**
   * @brief Allocate buffers using given information
   * @details This function allocates memory using given informaion
   *          Allocated memory can be different based on a given memType.
   *          Allocated memory is returned via an array of
   * EdenBuffer/eden_memory_t
   * @todo Proper error handling should be added in future to avoid memory leak
   * @param[in] numOfBuffers number of buffers to be allocated
   * @param[in] sizeOfBuffers array for each buffer size
   * @param[in] memType Memory type for input operands.
   * @param[out] buffers Array of EdenBuffer*.
   * @param[out] emaBuffers Array of eden_memory_t*
   * @returns return code
   */
  NnRet AllocateBuffers(int32_t numOfBuffers, int32_t* sizeOfBuffers,
                        EdenMemType memType, EdenBuffer** buffers,
                        eden_memory_t** emaBuffers);

  /**
   * @brief Load buffers using given information
   * @details This function connects the user's ion buffer to the
   * edenMemoryManager.
   * @param[in] numOfBuffers number of buffers to be allocated
   * @param[in] sizeOfBuffers array for each buffer size
   * @param[in] memType Memory type for input operands.
   * @param[in] buffers Array of UserBuffer*.
   * @param[out] buffers Array of EdenBuffer*.
   * @param[out] emaBuffers Array of eden_memory_t*
   * @returns return code
   */
  NnRet LoadBuffers(int32_t numOfBuffers, int32_t* sizeOfBuffers,
                    EdenMemType memType, UserBuffer* userBuffers,
                    EdenBuffer** edenBuffers, eden_memory_t** emaBuffers);

  /**
   * @brief Release EdenMemoryInfo matched to a given buffers
   * @details This function releases a EdenMemoryInfo from a matched resource.
   * @param[in] vecMemInfo vector for EdenMemoryInfo of a specific model
   * @param[in] buffers Array of EdenBuffer*.
   * @returns return code
   */
  NnRet FindAndReleaseMemoryInfo(std::vector<EdenMemoryInfo*>& vecMemInfo,
                                 EdenBuffer* buffers);

  /**
   * @brief Release EdenMemoryInfo matched to a given operandIds
   * @details This function releases a EdenMemoryInfo from a matched resource.
   * @param[in] vecMemInfo vector for EdenMemoryInfo of a specific model
   * @param[in] numOfOperand Number of operand id on operandIds.
   * @param[in] operandIds Array of operand id.
   * @returns return code
   */
  NnRet FindAndReleaseMemoryInfo(std::vector<EdenMemoryInfo*>& vecMemInfo,
                                 int32_t numOfOperand, int32_t* operandIds);

  /**
   * @brief Release a given EdenMemoryInfo
   * @details This function releases a resources allocated for a given
   * EdenMemoryInfo
   *          It does not release memInfo itself.
   * @param[in] memInfo EdenMemoryInfo to be released
   * @returns return code
   */
  NnRet ReleaseMemoryInfo(EdenMemoryInfo* memInfo);

  NnRet GetSizeOfBufferForOperandId(uint32_t modelId, int32_t operandId,
                                    int32_t& sizeOfBuffer);

  /**
   * @brief Return EdenModel* matched to a given modelId
   * @details This function returns a EdenModel* matched to a given modelId.
   * @param[in] modelId Model ID for EdenModel.
   * @param[out] model Matched EdenModel*
   * @returns return code
   */
  NnRet GetEdenModel(uint32_t modelId, EdenModel*& model);

  /**
   * @brief Return NnApiType matched to a given modelId
   * @details This function returns a NnApiType matched to a given modelId.
   * @param[in] modelId Model ID for NnApiType.
   * @param[out] nnApiType Matched NnApiType.
   * @returns return code
   */
  NnRet GetNnApiType(uint32_t modelId, NnApiType& nnApiType);

  /**
   * @brief Check if modelId is registered on EMM
   * @details This function checks modelId if it is registered.
   * @param[in] modelId Model ID for EdenModel.
   * @returns true(Registered), false(Not registered)
   */
  bool IsRegisteredModel(uint32_t modelId);

  Mutex mutex_mapModelIdToInputMemoryInfo_;
  Mutex mutex_mapModelIdToOutputMemoryInfo_;
  Mutex mutex_mapRequestIdToReallocatedBufferInfo_;
  Mutex mutex_mapModelIdToRellocatedRequest_;
  Mutex mutex_mapBufferAddrToBridgeBufferInfo_;
  Mutex mutex_mapModelIdToBridgeBufferAddr_;
  Mutex mutex_mapModelIdToTempMemoryInfo_;
  Mutex mutex_mapModelIdToModelPreference_;

  eden::util::map<uint32_t, EdenModel*>
      mapModelIdToModel_;  //*< Map for (modelId, EdenModel*)

  // For Input/Output Buffer
  std::map<uint32_t, std::vector<EdenMemoryInfo*>> mapModelIdToInputMemoryInfo_
      GUARDED_BY(mutex_mapModelIdToInputMemoryInfo_);  //*< Map for (modelId,
                                                       // input buffers)
  std::map<uint32_t, std::vector<EdenMemoryInfo*>> mapModelIdToOutputMemoryInfo_
      GUARDED_BY(mutex_mapModelIdToOutputMemoryInfo_);  //*< Map for (modelId,
                                                        // output buffers)

  // For Reallocate Buffer
  std::map<addr_t, std::map<int32_t, EdenBuffer*>>
      mapRequestIdToReallocatedBufferInfo_ GUARDED_BY(
          mutex_mapRequestIdToReallocatedBufferInfo_);  //*< Map for (requestId,
                                                        // map<operandId,
                                                        // EdenBuffer*>)
  std::map<uint32_t, std::vector<addr_t>> mapModelIdToRellocatedRequest_
      GUARDED_BY(mutex_mapModelIdToRellocatedRequest_);  //*< Map for (modelId,
                                                         // vector<requestId>)

  // For Bridge(Temp) Buffer
  std::map<addr_t, std::map<int32_t, EdenMemoryInfo*>>
      mapBufferAddrToBridgeBufferInfo_ GUARDED_BY(
          mutex_mapBufferAddrToBridgeBufferInfo_);  //*< Map for (inputBuffer
                                                    // addr, map<operandId,
                                                    // EdenMemoryInfo*>)
  std::map<uint32_t, std::vector<addr_t>> mapModelIdToBridgeBufferAddr_
      GUARDED_BY(mutex_mapModelIdToBridgeBufferAddr_);  //*< Map for (modelId,
                                                        // vector<inputBuffer
                                                        // addr>)
  std::map<uint32_t, std::vector<EdenMemoryInfo*>> mapModelIdToTempMemoryInfo_
      GUARDED_BY(mutex_mapModelIdToTempMemoryInfo_);  //*< Map for (modelId,
                                                      // vector<EdenMemoryInfo>)
  std::map<uint32_t, ModelPreference> mapModelIdToModelPreference_
      GUARDED_BY(mutex_mapModelIdToModelPreference_);  //*< Map for (modelId,
                                                       // ModelPreference)
};

}  // namespace nn
}  // namespace eden

#endif  // EDENMODEL_INCLUDE_EDEN_MEMORY_MANAGER_H_
