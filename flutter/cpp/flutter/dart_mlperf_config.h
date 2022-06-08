
#ifdef __cplusplus
extern "C" {
#endif

struct dart_ffi_mlperf_config_result {
  int size;
  char *data;
};

struct dart_ffi_mlperf_config_result *dart_ffi_mlperf_config(
    const char *pb_content);

void dart_ffi_mlperf_config_free(struct dart_ffi_mlperf_config_result *result);

#ifdef __cplusplus
}
#endif
