#ifndef MLPERF_BACKENDS_SD_STABLE_DIFFUSION_BACKEND_H_
#define MLPERF_BACKENDS_SD_STABLE_DIFFUSION_BACKEND_H_

#include <vector>
#include <memory>
#include <string>
#include <tuple>
#include "flutter/cpp/backend.h"
#include "flutter/cpp/backends/external.h"

namespace mlperf {
namespace mobile {

class StableDiffusionBackend : public Backend {
public:
    StableDiffusionBackend(const std::string& model_first_path,
                           const std::string& model_second_path,
                           const std::string& model_decoder_path,
                           const std::string& backend_lib_path,
                           const SettingList& settings,
                           const std::string& native_lib_path);

    const std::string& Name() const override;
    const std::string& Vendor() const override;
    const std::string& AcceleratorName() const override;

    void IssueQuery() override;
    void FlushQueries() override;
    void SetInputs(const std::vector<void*>& inputs, int batchIndex = 0) override;
    std::vector<void*> GetPredictedOutputs(int batchIndex = 0) override;

    const DataFormat& GetInputFormat() const override;
    const DataFormat& GetOutputFormat() const override;

    void ConvertInputs(int bytes, int image_width, int image_height, uint8_t* data) override;

private:
    std::vector<void*> final_output_;
    std::unique_ptr<ExternalBackend> first_model_;
    std::unique_ptr<ExternalBackend> second_model_;
    std::unique_ptr<ExternalBackend> decoder_;

    std::vector<void*> diffusion_step(std::vector<void*>& latent,
                                      std::vector<void*>& t_emb,
                                      std::vector<void*>& context);

    std::vector<void*> decode(std::vector<void*>& latent);
};

}  // namespace mobile
}  // namespace mlperf

#endif  // MLPERF_BACKENDS_SD_STABLE_DIFFUSION_BACKEND_H_