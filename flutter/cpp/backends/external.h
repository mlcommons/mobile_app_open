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
#ifndef MLPERF_BACKENDS_EXTERNAL_H_
#define MLPERF_BACKENDS_EXTERNAL_H_

#include "flutter/cpp/backend.h"
#include "flutter/cpp/c/type.h"
#include "flutter/cpp/datasets/allocator.h"
#include "flutter/cpp/proto/backend_setting.pb.h"
#include "flutter/cpp/utils.h"
#include "tensorflow/lite/shared_library.h"

namespace mlperf {
namespace mobile {

// CBackend represent the backend implemented in C.
struct BackendFunctions {
  // If lib_path is empty, try to check if symbol is already loaded.
  BackendFunctions(const std::string& lib_path);

  ~BackendFunctions() {
    if (handle) {
      tflite::SharedLibrary::UnLoadLibrary(handle);
    }
  }
  // Types for function pointer.
  using BackendMatchesPtr = std::add_pointer<bool(
      const char**, const char**, const mlperf_device_info_t*)>::type;
  using BackendCreatePtr = std::add_pointer<mlperf_backend_ptr_t(
      const char*, mlperf_backend_configuration_t*, const char*)>::type;
  using BackendNamePtr =
      std::add_pointer<const char*(mlperf_backend_ptr_t)>::type;
  using BackendVendorPtr =
      std::add_pointer<const char*(mlperf_backend_ptr_t)>::type;
  using AcceleratorNamePtr =
      std::add_pointer<const char*(mlperf_backend_ptr_t)>::type;
  using BackendDeletePtr = std::add_pointer<void(mlperf_backend_ptr_t)>::type;
  using IssueQueryPtr =
      std::add_pointer<mlperf_status_t(mlperf_backend_ptr_t, ft_callback, void*)>::type;
  using FlushQueriesPtr =
      std::add_pointer<mlperf_status_t(mlperf_backend_ptr_t)>::type;

  using GetInputCountPtr =
      std::add_pointer<int32_t(mlperf_backend_ptr_t)>::type;
  using GetInputTypePtr =
      std::add_pointer<mlperf_data_t(mlperf_backend_ptr_t, int32_t)>::type;
  using SetInputPtr = std::add_pointer<mlperf_status_t(
      mlperf_backend_ptr_t, uint32_t, int32_t, void*)>::type;

  using GetOutputCountPtr =
      std::add_pointer<int32_t(mlperf_backend_ptr_t)>::type;
  using GetOutputTypePtr =
      std::add_pointer<mlperf_data_t(mlperf_backend_ptr_t, int32_t)>::type;
  using GetOutputPtr = std::add_pointer<mlperf_status_t(
      mlperf_backend_ptr_t, uint32_t, int32_t, void**)>::type;
  using ConvertInputsPtr = std::add_pointer<void(mlperf_backend_ptr_t, int, int,
                                                 int, uint8_t*)>::type;
  using ConvertOutputsPtr = std::add_pointer<void(mlperf_backend_ptr_t, int,
                                                  int, int, uint8_t*)>::type;

  // Required functions.
  BackendMatchesPtr match{nullptr};
  BackendCreatePtr create{nullptr};
  BackendNamePtr backend_name{nullptr};
  BackendVendorPtr vendor{nullptr};
  AcceleratorNamePtr accelerator_name{nullptr};
  BackendDeletePtr destroy{nullptr};

  IssueQueryPtr issue_query{nullptr};
  FlushQueriesPtr flush_queries{nullptr};

  GetInputCountPtr get_input_count{nullptr};
  GetInputTypePtr get_input_type{nullptr};
  SetInputPtr set_input{nullptr};

  GetOutputCountPtr get_output_count{nullptr};
  GetOutputTypePtr get_output_type{nullptr};
  GetOutputPtr get_output{nullptr};

  // Optional functions
  AllocatorMgr::GetBufferFn get_buffer{nullptr};
  AllocatorMgr::ReleaseBufferFn release_buffer{nullptr};
  ConvertInputsPtr convert_inputs{nullptr};
  ConvertOutputsPtr convert_outputs{nullptr};

  bool isLoaded() { return isloaded; }

  static std::string isSupported(const std::string& lib_path,
                                 const std::string& native_lib_path,
                                 const std::string& manufacturer,
                                 const std::string& model, const char** pbdata);

