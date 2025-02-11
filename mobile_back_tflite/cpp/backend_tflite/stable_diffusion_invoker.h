#ifndef STABLE_DIFFUSION_INVOKER_H
#define STABLE_DIFFUSION_INVOKER_H

#include <memory>
#include <string>
#include <vector>

#include "stable_diffusion_pipeline.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/model_builder.h"

class StableDiffusionInvoker {
 public:
  // Constructor that takes the backend data
  StableDiffusionInvoker(SDBackendData* backend_data);

  // The main method to invoke the Stable Diffusion process
  std::vector<float> invoke();

 private:
  // Helper methods to encapsulate different stages of the pipeline
  std::vector<float> encode_prompt(const std::vector<int>& prompt);
  std::vector<float> diffusion_step(const std::vector<float>& latent,
                                    const std::vector<float>& t_emb,
                                    const std::vector<float>& context);
  std::vector<float> diffusion_process(
      const std::vector<float>& encoded_text,
      const std::vector<float>& unconditional_encoded_text, int num_steps,
      int seed);
  int get_tensor_index_by_name(TfLiteInterpreter* interpreter,
                               const std::string& name, bool is_input);
  std::vector<float> decode_image(const std::vector<float>& latent);

  // Utility methods
  std::vector<float> run_inference(TfLiteInterpreter* interpreter,
                                   const std::vector<int>& encoded);
  std::vector<float> run_inference(TfLiteInterpreter* interpreter,
                                   const std::vector<float>& input);
  std::vector<float> get_timestep_embedding(int timestep, int dim = 320,
                                            float max_period = 10000.0f);

  // Reference to the backend data structure containing models and interpreters
  SDBackendData* backend_data_;
};

#endif  // STABLE_DIFFUSION_INVOKER_H