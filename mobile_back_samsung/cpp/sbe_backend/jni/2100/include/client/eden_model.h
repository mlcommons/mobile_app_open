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

#ifndef EDENMODEL_INCLUDE_EDEN_MODEL_H_
#define EDENMODEL_INCLUDE_EDEN_MODEL_H_

#include <cstdint>  // int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t
#include <map>      // map
#include <memory>   // unique_ptr
#include <string>
#include <unordered_map>  // unordered_map, unordered_multimap
#include <utility>        // move, make_pair
#include <vector>         // vector

#include "eden_model_types.h"  // EdenOperand, EdenOperation, EdenDataType, EdenBufferLayout, EdenNode
#include "eden_nn_types.h"  // INVALID_MODEL_ID
#include "eden_types.h"     // INVALID_MODEL_ID

namespace eden {
namespace nn {

typedef enum {
  MODEL_STATE_NOT_READY = 0,  //*< Model does not have enough information to
                              // represent complete model.
  MODEL_STATE_READY,  //*< Model has enough information to represent complete
                      // model.
  MODEL_STATE_REGISTERED,        //*< Model is registered to EdenRuntime.
  MODEL_STATE_FAIL_TO_REGISTER,  //*< Model is failed to register in OpenModel
} ModelState;

typedef enum { MODEL_ERROR_NONE = 0, MODEL_ERROR_MEM_FULL } ModelError;

typedef enum { PREPARE_NOT_READY = 0, PREPARE_CONFIRM } PrepareState;

typedef enum {
  FP32 = 0,
  FP16,
  UINT8,
  INT8,
} ComputePrecision;

typedef struct __CellAlignSize {
  uint32_t width;
  uint32_t height;
  uint32_t channel;
  uint32_t number;
  uint32_t reserved[4];
} CellAlignSize;

class EdenMemoryManager;

/**
 *  EdenModel class. This class implements EDEN Model.
 */
class EdenModel {
 public:
  /**
   * @brief EdenModel constructor
   * @details Initialize internal variables and resources
   * @param void
   */
  explicit EdenModel(uint32_t modelId = INVALID_MODEL_ID);

  /**
   * @brief EdenModel destructor
   * @details Release internal resourses
   * @param void
   */
  virtual ~EdenModel(void);

  /**
   * @brief Set unique model id for this model
   * @details This function sets valid model id for this model.
   *          If model already has a valid modelId(not -1),
   *          RET_MODEL_ALREADY_HAS_VALID_MODEL_ID error will be returned.
   * @param[in] modelId an unique model id
   * @returns RET_OK or RET_MODEL_ALREADY_HAS_VALID_MODEL_ID
   */
  NnRet SetModelId(uint32_t modelId);

  /**
   * @brief Add operand into a model and return a operandId representing for it
   * @details This function adds new operand specified by a operandInfo.
   *          Once the operand is added on model,
   *          a unique operand id is mapped to that operand.
   * @param[in] operandInfo Information for operand to be added.
   * @param[out] operandId An unique operand id representing for operand.
   * @returns return code
   */
  NnRet AddOperand(EdenOperand* operandInfo, int32_t* operandId);

  /**
   * @brief Add operand for options into a model and return a operandId
   * representing for it
   * @details This function adds new operand specified by a operandInfo.
   *          Once the operand is added on model,
   *          a unique operand id is mapped to that operand.
   * @param[in] operandInfo Information for operand to be added.
   * @param[out] operandId An unique operand id representing for operand.
   * @returns return code
   */
  NnRet AddOperandForOptions(EdenOperand* operandInfo, int32_t* operandId);

  /**
   * @brief Set operand value from a given buffer as offset and length
   * @details This function reads data from a bufferAddr with offset and length,
   *          then sets it to a operand of operandId.
   * @param[in] operandId Operand id to be updated.
   * @param[in] bufferAddr Buffer address of data to be loaded.
   * @param[in] offset Offset for start address of data on bufferAddr.
   * @param[in] length Length of data to be loaded.
   * @returns return code
   */
  NnRet SetOperandValue(int32_t operandId, void* bufferAddr, int32_t offset,
                        int32_t length);

  /**
   * @brief Add new operation for opType to model
   * @details This function adds new operation for opType.
   *          Caller should set correct input/output operand information as per
   * current opType.
   *          operationId representing for this operation is returned.
   * @param[in] operationInfo operation information packed as EdenOperation.
   * @param[out] operationId an unique operation id representing for opeartion.
   * @returns return code
   */
  NnRet AddOperation(EdenOperation* operationInfo, int32_t* operationId);

