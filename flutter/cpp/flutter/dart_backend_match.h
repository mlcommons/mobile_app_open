#ifdef __cplusplus
extern "C" {
#endif

struct dart_ffi_backend_match_result {
  int matches;  // 0 or 1
  char *error_message;

  int pbdata_size;
  char *pbdata;
};

struct dart_ffi_backend_match_result *dart_ffi_backend_match(
    const char *lib_path, const char *manufacturer, const char *model,
    const char *native_lib_path);

void dart_ffi_backend_match_free(struct dart_ffi_backend_match_result *result);

#ifdef __cplusplus
}
#endif
