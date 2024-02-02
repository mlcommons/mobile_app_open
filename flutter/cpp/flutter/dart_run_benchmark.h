#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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
  int32_t image_width;
  int32_t image_height;

  const char *scenario;

  const char *mode;
  int32_t batch_size;
  int32_t min_query_count;
  double min_duration;
  double max_duration;
  int32_t single_stream_expected_latency_ns;
  const char *output_dir;
};

struct dart_ffi_run_benchmark_out_accuracy {
  float normalized;
  char *formatted;
};

struct dart_ffi_run_benchmark_out {
  bool run_ok;

  struct dart_ffi_run_benchmark_out_accuracy *accuracy1;
  struct dart_ffi_run_benchmark_out_accuracy *accuracy2;

  int32_t num_samples;
  float duration;
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
