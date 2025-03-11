
#include "stable_diffusion_pipeline.h"

#include <fstream>
#include <iostream>
#include <random>
#include <valarray>

#include "embedding_utils.h"
#include "flutter/cpp/c/backend_c.h"
#include "flutter/cpp/utils.h"
#include "stable_diffusion_invoker.h"
#include "tensorflow/lite/c/c_api.h"
#include "tensorflow/lite/c/common.h"
#include "thread_pool.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

static bool backendExists = false;

inline mlperf_data_t::Type TfType2Type(TfLiteType type) {
  switch (type) {
    case kTfLiteFloat32:
      return mlperf_data_t::Float32;
    case kTfLiteUInt8:
      return mlperf_data_t::Uint8;
    case kTfLiteInt8:
      return mlperf_data_t::Int8;
    case kTfLiteFloat16:
      return mlperf_data_t::Float16;
    case kTfLiteInt32:
      return mlperf_data_t::Int32;
    case kTfLiteInt64:
      return mlperf_data_t::Int64;
    default:
      LOG(ERROR) << "TfLiteType " << type << " is not supported";
      return mlperf_data_t::Float32;
  }
}

// Add definition for TFLiteNumElements
size_t TFLiteNumElements(const TfLiteTensor* tensor) {
  size_t result = 1;
  for (int i = 0; i < TfLiteTensorNumDims(tensor); ++i) {
    result *= TfLiteTensorDim(tensor, i);
  }
  return result;
}

mlperf_backend_ptr_t StableDiffusionPipeline::backend_create(
    const char* model_path, mlperf_backend_configuration_t* configs,
    const char* native_lib_path) {
  // The model_path received from main.cc or the app can be a file path or a
  // directory path. Assuming for Stable Diffusion task, the model_path point
  // to a directory where all model files are located and the file names are
  // known beforehand.

  // Verify only one instance of the backend exists at any time
  if (backendExists) {
    LOG(ERROR) << "Backend already exists";
    return nullptr;
  }

  SDBackendData* backend_data = new SDBackendData();
  backendExists = true;

  // Read seed and num_steps value from SD task settings
  backend_data->seed =
      mlperf::mobile::GetConfigValue(configs, "stable_diffusion_seed", 0);
  if (backend_data->seed == 0) {
    LOG(ERROR) << "Cannot get stable_diffusion_seed";
    return nullptr;
  }
  backend_data->num_steps =
      mlperf::mobile::GetConfigValue(configs, "stable_diffusion_num_steps", 0);
  if (backend_data->num_steps == 0) {
    LOG(ERROR) << "Cannot get stable_diffusion_num_steps";
    return nullptr;
  }
  
  std::string text_encoder_name = "";
  std::string diffusion_model_name = "";
  std::string decoder_name = "";
  std::string timestep_embeddings_name = "";

  // Look for custom model filename settings
  for (int i = 0; i < configs->count; ++i) {
    if (strcmp(configs->keys[i], "text_encoder_filename") == 0) {
      text_encoder_name = configs->values[i];
    } else if (strcmp(configs->keys[i], "diffusion_model_filename") == 0) {
      diffusion_model_name = configs->values[i];
    } else if (strcmp(configs->keys[i], "decoder_filename") == 0) {
      decoder_name = configs->values[i];
    } else if (strcmp(configs->keys[i], "timestep_embeddings_filename") == 0) {
      timestep_embeddings_name = configs->values[i];
    }
  }

  // Load models from the provided directory path
  std::string text_encoder_path =
      std::string(model_path) + "/" + text_encoder_name;
  std::string sd_model_path =
      std::string(model_path) + "/" + diffusion_model_name;
  std::string decoder_path = std::string(model_path) + "/" + decoder_name;
  std::string ts_embedding_path =
      std::string(model_path) + "/" + timestep_embeddings_name;

  backend_data->text_encoder_model =
      TfLiteModelCreateFromFile(text_encoder_path.c_str());
  backend_data->sd_model = TfLiteModelCreateFromFile(sd_model_path.c_str());
  backend_data->decoder_model = TfLiteModelCreateFromFile(decoder_path.c_str());

  if (!backend_data->text_encoder_model || !backend_data->sd_model ||
      !backend_data->decoder_model) {
    delete backend_data;
    return nullptr;
  }

  backend_data->text_encoder_interpreter =
      create_interpreter(backend_data->text_encoder_model);
  backend_data->sd_interpreter = create_interpreter(backend_data->sd_model);
  backend_data->decoder_interpreter =
      create_interpreter(backend_data->decoder_model);

  if (!backend_data->text_encoder_interpreter ||
      !backend_data->sd_interpreter || !backend_data->decoder_interpreter) {
    backend_delete(backend_data);
    return nullptr;
  }

  if (!EmbeddingManager::getInstance().load_timestep_embeddings(
          ts_embedding_path)) {
    LOG(ERROR) << "Failed to load timestep embeddings from "
               << ts_embedding_path;
    backend_delete(backend_data);
    return nullptr;
  }

  return backend_data;
}

TfLiteInterpreter* StableDiffusionPipeline::create_interpreter(
    TfLiteModel* model) {
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
  SDBackendData* backend_data = static_cast<SDBackendData*>(backend_ptr);
  return backend_data->vendor;
}

const char* StableDiffusionPipeline::backend_accelerator_name(
    mlperf_backend_ptr_t backend_ptr) {
  SDBackendData* backend_data = static_cast<SDBackendData*>(backend_ptr);
  return backend_data->accelerator;
}

