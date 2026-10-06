#include "embedding_utils.h"

#include <iostream>

bool TsEmbeddingParser::parse_pickle(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return false;
  }

  // Read timesteps array
  uint32_t num_timesteps = 0;
  if (!file.read(reinterpret_cast<char*>(&num_timesteps), sizeof(uint32_t))) {
    std::cerr << "Failed to read the timestep count from: " << filename
              << std::endl;
    return false;
  }
  // One entry per diffusion step of a 1000-step schedule; anything outside
  // that range means this is not a timestep embedding table, and resizing by
  // it would allocate an absurd amount of memory.
  if (num_timesteps == 0 || num_timesteps > MAX_TIMESTEPS) {
    std::cerr << "Implausible timestep count (" << num_timesteps
              << ") in: " << filename << std::endl;
    return false;
  }

  std::vector<int32_t> timesteps(num_timesteps);
  if (!file.read(reinterpret_cast<char*>(timesteps.data()),
                 num_timesteps * sizeof(int32_t))) {
    std::cerr << "Failed to read " << num_timesteps
              << " timesteps from: " << filename << std::endl;
    return false;
  }

  // Read embeddings array
  std::vector<std::vector<float>> embeddings(num_timesteps);
  for (auto& emb : embeddings) {
    emb.resize(EMBEDDING_DIM);
    if (!file.read(reinterpret_cast<char*>(emb.data()),
                   EMBEDDING_DIM * sizeof(float))) {
      std::cerr << "Failed to read the timestep embeddings from: " << filename
                << std::endl;
      return false;
    }
  }

  // Reverse both timesteps and embeddings before storing
  std::reverse(timesteps.begin(), timesteps.end());
  std::reverse(embeddings.begin(), embeddings.end());

  // Store in maps
  timesteps_[num_timesteps] = std::move(timesteps);
  embeddings_[num_timesteps] = std::move(embeddings);

  return true;
}

std::vector<float> TsEmbeddingParser::get_timestep_embedding(
    int32_t steps, int32_t step_index) const {
  auto emb_it = embeddings_.find(steps);
  if (emb_it == embeddings_.end() || step_index < 0 ||
      static_cast<size_t>(step_index) >= emb_it->second.size()) {
    return {};
  }
  return emb_it->second[step_index];
}

std::vector<int32_t> TsEmbeddingParser::get_timesteps(int32_t steps) const {
  auto ts_it = timesteps_.find(steps);
  if (ts_it == timesteps_.end()) {
    return {};
  }
  return ts_it->second;
}

bool EmbeddingManager::load_timestep_embeddings(const std::string& filename) {
  ts_parser_ = std::make_unique<TsEmbeddingParser>();
  return ts_parser_->parse_pickle(filename);
}

std::vector<float> EmbeddingManager::get_timestep_embedding(
    int32_t timestep, int num_steps) const {
  if (!ts_parser_) return {};
  return ts_parser_->get_timestep_embedding(num_steps, timestep);
}

std::vector<int32_t> EmbeddingManager::get_timesteps(int num_steps) const {
  if (!ts_parser_) return {};
  return ts_parser_->get_timesteps(num_steps);
}