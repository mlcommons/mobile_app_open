# Mobile backend LiteRT

This backend runs all benchmarks on [LiteRT](https://github.com/google-ai-edge/LiteRT) 2.1.5:
the `llm-*` benchmarks on the LiteRT compiled-model API, and the vision/NLP
benchmarks on the TFLite interpreter backed by LiteRT's vendored runtime.

## Overview

* Android arm64 only. The backend claims the `llm-*` benchmarks and the
  vision/NLP benchmarks (stable diffusion is not supported yet).
* `claim_policy: CLAIM_SHARED`: lower-priority backends (e.g. the TFLite
  fallback) stay selectable next to LiteRT for every claimed benchmark.
* `llm_pipeline.cc` drives a `litert::CompiledModel` with explicit `TensorBuffer`s
  for the prefill and decode signatures.
* `single_model_pipeline.cc` runs the vision/NLP benchmarks on the TFLite
  interpreter with the NNAPI or GPU delegate. It compiles against the TFLite
  runtime vendored inside `@litert` — the same copy the compiled-model API
  links — so the `.so` contains a single TF Lite runtime.
* `llm-1b` and `llm-1b-instruct` default to the GPU delegate
  (`libLiteRtClGlAccelerator.so`, downloaded and bundled) with a dedicated
  GPU model export; GPU compilation failure falls back to CPU. The larger
  LLM benchmarks stay on the CPU delegate: the CL delegate cannot fully
  delegate those graphs yet, and partial delegation breaks the KV cache.

## Build

The backend is disabled by default (`WITH_LITERT=0` in the root Makefile),
like the vendor backends. Add `WITH_LITERT=1` to include it; the unified CI
and release builds do.

Build it together with the app libs:

```bash
make flutter/android/libs
```

Or build only the backend library:

```bash
bazel build -c opt --config=android_arm64 //mobile_back_litert/cpp/backend_litert:liblitertbackend.so
```

## Files

* `cpp/backend_litert/litert_c.cc` — MLPerf backend C API implementation and
  pipeline dispatch (the `pipeline` custom setting selects the LLM pipeline).
* `cpp/backend_litert/llm_pipeline.h` / `llm_pipeline.cc` — LLM pipeline: tokenizer, prefill, decode, KV cache.
* `cpp/backend_litert/single_model_pipeline.h` / `single_model_pipeline.cc` — TFLite interpreter pipeline for the vision/NLP benchmarks.
* `cpp/backend_litert/backend_settings/litert_settings_android.pbtxt` — benchmark settings (models, delegates).
* `litert_backend.mk` — make variables and the GPU accelerator download.

Models and tokenizers are downloaded from `mobile.mlcommons-storage.org`
as defined in the settings file.
