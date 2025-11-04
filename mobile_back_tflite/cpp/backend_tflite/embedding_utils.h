#ifndef EMBEDDING_UTILS_H_
#define EMBEDDING_UTILS_H_

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <vector>

class TsEmbeddingParser {
 public:
  bool parse_pickle(const std::string& filename);
  std::vector<float> get_timestep_embedding(int32_t steps,
                                            int32_t step_index) const;
  std::vector<int32_t> get_timesteps(int32_t steps) const;

 private:
  static constexpr size_t EMBEDDING_DIM = 1280;
  std::map<int32_t, std::vector<int32_t>> timesteps_;
  std::map<int32_t, std::vector<std::vector<float>>> embeddings_;
};

class EmbeddingManager {
 public:
  static EmbeddingManager& getInstance() {
    static EmbeddingManager instance;
    return instance;
  }

  bool load_timestep_embeddings(const std::string& filename);
  std::vector<float> get_timestep_embedding(int32_t timestep,
                                            int num_steps) const;
  std::vector<int32_t> get_timesteps(int num_steps) const;

 private:
  EmbeddingManager() = default;
  std::unique_ptr<TsEmbeddingParser> ts_parser_;
};

#endif  // EMBEDDING_UTILS_H_
