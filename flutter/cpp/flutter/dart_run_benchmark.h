#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void fake_calls();

struct dart_ffi_run_benchmark_in {
  const char *backend_model_path;
  const char *backend_lib_path;
  const void *backend_settings_data;
  int32_t backend_settings_len;
  const char *backend_native_lib_path;

  int dataset_type;
  const char *dataset_data_path;
  const char *dataset_groundtruth_path;
  int32_t dataset_offset;

  const char *scenario;

  const char *mode;
  int32_t min_query_count;
  int32_t min_duration;
  int32_t single_stream_expected_latency_ns;
  const char *output_dir;
};

struct dart_ffi_run_benchmark_out {
  int32_t ok;

  float latency;
  float accuracy_normalized;
  char *accuracy_formatted;
  float accuracy_normalized2;
  char *accuracy_formatted2;
  int32_t num_samples;
  float duration_ms;
  char *backend_name;
  char *backend_vendor;
  char *accelerator_name;
};

struct dart_ffi_run_benchmark_out *dart_ffi_run_benchmark(
    const struct dart_ffi_run_benchmark_in *in);
void dart_ffi_run_benchmark_free(struct dart_ffi_run_benchmark_out *out);

int32_t dart_ffi_get_dataset_size();
int32_t dart_ffi_get_query_counter();

#ifdef __cplusplus
}
#endif
