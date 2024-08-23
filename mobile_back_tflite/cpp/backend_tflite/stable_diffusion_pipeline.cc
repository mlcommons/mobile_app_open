
#include "stable_diffusion_pipeline.h"

#include "flutter/cpp/c/backend_c.h"

#include "tensorflow/lite/c/c_api.h"
#include "tensorflow/lite/c/common.h"
#include "thread_pool.h"
#include "utils.h"
#include "stable_diffusion_invoker.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

static bool backendExists = false;

mlperf_backend_ptr_t StableDiffusionPipeline::backend_create(
    const char* model_path, mlperf_backend_configuration_t* configs,
    const char* native_lib_path) {
  // The model_path received from main.cc or the app can be a file path or a
  // directory path. Assuming for Stable Diffusion task, the model_path point
  // to a directory where all model files are located and the file names are
  // known beforehand.

  // Verify only one instance of the backend exists at any time
  if (backendExists) {
    // LOG(ERROR) << "Only one backend instance should exist at a time";
    return nullptr;
  }

  SDBackendData* backend_data = new SDBackendData();

  backendExists = true;

  // Load models from the provided directory path
  std::string text_encoder_path = std::string(model_path) + "/text_encoder.tflite";
  std::string first_model_path = std::string(model_path) + "/first_model.tflite";
  std::string second_model_path = std::string(model_path) + "/second_model.tflite";
  std::string decoder_path = std::string(model_path) + "/decoder.tflite";

  backend_data->text_encoder_model = TfLiteModelCreateFromFile(text_encoder_path.c_str());
  backend_data->first_model = TfLiteModelCreateFromFile(first_model_path.c_str());
  backend_data->second_model = TfLiteModelCreateFromFile(second_model_path.c_str());
  backend_data->decoder_model = TfLiteModelCreateFromFile(decoder_path.c_str());

  if (!backend_data->text_encoder_model || !backend_data->first_model ||
      !backend_data->second_model || !backend_data->decoder_model) {
    delete backend_data;
    return nullptr;
  }

  backend_data->text_encoder_interpreter = create_interpreter(backend_data->text_encoder_model);
  backend_data->first_interpreter = create_interpreter(backend_data->first_model);
  backend_data->second_interpreter = create_interpreter(backend_data->second_model);
  backend_data->decoder_interpreter = create_interpreter(backend_data->decoder_model);

  if (!backend_data->text_encoder_interpreter || !backend_data->first_interpreter ||
      !backend_data->second_interpreter || !backend_data->decoder_interpreter) {
    backend_delete(backend_data);
    return nullptr;
  }

  return backend_data;
}

TfLiteInterpreter* StableDiffusionPipeline::create_interpreter(TfLiteModel* model) {
  TfLiteInterpreterOptions* options = TfLiteInterpreterOptionsCreate();
  TfLiteInterpreter* interpreter = TfLiteInterpreterCreate(model, options);
  TfLiteInterpreterOptionsDelete(options);

  if (TfLiteInterpreterAllocateTensors(interpreter) != kTfLiteOk) {
    TfLiteInterpreterDelete(interpreter);
    return nullptr;
  }

  return interpreter;
}

const char* StableDiffusionPipeline::backend_vendor_name(
    mlperf_backend_ptr_t backend_ptr) {
  return "";
}

const char* StableDiffusionPipeline::backend_accelerator_name(
    mlperf_backend_ptr_t backend_ptr) {
  return "";
}

const char* StableDiffusionPipeline::backend_name(
    mlperf_backend_ptr_t backend_ptr) {
  return "";
}

void StableDiffusionPipeline::backend_delete(mlperf_backend_ptr_t backend_ptr) {
  SDBackendData* backend_data = static_cast<SDBackendData*>(backend_ptr);
  if (backend_data) {
    TfLiteModelDelete(backend_data->text_encoder_model);
    TfLiteModelDelete(backend_data->first_model);
    TfLiteModelDelete(backend_data->second_model);
    TfLiteModelDelete(backend_data->decoder_model);
    delete backend_data;
  }
  backendExists = false;
}

mlperf_status_t StableDiffusionPipeline::backend_issue_query(
    mlperf_backend_ptr_t backend_ptr) {
  SDBackendData *backend_data = (SDBackendData *)backend_ptr;
  StableDiffusionInvoker *invoker = new StableDiffusionInvoker(backend_data);
  invoker->invoke();

}

mlperf_status_t StableDiffusionPipeline::backend_flush_queries(
    mlperf_backend_ptr_t backend_ptr) {
  return MLPERF_FAILURE;
}

int32_t StableDiffusionPipeline::backend_get_input_count(
    mlperf_backend_ptr_t backend_ptr) {
  return 2;  // We expect prompt tokens and unconditional tokens
}

mlperf_data_t StableDiffusionPipeline::backend_get_input_type(
    mlperf_backend_ptr_t backend_ptr, int32_t i) {
  mlperf_data_t result; 
  result.type = mlperf_data_t::Float32;
  result.size = 0;
  return result;
}
mlperf_status_t StableDiffusionPipeline::backend_set_input(
    mlperf_backend_ptr_t backend_ptr, int32_t batchIndex, int32_t i,
    void* data) {
      
  SDBackendData* backend_data = static_cast<SDBackendData*>(backend_ptr);

  // Assuming "data" is a vector of integers representing the tokens
  int* tokens = static_cast<int*>(data);
  size_t token_count = backend_data->input_prompt_tokens.size();

  if (i == 0) {
    backend_data->input_prompt_tokens.assign(tokens, tokens + token_count);
  } else if (i == 1) {
    backend_data->unconditional_tokens.assign(tokens, tokens + token_count);
  } else {
    return MLPERF_FAILURE;
  }

  return MLPERF_SUCCESS;
}

int32_t StableDiffusionPipeline::backend_get_output_count(
    mlperf_backend_ptr_t backend_ptr) {
  return 1;
}

mlperf_data_t StableDiffusionPipeline::backend_get_output_type(
    mlperf_backend_ptr_t backend_ptr, int32_t i) {
  mlperf_data_t result;
  result.type = mlperf_data_t::Float32;
  result.size = 0;
  return result;
}

mlperf_status_t StableDiffusionPipeline::backend_get_output(
    mlperf_backend_ptr_t backend_ptr, uint32_t batchIndex, int32_t i,
    void** data) {
  SDBackendData* backend_data = static_cast<SDBackendData*>(backend_ptr);

  if (i == 0) {
    *data = backend_data->output_image.data();
    return MLPERF_SUCCESS;
  }

  return MLPERF_FAILURE;
}

void StableDiffusionPipeline::backend_convert_inputs(
    mlperf_backend_ptr_t backend_ptr, int bytes, int width, int height,
    uint8_t* data) {}

void* StableDiffusionPipeline::backend_get_buffer(size_t n) { return nullptr; }
void StableDiffusionPipeline::backend_release_buffer(void* p) {}



#ifdef __cplusplus
}
#endif  // __cplusplus