  /**
   * @brief Identify input and output operands for this model.
   * @details This function sets input/output operands for this model with a
   * given information.
   *          This is just marking operands for input/output.
   * @param[in] numOfInputs Number of item on inputOperandIndexes.
   * @param[in] inputOperandIndexes Array of operands for input.
   * @param[in] numOfOutputs Number of item on outputOperandIndexes.
   * @param[in] outputOperandIndexes Array of operands for output.
   * @returns return code
   */
  NnRet IdentifyInputsAndOutputs(int32_t numOfInputs,
                                 int32_t* inputOperandIndexes,
                                 int32_t numOfOutputs,
                                 int32_t* outputOperandIndexes);

  /**
   * @brief Close a model
   * @details This function releases a model related resources and destroies a
   * model.
   * @param void
   * @returns return code
   */
  NnRet CloseModel(void);

  int32_t CheckConstraint(int32_t supportedDevice, int32_t operationId);
  int32_t CheckConstraint(int32_t supportedDevice, EdenOperation* operation);

  /**
   * @brief Set EdenShapeInfo for a EdenOperand of operandId
   * @details This function creates a EdenShapeInfo with a given parameters and
   * sets it to
   *          the EdenOperand of operandId.
   * @param[in] operandId
   * @param[in] dataType
   * @param[in] bufferLayout
   * @param[in] numOfContents
   * @param[in] channel
   * @param[in] height
   * @param[in] width
   * @returns return code
   */
  NnRet SetOperandShapeInfo(int32_t operandId, EdenDataType dataType,
                            EdenBufferLayout bufferLayout,
                            int32_t numOfContents, int32_t channel,
                            int32_t height, int32_t width);

  /**
   * @brief Set EdenScaleQuantInfo for a EdenOperand of operandId
   * @details This function creates a EdenScaleQuantInfo with a given parameters
   * and sets it to
   *          the EdenOperand of operandId.
   * @param[in] operandId
   * @param[in] scale
   * @param[in] zeroPoint
   * @param[in] realValueMin
   * @param[in] realValueMax
   * @param[in] bitLength
   * @returns return code
   */
  NnRet SetOperandScaleQuantInfo(int32_t operandId, float scale,
                                 uint32_t zeroPoint, float realValueMin,
                                 float realValueMax, uint32_t bitLength);

  /**
   * @brief Set EdenFLQuantInfo for a EdenOperand of operandId
   * @details This function creates a EdenFLQuantInfo with a given parameters
   * and sets it to
   *          the EdenOperand of operandId.
   * @param[in] operandId
   * @param[in] integerBitLength
   * @param[in] fractionalBitLength
   * @param[in] bitLength
   * @returns return code
   */
  NnRet SetOperandFLQuantInfo(int32_t operandId, uint32_t integerBitLength,
                              uint32_t fractionalBitLength, uint32_t bitLength);

  // Setter functions
  void SetCompatible(const int32_t& compatible);
  void SetState(ModelState state) { state_ = state; }
  void SetPreapreState(PrepareState state) { prepare_ = state; }
  void SetError(ModelError error) { errno_ = error; }
  void SetMemoryManager(EdenMemoryManager* memoryManager) {
    memoryManager_ = memoryManager;
  }
  void SetPrivateData(std::unique_ptr<int8_t[]>& privateData) {
    privateData_ = std::move(privateData);
  }
  void SetModelPreference(ModelPreference pref) { modelPreference_ = pref; }
  void SetComputePrecision(ComputePrecision precision) {
    computePrecision_ = precision;
  }
  void SetNPUPrecisionAndType(EdenDataType dataType,
                              EdenDataTypeSize dataTypeSize) {
    edenNpuPrecision_ = dataType;
    edenNpuPrecisionSize_ = dataTypeSize;
  }

  void AddModelCompileData(std::string data) {
    modelCompileData_.push_back(data);
  }
  void NpuIsNotSupportingSoftmax(void) { npuIsNotSupportingSoftmax_ = true; }
  void ClearNpuIsNotSupportingSoftmax(void) {
    npuIsNotSupportingSoftmax_ = false;
  }
  void SetCellAlign(bool cellAlign);

