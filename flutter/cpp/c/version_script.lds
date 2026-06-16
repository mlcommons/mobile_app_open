VERS_1.0 {
  # Export symbols in backend_c.h.
  global:
    mlperf_backend_*;
    LiteRt*;

  # Hide everything else.
  local:
    *;
};
