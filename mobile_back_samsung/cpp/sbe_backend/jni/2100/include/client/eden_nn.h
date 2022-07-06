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

#ifndef NN_EDEN_NN_H_
#define NN_EDEN_NN_H_

#include <cstdint>  // int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t
#include <unordered_set>  // unordered_set

// nn
#include "eden_nn_types.h"  // NnRet
// common
#include "eden_types.h"  // EdenModelFile, EdenPreference, EdenBuffer, EdenRequest
// osal
#include "osal_types.h"  // addr_t

namespace eden {
namespace nn {

/**
 *  EdenNN class. This class implements EDEN NN API.
 */
class EdenNN {
 public:
  /**
   * @brief EdenNN constructor
   * @details Initialize internal variables and resources
   * @param void
   */
  EdenNN(void) : nextRequestId_(0) {}

  /**
   * @brief EdenNN destructor
   * @details Release internal resourses
   * @param void
   */
  virtual ~EdenNN(void) {}

  /**
   * @brief Initialize EDEN NN.
   * @details This function initializes the EDEN NN framework.
   *          Internal data structure and related resource preparation is taken
   * place.
   * @param void
   * @returns return code
   */
  NnRet Initialize(void);

  /**
   * @brief Initialize EDEN NN with specific target device
   * @details This function initializes the EDEN NN framework.
   *          Internal data structure and related resource preparation is taken
   * place.
   * @param [in] target It represents for a specific target device.
   * @returns return code
   */
  NnRet Initialize(uint32_t target);

  /**
   * @brief [Deprecated] Open a model file and generates an in-memory model
   * structure
   * @details Deprecated function.
   */
  NnRet OpenModel(EdenModelFile* modelFile, uint32_t* modelId,
                  EdenPreference preference);

  /**
   * @brief Open a model file and generates an in-memory model structure
   * @details This function reads a model file and construct an in-memory model
   * structure.
   *          The model file should be one of the supported model file format.
   *          Once EDEN NN successes to parse a given model file,
   *          unique model id is returned via modelId.
   * @param[in] modelFile It is representing for EDEN model file such as file
   * path.
   * @param[out] modelId It is representing for constructed EdenModel with a
   * unique id.
   * @param[in] options It is representing for a model options.
   * @returns return code
   */
  NnRet OpenModel(EdenModelFile* modelFile, uint32_t* modelId,
                  EdenModelOptions& options);

  /**
   *  @brief Read a in-memory model on address and open it as a EdenModel
   *  @details Deprecated function.
   */
  NnRet OpenModelFromMemory(ModelTypeInMemory modelTypeInMemory, int8_t* addr,
                            int32_t size, bool encrypted, uint32_t* modelId,
                            EdenPreference preference);

  /**
   *  @brief Read a in-memory model on address and open it as a EdenModel
   *  @details This function reads a in-memory model on a given address and
   * convert it to EdenModel.
   *           The in-memory model should be one of the supported model type in
   * memory.
   *           Once it successes to parse a given in-memory model,
   *           unique model id is returned via modelId.
   *  @param[in] modelTypeInMemory it is representing for in-memory model such
   * as Android NN Model.
   *  @param[in] addr address of in-memory model
   *  @param[in] size size of in-memory model
   *  @param[in] encrypted data on addr is encrypted
   *  @param[out] modelId It is representing for constructed EdenModel with a
   * unique id.
   *  @param[in] options It is representing for a model options.
   *  @returns return code
   */
  NnRet OpenModelFromMemory(ModelTypeInMemory modelTypeInMemory, int8_t* addr,
                            int32_t size, bool encrypted, uint32_t* modelId,
                            EdenModelOptions& options);

  /**
   * @brief Allocate a buffer to execute a model
   * @details This function allocates an efficient buffer to execute a buffer.
   * @param[in] modelId The model id to be applied by.
   * @param[out] buffers Array of EdenBuffers for input
   * @param[out] numOfBuffers # of buffers
   * @returns return code
   */
  NnRet AllocateInputBuffers(uint32_t modelId, EdenBuffer** buffers,
                             int32_t* numOfBuffers);

  /**
   * @brief Allocate a buffer to execute a model
   * @details This function allocates an efficient buffer to execute a buffer.
   * @param[in] modelId The model id to be applied by.
   * @param[out] buffers Array of EdenBuffers for output
   * @param[out] numOfBuffers # of buffers
   * @returns return code
   */
  NnRet AllocateOutputBuffers(uint32_t modelId, EdenBuffer** buffers,
                              int32_t* numOfBuffers);

  /**
   * @brief Load the buffers of user to execute a model
   * @details This function loads the user buffers for execution
   * @param[in] modelId The model id to be applied by
   * @param[in] Array of Userbuffers for input
   * @param[in] numOfBuffers # of buffers.
   * @param[out] buffers Array of EdenBuffers for input
   * @returns return code
   */
  NnRet LoadInputBuffers(uint32_t modelId, UserBuffer* userBuffers,
                         int32_t numOfBuffers, EdenBuffer** edenBuffers);