const char* StableDiffusionPipeline::backend_name(
    mlperf_backend_ptr_t backend_ptr) {
  SDBackendData* backend_data = static_cast<SDBackendData*>(backend_ptr);
  return backend_data->name;
}

void StableDiffusionPipeline::backend_delete(mlperf_backend_ptr_t backend_ptr) {
  SDBackendData* backend_data = static_cast<SDBackendData*>(backend_ptr);
  if (backend_data) {
    TfLiteModelDelete(backend_data->text_encoder_model);
    TfLiteModelDelete(backend_data->sd_model);
    TfLiteModelDelete(backend_data->decoder_model);
    delete backend_data;
  }
  backendExists = false;
}

mlperf_status_t StableDiffusionPipeline::backend_issue_query(
    mlperf_backend_ptr_t backend_ptr) {
  SDBackendData* backend_data = (SDBackendData*)backend_ptr;
  StableDiffusionInvoker* invoker = new StableDiffusionInvoker(backend_data);
  backend_data->output = invoker->invoke();
  return MLPERF_SUCCESS;
}

mlperf_status_t StableDiffusionPipeline::backend_flush_queries(
    mlperf_backend_ptr_t backend_ptr) {
  return MLPERF_SUCCESS;
}

int32_t StableDiffusionPipeline::backend_get_input_count(
    mlperf_backend_ptr_t backend_ptr) {
  return 1;
}

mlperf_data_t StableDiffusionPipeline::backend_get_input_type(
    mlperf_backend_ptr_t backend_ptr, int32_t i) {
  // Cast the backend pointer to SDBackendData
  SDBackendData* backend_data = static_cast<SDBackendData*>(backend_ptr);

  // Initialize the result with a default type and size
  mlperf_data_t result;
  result.type = mlperf_data_t::Float32;  // Default type, will be updated below
  result.size = 0;

  const TfLiteTensor* tensor = nullptr;

  switch (i) {
    case 0:
      tensor = TfLiteInterpreterGetInputTensor(
          backend_data->text_encoder_interpreter, 0);
      break;
    default:
      std::cerr << "Unsupported input index: " << i << std::endl;
      return result;
  }

  if (tensor) {
    // Map the TensorFlow Lite type to mlperf_data_t::Type
    result.type = TfType2Type(TfLiteTensorType(tensor));
    // Calculate the total number of elements in the tensor
    result.size = TFLiteNumElements(tensor);
  } else {
    std::cerr << "Failed to retrieve tensor for input index: " << i
              << std::endl;
  }

  return result;
}

mlperf_status_t StableDiffusionPipeline::backend_set_input(
    mlperf_backend_ptr_t backend_ptr, int32_t batchIndex, int32_t i,
    void* data) {
  SDBackendData* backend_data = static_cast<SDBackendData*>(backend_ptr);

  int* tokens = static_cast<int*>(data);
  size_t token_count = 0;
  while (tokens[token_count] != 0) {
    ++token_count;
  }

  std::vector<int> unconditioned_tokens(77, 49407);
  unconditioned_tokens[0] = 49406;

  backend_data->input_prompt_tokens.assign(tokens, tokens + token_count);
  backend_data->unconditional_tokens.assign(unconditioned_tokens.begin(),
                                            unconditioned_tokens.end());

  return MLPERF_SUCCESS;
}

int32_t StableDiffusionPipeline::backend_get_output_count(
    mlperf_backend_ptr_t backend_ptr) {
  return 1;
}

mlperf_data_t StableDiffusionPipeline::backend_get_output_type(
    mlperf_backend_ptr_t backend_ptr, int32_t i) {
  SDBackendData* backend_data = static_cast<SDBackendData*>(backend_ptr);

  mlperf_data_t result;
  result.type = mlperf_data_t::Float32;
  result.size = 0;

  const TfLiteTensor* tensor = nullptr;

  switch (i) {
    case 0:
      tensor = TfLiteInterpreterGetOutputTensor(
          backend_data->decoder_interpreter, 0);
      break;
    default:
      std::cerr << "Unsupported output index: " << i << std::endl;
      return result;
  }

  if (tensor) {
    result.type = TfType2Type(TfLiteTensorType(tensor));
    result.size = TFLiteNumElements(tensor);
  } else {
    std::cerr << "Failed to retrieve tensor for output index: " << i
              << std::endl;
  }

  return result;
}

mlperf_status_t StableDiffusionPipeline::backend_get_output(
    mlperf_backend_ptr_t backend_ptr, uint32_t batchIndex, int32_t i,
    void** data) {
  SDBackendData* backend_data = static_cast<SDBackendData*>(backend_ptr);

  if (i == 0) {
    *data = backend_data->output.data();
    return MLPERF_SUCCESS;
  }

  return MLPERF_FAILURE;
}

void StableDiffusionPipeline::backend_convert_inputs(
    mlperf_backend_ptr_t backend_ptr, int bytes, int width, int height,
    uint8_t* data) {}

void StableDiffusionPipeline::backend_convert_outputs(
    mlperf_backend_ptr_t backend_ptr, int bytes, int width, int height,
    uint8_t* data) {}

void* StableDiffusionPipeline::backend_get_buffer(size_t n) {
  return ::operator new(n);
}

void StableDiffusionPipeline::backend_release_buffer(void* p) {
  ::operator delete(p);
}

#ifdef __cplusplus
}
#endif  // __cplusplus