  // Getter functions
  int32_t GetCompatible(void) const { return modelCompatible_; }
  uint32_t GetModelId(void) const { return modelId_; }
  int32_t GetState(void) const { return state_; }
  int32_t GetPrepareState(void) const { return prepare_; }
  int32_t GetError(void) const { return errno_; }
  ModelPreference GetModelPreference(void) const { return modelPreference_; }
  EdenMemoryManager* GetMemoryManager(void) const { return memoryManager_; }
  uint32_t& GetNumberOfRequest(void) { return numberOfRequest_; }
  ComputePrecision GetComputePrecision(void) { return computePrecision_; }
  int32_t GetTypeSizeFromDataType(EdenDataType dataType);
  void GetNPUPrecisionAndType(EdenDataType& type, EdenDataTypeSize& size) {
    type = edenNpuPrecision_;
    size = edenNpuPrecisionSize_;
  }
  std::vector<std::string>& GetModelCompileData() { return modelCompileData_; }
  uint32_t GetCellAlignSizeWidth(void) const { return cellAlignSize_.width; }
  uint32_t GetCellAlignSizeHeight(void) const { return cellAlignSize_.height; }
  uint32_t GetCellAlignSizeChannel(void) const {
    return cellAlignSize_.channel;
  }
  uint32_t GetCellAlignSizeNumber(void) const { return cellAlignSize_.number; }

  bool& ShouldRunCFU(void) { return shouldRunCFU; }

#ifdef DSP_USERDRIVER
  // Async Call of NPU UD and DSP UD, Scenario Test Cleanup by removing
  // older failing test
  bool& AsyncRunNpuDsp(void) { return asyncRunNpuDsp; }
#endif

  std::vector<EdenOperand*> const& GetOperands() const { return operands_; }
  std::vector<EdenOperand*> const& GetOperandsForOption() const {
    return operandsForOptions_;
  }
  std::vector<EdenOperation*> const& GetOperations() const {
    return operations_;
  }
  std::vector<int32_t> const& GetInputIndexes() const { return inputIndexes_; }
  std::vector<int32_t> const& GetOutputIndexes() const {
    return outputIndexes_;
  }

  std::map<int32_t, EdenOperand*> const& GetMapToOperand(void) const {
    return mapToOperand_;
  }
  std::map<int32_t, EdenOperation*> const& GetMapToOperation(void) const {
    return mapToOperation_;
  }
  std::map<EdenOperand*, int32_t> const& GetMapToOperandId(void) const {
    return mapToOperandId_;
  }
  std::map<EdenOperation*, int32_t> GetMapToOperationId(void) const {
    return mapToOperationId_;
  }

  std::map<int32_t, EdenNode*> const& GetMapToNode(void) const {
    return mapToNode_;
  }

  std::unordered_multimap<int32_t, EdenOperation*> const&
  GetMapInputOperandIdToOperation(void) const {
    return mapInputOperandIdToOperation_;
  }
  std::unordered_multimap<int32_t, EdenOperation*> const&
  GetMapOutputOperandIdToOperation(void) const {
    return mapOutputOperandIdToOperation_;
  }

  int32_t* GetSequentialOrder(int32_t** order, int32_t* order_size);
  EdenOperation* GetOneEdenOperationHavingEdenOperandAsInput(int32_t operandId);
  EdenOperation* GetOneEdenOperationHavingEdenOperandAsOutput(
      int32_t operandId);
  EdenOperation* GetNextOneEdenOperation(int32_t curOperationId);
  EdenOperation* GetPrevOneEdenOperation(int32_t curOperationId);

  std::vector<EdenOperation*> GetNextEdenOperations(int32_t curOperationId);

  int32_t GetBufferSizeForOperandId(int32_t operandId);

  bool ReadyToAllocateInputBuffers(const int32_t* edenOperandIdx);
  bool ReadyToAllocateOutputBuffers(const int32_t* edenOperandIdx);
  bool UpdateOperandDimensions(int32_t operandIndex, int32_t numOfDims,
                               int32_t* newDims);
  bool DeduceOperandDimensions(int32_t operandIndex);

  void DumpEdenOperand(EdenOperand* edenOperand);
  void DumpEdenOperation(EdenOperation* edenOperation);
  void DumpEdenModel(void);
  void DumpEdenOperandForLOGD(EdenOperand* edenOperand);
  void DumpEdenOperationForLOGD(EdenOperation* edenOperation);
  void DumpEdenModelForLOGD(void);

