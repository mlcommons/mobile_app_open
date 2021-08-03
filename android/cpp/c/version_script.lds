VERS_1.0 {
  # Export symbols in backend_c.h.
  global:
    mlperf_backend_*;

  # Hide everything else.
  local:
    *;
};