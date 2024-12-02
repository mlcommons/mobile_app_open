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

  bool size_check = total_read == file_size;
  // if (size_check) {
  //   std::string json_filename = "timestep_embeddings_debug.json";
  //   if (!saveToJson(json_filename)) {
  //     std::cerr << "Warning: Failed to save embeddings to JSON" << std::endl;
  //   }
  // }

  return size_check;
}

std::string TsEmbeddingParser::vectorToJsonArray(const std::vector<float>& vec,
                                                 bool full_output) {
  std::ostringstream ss;
  ss << "[";
  size_t count = full_output ? vec.size() : std::min(size_t(10), vec.size());
  for (size_t i = 0; i < count; ++i) {
    if (i > 0) ss << ", ";
    ss << std::fixed << std::setprecision(6) << vec[i];
  }
  if (!full_output && vec.size() > 10) ss << ", ...";
  ss << "]";
  return ss.str();
}

std::string TsEmbeddingParser::vectorToJsonArray(
    const std::vector<int32_t>& vec) {
  std::ostringstream ss;  // Changed to ostringstream for consistency
  ss << "[";
  for (size_t i = 0; i < vec.size(); ++i) {
    if (i > 0) ss << ", ";
    ss << vec[i];
  }
  ss << "]";
  return ss.str();
}

bool TsEmbeddingParser::saveToJson(const std::string& filename) const {
  std::ofstream out(filename);
  if (!out.is_open()) {
    std::cerr << "Failed to open output file: " << filename << std::endl;
    return false;
  }

  out << "{\n  \"timestep_sequences\": {\n";

  // Save timestep sequences
  size_t seq_count = 0;
  for (const auto& [steps, sequence] : ts_seq_map_) {
    if (seq_count++ > 0) out << ",\n";
    out << "    \"" << steps << "\": " << vectorToJsonArray(sequence);
  }

  out << "\n  },\n  \"embeddings\": {\n";

  // Save embeddings
  size_t steps_count = 0;
  for (const auto& [steps, step_embeddings] : ts_embedding_seq_map_) {
    if (steps_count++ > 0) out << ",\n";
    out << "    \"" << steps << "\": [\n";

    for (size_t i = 0; i < step_embeddings.size(); ++i) {
      const auto& embedding = step_embeddings[i];

      auto [min_it, max_it] =
          std::minmax_element(embedding.begin(), embedding.end());
      float sum = std::accumulate(embedding.begin(), embedding.end(), 0.0f);
      float avg = sum / embedding.size();

      if (i > 0) out << ",\n";
      out << "      {\n"
          << "        \"step\": " << i << ",\n"
          << "        \"embedding\": " << vectorToJsonArray(embedding, true)
          << ",\n"  // Full output
          << "        \"embedding_size\": " << embedding.size() << ",\n"
          << "        \"min\": " << *min_it << ",\n"
          << "        \"max\": " << *max_it << ",\n"
          << "        \"avg\": " << avg << "\n"
          << "      }";
    }
    out << "\n    ]";
  }

  out << "\n  }\n}";

  if (out.fail()) {
    std::cerr << "Error while writing to file: " << filename << std::endl;
    return false;
  }

  std::cout << "Successfully saved embeddings to " << filename << std::endl;
  return true;
}

std::vector<float> TsEmbeddingParser::get_timestep_embedding(
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

bool EmbeddingManager::load_timestep_embeddings(const std::string& filename) {
  ts_parser_ = std::make_unique<TsEmbeddingParser>();

  std::ifstream fin(filename, std::ios::binary);
  if (!fin) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return false;
  }
  auto file_size = std::filesystem::file_size(filename);
  bool success = ts_parser_->parse(fin, file_size);
  if (!success) {
    std::cerr << "Failed to parse timestep embeddings from " << filename
              << std::endl;
    ts_parser_.reset();
  }
  return success;
}

std::vector<float> EmbeddingManager::get_timestep_embedding(
    int32_t timestep, int num_steps) const {
  if (!ts_parser_) {
    std::cerr << "No timestep parser initialized" << std::endl;
    return std::vector<float>();
  }
  return ts_parser_->get_timestep_embedding(num_steps, timestep);
}

std::vector<int32_t> EmbeddingManager::get_timesteps(int num_steps) const {
  if (!ts_parser_) {
    std::cerr << "No timestep parser initialized" << std::endl;
    return std::vector<int32_t>();
  }
  return ts_parser_->get_timesteps(num_steps);
}