 private:
  void ExecuteDFS(EdenNode* curNode, int32_t* sequenceOrder, int32_t& curPos);

  /**
   * @brief Add new operation for opType to model
   * @details This function adds new operation for opType but sets a given
   * operation id
   *          Caller should set correct input/output operand information as per
   * current opType.
   * @param[in] operationInfo instance of EdenOperation having operation info
   * @param[in] operationId an unique operation id representing for opeartion.
   * @returns return code
   */
  NnRet AddOperationWithId(EdenOperation* operationInfo, int32_t operationId);

  /**
   * @brief Remove operation of operationId from model
   * @details This function removes an operation of a given operationId
   * @param[in] operationId an unique operation id to be removed from a model
   * @returns return code
   */
  NnRet RemoveOperationWithId(int32_t operationId);

  /**
   * @brief Create a graph with current model information
   * @details This function creates a graph representation with current model
   * information.
   *          The operand and operation information is used.
   *          If model information is not ready to create a complete DAG, error
   * is returned.
   * @todo Need to introduce internal state machine to reconginze model
   * information is ready.
   * @param void
   * @returns return code
   */
  NnRet CreateGraph(void);

  /**
   * @brief Destroy a graph
   * @details This function destroys a graph representation.
   * @param void
   * @returns return code
   */
  NnRet DestroyGraph(void);

  NnRet DeduceOutputShape(EdenOperation* operation);

  /**
   * @brief Dump a graph
   * @details This function prints out a graph information
   * @param void
   * @returns return code
   */
  NnRet DumpGraph(void);

  /**
   * @brief Dump a graph through LOGD
   * @details This function prints out a graph information
   * @param void
   * @returns return code
   */
  NnRet DumpGraphForLOGD(void);

  uint32_t modelId_;  //*< an unique mode id delivered on constructor
  int32_t state_;  //*< internal state for this model (e.g. ready to executed,
                   // not yet identified for input/output etc)
  int32_t errno_;
  int32_t prepare_;

  ModelPreference modelPreference_;
  ComputePrecision computePrecision_;
  EdenDataType edenNpuPrecision_;
  EdenDataTypeSize edenNpuPrecisionSize_;
  uint32_t numberOfRequest_;

  std::vector<EdenOperand*> operands_;  //*< Array of EdenOperand
  std::vector<EdenOperand*>
      operandsForOptions_;  //*< Array of EdenOperand used for XXXOptions
  std::vector<EdenOperation*> operations_;  //*< Array of EdenOperation
  std::vector<int32_t> inputIndexes_;       //*< Array of input operand id
  std::vector<int32_t> outputIndexes_;      //*< Array of output operand id

  int32_t nextOperandId_;
  int32_t nextOperationId_;

  std::map<int32_t, EdenOperand*>
      mapToOperand_;  //*< Map for (operandId, EdenOperand*)
  std::map<int32_t, EdenOperation*>
      mapToOperation_;  //*< Map for (operationId, EdenOperation*)
  std::map<EdenOperand*, int32_t>
      mapToOperandId_;  //*< Map for (EdenOperand*, operandId)
  std::map<EdenOperation*, int32_t>
      mapToOperationId_;  //*< Map for (EdenOperation*, operationId)

  std::unordered_multimap<int32_t, EdenOperation*>
      mapInputOperandIdToOperation_;  //*< Map for (operandId for input,
                                      // EdenOpeartion)
  std::unordered_multimap<int32_t, EdenOperation*>
      mapOutputOperandIdToOperation_;  //*< Map for (operandId for output,
                                       // EdenOpeartion)

  EdenNode* graphRoot;  //*!< Root for graph representation
  std::map<int32_t, EdenNode*>
      mapToNode_;  //*< Map for (operationId, EdenNode*)

  EdenMemoryManager* memoryManager_;  //*!< Instance of memory manager
  std::unique_ptr<int8_t[]> privateData_;

  int32_t modelCompatible_;
  std::vector<std::string> modelCompileData_;

  bool shouldRunCFU;
#ifdef DSP_USERDRIVER
  bool asyncRunNpuDsp;
#endif
  bool npuIsNotSupportingSoftmax_;
  CellAlignSize cellAlignSize_;
};

}  // namespace nn
}  // namespace eden

#endif  // EDENMODEL_INCLUDE_EDEN_MODEL_H_
