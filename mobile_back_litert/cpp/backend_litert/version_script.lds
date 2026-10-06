VERS_1.0 {
  # Export symbols in backend_c.h, plus the LiteRT C API: the GPU accelerator
  # library (libLiteRtClGlAccelerator.so) is dlopened at runtime and binds
  # against the LiteRt* symbols of the library that loads it.
  global:
    mlperf_backend_*;
    LiteRt*;

  # Hide everything else.
  local:
    *;
};
