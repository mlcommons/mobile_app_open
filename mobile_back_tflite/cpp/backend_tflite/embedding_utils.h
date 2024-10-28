// embedding_utils.h
#ifndef EMBEDDING_UTILS_H_
#define EMBEDDING_UTILS_H_

#include <cstdint>
#include <fstream>
#include <map>
#include <string>
#include <vector>

using float32_t = float;
using tensor_data_float32_t = std::vector<float32_t>;

#define TS_EMBEDDING_ELEMENT_COUNT (1*1280)

class FileParser {
public:
    virtual bool parse(std::ifstream& fin, size_t file_size) = 0;
    virtual ~FileParser() = default;
};

class TsEmbeddingParser : public FileParser {
public:
    TsEmbeddingParser() = default;
    ~TsEmbeddingParser() override = default;
    
    bool parse(std::ifstream& fin, size_t file_size) override;

    // Retrieves the timestamp embedding for a specific step
    std::vector<float> get_timestamp_embedding(int32_t steps, int32_t step_index) const;
    
    // Get all timesteps for a specific number of steps
    std::vector<int32_t> get_timesteps(int32_t steps) const;

private:
    std::map<int32_t, std::vector<int32_t>> ts_seq_map_;
    std::map<int32_t, std::vector<tensor_data_float32_t>> ts_embedding_seq_map_;
};

class EmbeddingManager {
public:
    static EmbeddingManager& getInstance() {
        static EmbeddingManager instance;
        return instance;
    }

    bool load_timestamp_embeddings(const std::string& filename);
    std::vector<float> get_timestamp_embedding(int32_t timestep, int num_steps) const;
    std::vector<int32_t> get_timesteps(int num_steps) const;

private:
    EmbeddingManager() = default;
    std::unique_ptr<TsEmbeddingParser> ts_parser_;
};

#endif  // EMBEDDING_UTILS_H_
