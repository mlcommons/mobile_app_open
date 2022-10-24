
#ifdef __cplusplus
extern "C" {
#endif

struct dart_ffi_cpuinfo_result {
  const char *soc_name;
};

struct dart_ffi_cpuinfo_result *dart_ffi_cpuinfo();

void dart_ffi_cpuinfo_free(struct dart_ffi_cpuinfo_result *result);

#ifdef __cplusplus
}
#endif
