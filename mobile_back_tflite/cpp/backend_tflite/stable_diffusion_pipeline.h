/* Copyright 2024 The MLPerf Authors. All Rights Reserved.
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

#ifndef TFLITE_STABLE_DIFFUSION_PIPELINE_H_
#define TFLITE_STABLE_DIFFUSION_PIPELINE_H_

#include "flutter/cpp/c/type.h"
#include "pipeline.h"
#include "tensorflow/lite/c/c_api.h"
#include "tensorflow/core/platform/logging.h"
#include "thread_pool.h"

#include <vector>

struct SDBackendData {
  TfLiteModel* text_encoder_model{nullptr};
  TfLiteModel* first_model{nullptr};
  TfLiteModel* second_model{nullptr};
  TfLiteModel* decoder_model{nullptr};

  TfLiteInterpreter* text_encoder_interpreter{nullptr};
  TfLiteInterpreter* first_interpreter{nullptr};
  TfLiteInterpreter* second_interpreter{nullptr};
  TfLiteInterpreter* decoder_interpreter{nullptr};

  std::vector<int> input_prompt_tokens;
  std::vector<int> unconditional_tokens;

  int num_steps{50};  // Default value, can be modified
  int seed{42};       // Default seed, can be modified

  std::vector<uint8_t> output_image;
  std::unique_ptr<Threadpool> executer;
};

// A pipeline for Stable Diffusion.
class StableDiffusionPipeline : public Pipeline {
 public:
  StableDiffusionPipeline() = default;

  ~StableDiffusionPipeline() override = default;

  void backend_delete(mlperf_backend_ptr_t backend_ptr) override;

  mlperf_backend_ptr_t backend_create(const char *model_path,
                                      mlperf_backend_configuration_t *configs,
                                      const char *native_lib_path) override;

  const char *backend_vendor_name(mlperf_backend_ptr_t backend_ptr) override;

  const char *backend_accelerator_name(
      mlperf_backend_ptr_t backend_ptr) override;

  const char *backend_name(mlperf_backend_ptr_t backend_ptr) override;

  mlperf_status_t backend_issue_query(
      mlperf_backend_ptr_t backend_ptr) override;

  mlperf_status_t backend_flush_queries(
      mlperf_backend_ptr_t backend_ptr) override;

  int32_t backend_get_input_count(mlperf_backend_ptr_t backend_ptr) override;

  mlperf_data_t backend_get_input_type(mlperf_backend_ptr_t backend_ptr,
                                       int32_t i) override;

  mlperf_status_t backend_set_input(mlperf_backend_ptr_t backend_ptr,
                                    int32_t batch_index, int32_t i,
                                    void *data) override;

  int32_t backend_get_output_count(mlperf_backend_ptr_t backend_ptr) override;

  mlperf_data_t backend_get_output_type(mlperf_backend_ptr_t backend_ptr,
                                        int32_t i) override;

  mlperf_status_t backend_get_output(mlperf_backend_ptr_t backend_ptr,
                                     uint32_t batchIndex, int32_t i,
                                     void **data) override;

  void backend_convert_inputs(mlperf_backend_ptr_t backend_ptr, int bytes,
                              int width, int height, uint8_t *data) override;

  void *backend_get_buffer(size_t n) override;

  void backend_release_buffer(void *p) override;

  private:
     TfLiteInterpreter* create_interpreter(TfLiteModel* model);
};

#endif  // TFLITE_STABLE_DIFFUSION_PIPELINE_H_