 private:
  bool isloaded = false;

  // Handle to the loaded library.
  void* handle{nullptr};

  void* GetSymbol(const std::string& name) {
    void* result;
    if (handle) {
      result = tflite::SharedLibrary::GetLibrarySymbol(handle, name.c_str());
    } else {
      result = tflite::SharedLibrary::GetSymbol(name.c_str());
    }

    if (!result) {
      LOG(FATAL) << "Unable to load symbol " << name << ": "
                 << tflite::SharedLibrary::GetError();
    }
    return result;
  }

  void* CheckSymbol(const std::string& name) {
    void* result = nullptr;
    if (handle) {
      result = tflite::SharedLibrary::GetLibrarySymbol(handle, name.c_str());
    } else {
      result = tflite::SharedLibrary::GetSymbol(name.c_str());
    }
    return result;
  }
};

// ExternalBackend runs ML inferences with TFLite.
class ExternalBackend : public Backend {
 public:
  ExternalBackend(const std::string& model_file_path,
                  const std::string& lib_path, const SettingList& settings,
                  const std::string& native_lib_path);

  ~ExternalBackend() {
    backend_functions_.destroy(backend_ptr_);
    DeleteBackendConfiguration(&backend_config_);
  }

  // A human-readable string for logging purposes.
  const std::string& Name() const override { return backend_name_; }

  const std::string& Vendor() const override { return vendor_; }

  // Accelerator name
  const std::string& AcceleratorName() const override {
    return accelerator_name_;
  }

  // Run inference for a sample.
  void IssueQuery(ft_callback callback, void* context) override {
    if (backend_functions_.issue_query(backend_ptr_, callback, context) != MLPERF_SUCCESS) {
      LOG(FATAL) << "Error while inferencing model";
    }
  }

  // Flush the staged queries immediately.
  void FlushQueries() override {
    if (backend_functions_.flush_queries(backend_ptr_) != MLPERF_SUCCESS) {
      LOG(FATAL) << "Error while flushing queries";
    }
  };

  // Sets inputs for a sample before inferencing.
  void SetInputs(const std::vector<void*>& inputs,
                 int batchIndex = 0) override {
    if (inputs.size() != input_format_.size()) {
      LOG(FATAL) << "Number of inputs does not match";
    }

    for (int i = 0; i < inputs.size(); ++i) {
      if (backend_functions_.set_input(backend_ptr_, batchIndex, i,
                                       inputs[i]) != MLPERF_SUCCESS) {
        LOG(FATAL) << "Error while setting inputs";
      }
    }
  }

  // Returns the result after inferencing.
  std::vector<void*> GetPredictedOutputs(int batchIndex = 0) override {
    std::vector<void*> outputs;
    for (int i = 0; i < output_format_.size(); ++i) {
      void* output = nullptr;
      if (backend_functions_.get_output(backend_ptr_, batchIndex, i, &output) !=
          MLPERF_SUCCESS) {
        LOG(FATAL) << "Error while setting inputs";
      }
      outputs.push_back(output);
    }
    return outputs;
  }

  // Returns the input format required by the model.
  const DataFormat& GetInputFormat() override { return input_format_; }

  // Returns the output format produced by the model.
  const DataFormat& GetOutputFormat() override { return output_format_; }

  // Optional function to do input data re-formatting
  void ConvertInputs(int bytes, int width, int height, uint8_t* data) override {
    if (backend_functions_.convert_inputs) {
      backend_functions_.convert_inputs(backend_ptr_, bytes, width, height,
                                        data);
    }
  }

  // Optional function to do output data re-formatting
  void ConvertOutputs(int bytes, int width, int height,
                      uint8_t* data) override {
    if (backend_functions_.convert_outputs) {
      backend_functions_.convert_outputs(backend_ptr_, bytes, width, height,
                                         data);
    }
  }

 private:
  std::string backend_name_;
  std::string vendor_;
  std::string accelerator_name_;
  std::string settings_;
  DataFormat input_format_;
  DataFormat output_format_;
  BackendFunctions backend_functions_;
  mlperf_backend_ptr_t backend_ptr_;
  mlperf_backend_configuration_t backend_config_;
};

}  // namespace mobile
}  // namespace mlperf
#endif  // MLPERF_BACKENDS_EXTERNAL_H_
