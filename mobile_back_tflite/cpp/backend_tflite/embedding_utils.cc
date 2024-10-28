#include "embedding_utils.h"

#include <filesystem>
#include <iostream>

bool TsEmbeddingParser::parse(std::ifstream& fin, size_t file_size) {
  int32_t steps = 0;
  size_t total_read = 0;

  if (!fin.read(reinterpret_cast<char*>(&steps), sizeof(int32_t))) {
    return false;
  }

  total_read += sizeof(int32_t);
  ts_seq_map_[steps].resize(steps);

  if (!fin.read(reinterpret_cast<char*>(ts_seq_map_[steps].data()),
                sizeof(int32_t) * steps)) {
    return false;
  }

  total_read += sizeof(int32_t) * steps;
  ts_embedding_seq_map_[steps].resize(steps);

  for (int32_t idx = 0; idx < steps; idx++) {
    ts_embedding_seq_map_[steps][idx].resize(TS_EMBEDDING_ELEMENT_COUNT);

    if (!fin.read(
            reinterpret_cast<char*>(ts_embedding_seq_map_[steps][idx].data()),
            sizeof(float32_t) * TS_EMBEDDING_ELEMENT_COUNT)) {
      return false;
    }

    total_read += sizeof(float32_t) * TS_EMBEDDING_ELEMENT_COUNT;
  }

  return total_read == file_size;
}

std::vector<float> TsEmbeddingParser::get_timestamp_embedding(
    int32_t steps, int32_t step_index) const {
  auto steps_it = ts_embedding_seq_map_.find(steps);
  if (steps_it == ts_embedding_seq_map_.end() ||
      step_index >= steps_it->second.size()) {
    return std::vector<float>();
  }
  return steps_it->second[step_index];
}

std::vector<int32_t> TsEmbeddingParser::get_timesteps(int32_t steps) const {
  auto steps_it = ts_seq_map_.find(steps);
  if (steps_it == ts_seq_map_.end()) {
    return std::vector<int32_t>();
  }
  return steps_it->second;
}

bool EmbeddingManager::load_timestamp_embeddings(const std::string& filename) {
  ts_parser_ = std::make_unique<TsEmbeddingParser>();

  std::ifstream fin(filename, std::ios::binary);
  if (!fin) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return false;
  }
  auto file_size = std::filesystem::file_size(filename);
  bool success = ts_parser_->parse(fin, file_size);
  if (!success) {
    std::cerr << "Failed to parse timestamp embeddings from " << filename
              << std::endl;
    // LOG(ERROR) << "Failed to parse embedding file: " << filename;
    ts_parser_.reset();
  }
  return success;
}

std::vector<float> EmbeddingManager::get_timestamp_embedding(
    int32_t timestep, int num_steps) const {
  if (!ts_parser_) {
    std::cerr << "No timestamp parser initialized" << std::endl;
    // LOG(ERROR) << "No timestamp parser initialized";
    return std::vector<float>();
  }
  return ts_parser_->get_timestamp_embedding(num_steps, timestep);
}

std::vector<int32_t> EmbeddingManager::get_timesteps(int num_steps) const {
  if (!ts_parser_) {
    std::cerr << "No timestamp parser initialized" << std::endl;
    // LOG(ERROR) << "No timestamp parser initialized";
    return std::vector<int32_t>();
  }
  return ts_parser_->get_timesteps(num_steps);
}