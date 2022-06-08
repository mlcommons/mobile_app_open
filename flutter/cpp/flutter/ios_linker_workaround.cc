#include "./ios_linker_workaround.h"

#include <stdint.h>

#include "./dart_backend_match.h"
#include "./dart_mlperf_config.h"
#include "./dart_run_benchmark.h"
#include "flutter/cpp/c/backend_c.h"

// On iOS we link backend statically and don't export any functions
// Linker sees that we don't use some of the functions and removes them
// (when building for release of profile modes)
// But we still want to access these functions dynamically,
// so we call this function in iOS native code to prevent linker from removing them.
extern "C" void ios_linker_workaround() {
  volatile intptr_t a = 1;
  if (a) return;
  a = (intptr_t)dart_ffi_run_benchmark;
  a = (intptr_t)dart_ffi_run_benchmark_free;
  a = (intptr_t)dart_ffi_mlperf_config;
  a = (intptr_t)dart_ffi_mlperf_config_free;
  a = (intptr_t)dart_ffi_backend_match;
  a = (intptr_t)dart_ffi_backend_match_free;
  a = (intptr_t)mlperf_backend_matches_hardware;
  a = (intptr_t)mlperf_backend_create;
  a = (intptr_t)mlperf_backend_vendor_name;
  a = (intptr_t)mlperf_backend_accelerator_name;
  a = (intptr_t)mlperf_backend_name;
  a = (intptr_t)mlperf_backend_delete;
  a = (intptr_t)mlperf_backend_issue_query;
  a = (intptr_t)mlperf_backend_flush_queries;
  a = (intptr_t)mlperf_backend_get_input_count;
  a = (intptr_t)mlperf_backend_get_input_type;
  a = (intptr_t)mlperf_backend_set_input;
  a = (intptr_t)mlperf_backend_get_output_count;
  a = (intptr_t)mlperf_backend_get_output_type;
  a = (intptr_t)mlperf_backend_get_output;
}