  /**
   * @brief Load the buffers of user to execute a model
   * @details This function loads to a user buffer for execution
   * @param[in] modelId The model id to be applied by
   * @param[in] Array of Userbuffers for output
   * @param[in] numOfBuffers # of buffers.
   * @param[out] buffers Array of EdenBuffers for output
   * @returns return code
   */
  NnRet LoadOutputBuffers(uint32_t modelId, UserBuffer* userBuffers,
                          int32_t numOfBuffers, EdenBuffer** edenBuffers);

  /**
   * @brief [Deprecated] Execute a model with given buffers in nonblocking mode.
   * @details Deprecated function.
   */
  NnRet ExecuteModel(EdenRequest* request, addr_t* requestId,
                     EdenPreference preference);

  /**
   * @brief Execute a model with given buffers in nonblocking mode.
   * @details This function executes a model with input/output buffers.
   *          Internally EDEN NN creates a request to execute a model with
   * buffers,
   *          and this request is numbered with an unique id.
   *          This unique id is returned to caller via requestId.
   *          When the execution is complete, the callback's notify is executed
   * by EDEN NN.
   * @param[in] request It is representing for eden model, input/output buffers
   * and callback.
   * @param[out] requestId Unique id representing for this request.
   * @param[in] options It is representing for a request options.
   * @returns return code
   */
  NnRet ExecuteModel(EdenRequest* request, addr_t* requestId,
                     const EdenRequestOptions& options);

  /**
   * @brief Release a buffer allocated by Eden framework
   * @details This function releases a buffer returned by AllocateXXXBuffers.
   * @param[in] modelId The model id to be applied by.
   * @param[in] buffers Buffer pointer allocated by AllocateXXXBuffers
   * @returns return code
   */
  NnRet FreeBuffers(uint32_t modelId, EdenBuffer* buffers);

  /**
   * @brief Close a model
   * @details This function releases a model related resources and destroies a
   * model.
   * @param[in] modelId The model id to be applied by.
   * @returns return code
   */
  NnRet CloseModel(uint32_t modelId);

  /**
   * @brief Shutdown NN framework
   * @details This function destorys the Eden NN framework.
   *          Eden NN lets Eden Runtime know there is no more NN activity.
   * @param void
   * @returns return code
   */
  NnRet Shutdown(void);

  /**
   *  @brief Get the input buffer information
   *  @details This function gets buffer shape for input buffer of a specified
   * model.
   *  @param[in] modelId The model id to be applied by.
   *  @param[in] inputIndex Input index starting 0.
   *  @param[out] width Width
   *  @param[out] height Height
   *  @param[out] channel Channel
   *  @param[out] number Number
   *  @returns return code
   */
  NnRet GetInputBufferShape(uint32_t modelId, int32_t inputIndex,
                            int32_t* width, int32_t* height, int32_t* channel,
                            int32_t* number);

  /**
   *  @brief Get the output buffer information
   *  @details This function gets buffer shape for output buffers of a specified
   * model.
   *  @param[in] modelId The model id to be applied by.
   *  @param[in] outputIndex Output index starting 0.
   *  @param[out] width Width
   *  @param[out] height Height
   *  @param[out] channel Channel
   *  @param[out] number Number
   *  @returns return code
   */
  NnRet GetOutputBufferShape(uint32_t modelId, int32_t outputIndex,
                             int32_t* width, int32_t* height, int32_t* channel,
                             int32_t* number);

  /**
   *  @brief Get Eden version
   *  @details This function gets EdenVersion with a current EDEN framework
   * version.
   *           It includes hardware and software version too.
   *  @param version It is representing for EDEN version information.
   *         This function is returned with version filled with a current
   * information.
   *  @returns return code
   */
  NnRet GetEdenVersion(uint32_t modelId, int32_t* versions);

  /**
   *  @brief Get model compile version
   *  @details This function gets compile version
   *  @param[in] modelId The model id to be applied by.
   *  @param[in] modelFile It is representing for EDEN model file such as file
   * path.
   *  @param[out] version It is representing for compile version information.
   *         This function is returned with version filled with a current
   * information.
   *  @returns return code
   */
  NnRet GetCompileVersion(uint32_t modelId, EdenModelFile* modelFile,
                          char versions[][VERSION_LENGTH_MAX]);

  /**
   *  @brief Get model compile version
   *  @details This function gets compile version
   *  @param[in] modelTypeInMemory it is representing for in-memory model such
   * as Android NN Model.
   *  @param[in] addr address of in-memory model
   *  @param[in] size size of in-memory model
   *  @param[in] encrypted data on addr is encrypted
   *  @param[out] version It is representing for compile version information.
   *         This function is returned with version filled with a current
   * information.
   *  @returns return code
   */
  NnRet GetCompileVersionFromMemory(ModelTypeInMemory typeInMemory,
                                    int8_t* addr, int32_t size, bool encrypted,
                                    char versions[][VERSION_LENGTH_MAX]);

 protected:
  NnRet GenerateRequestId(uint32_t* requestId);

 private:
  std::unordered_set<uint32_t> setOfRequestId_;
  uint32_t nextRequestId_;
};

}  // namespace nn
}  // namespace eden

#endif  // NN_EDEN_NN_H_
