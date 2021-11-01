#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void stop_backend();

// dart_backend.cpp

void fake_calls();

// main.cpp

struct dart_ffi_run_benchmark_in {
  const char *backend_model_path;
  const char *backend_lib_path;
  const void *backend_settings_data;
  int32_t backend_settings_len;
  const char *backend_native_lib_path;

  int dataset_type;  // 0: Imagenet, 1: Coco, 2: Squad, 3: Ade20k
  const char *dataset_data_path;
  const char *dataset_groundtruth_path;
  int32_t dataset_offset;

  const char *scenario;  // SingleStream/Offline
  int32_t batch;

  const char *mode;  // Submission/Accuracy/Performance
  int32_t min_query_count;
  int32_t min_duration;
  const char *output_dir;
};

struct dart_ffi_run_benchmark_out {
  int32_t ok;

  float latency;
  char *accuracy;
  int32_t num_samples;
  float duration_ms;
};

struct dart_ffi_run_benchmark_out *dart_ffi_run_benchmark(
    const struct dart_ffi_run_benchmark_in *in);
void dart_ffi_run_benchmark_free(struct dart_ffi_run_benchmark_out *out);

// dart_ffi_mlperf_config.cc

struct dart_ffi_mlperf_config_result {
  int size;
  char *data;
};

struct dart_ffi_mlperf_config_result *dart_ffi_mlperf_config(
    const char *pb_content);

void dart_ffi_mlperf_config_free(struct dart_ffi_mlperf_config_result *result);

// dart_ffi_backend_match.cc

struct dart_ffi_backend_match_result {
  int matches;  // 0 or 1
  const char *error_message;

  int pbdata_size;
  char *pbdata;
};

struct dart_ffi_backend_match_result *dart_ffi_backend_match(
    const char *lib_path, const char *manufacturer, const char *model);

void dart_ffi_backend_match_free(struct dart_ffi_backend_match_result *result);

#ifdef __cplusplus
}
#endif
