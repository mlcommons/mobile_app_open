VERS_1.0 {
  # Export symbols in backend_c.h.
  global:
    dart_ffi_run_benchmark;
    dart_ffi_run_benchmark_free;
    dart_ffi_get_dataset_size;
    dart_ffi_get_query_counter;
    dart_ffi_backend_match;
    dart_ffi_backend_match_free;
    dart_ffi_mlperf_config;
    dart_ffi_mlperf_config_free;
    dart_ffi_cpuinfo;
    dart_ffi_cpuinfo_free;

  # Hide everything else.
  local:
    *;
};
