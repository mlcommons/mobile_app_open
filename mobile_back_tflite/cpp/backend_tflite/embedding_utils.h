#ifndef EMBEDDING_UTILS_H_
#define EMBEDDING_UTILS_H_

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

using float32_t = float;
using tensor_data_float32_t = std::vector<float32_t>;

class TsEmbeddingParser {
 public:
  bool parse(std::ifstream& fin, size_t file_size);
  std::vector<float> get_timestep_embedding(int32_t steps,
                                            int32_t step_index) const;
  std::vector<int32_t> get_timesteps(int32_t steps) const;
  bool saveToJson(const std::string& filename) const;

 private:
  static std::string vectorToJsonArray(const std::vector<float>& vec,
                                       bool full_output = false);
  static std::string vectorToJsonArray(const std::vector<int32_t>& vec);

  static constexpr size_t TS_EMBEDDING_ELEMENT_COUNT = 1280;
  std::map<int32_t, std::vector<int32_t>> ts_seq_map_;
  std::map<int32_t, std::vector<std::vector<float>>> ts_embedding_seq_map_;
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