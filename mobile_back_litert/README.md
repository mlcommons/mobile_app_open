# Mobile backend LiteRT

This backend runs the `llm-*` benchmarks on [LiteRT](https://github.com/google-ai-edge/LiteRT) 2.1.5.

## Overview

* Android arm64 only. The backend claims only the six `llm-*` benchmarks.
* `claim_policy: CLAIM_SHARED`: the TFLite implementation stays selectable
  next to LiteRT for the `llm-*` benchmarks; all other benchmarks fall
  through to the TFLite backend.
* `llm_pipeline.cc` drives a `litert::CompiledModel` with explicit `TensorBuffer`s
  for the prefill and decode signatures.
* Inference runs on the CPU delegate by default.
* The GPU accelerator (`libLiteRtClGlAccelerator.so`) is downloaded and bundled,
  but only used when a benchmark setting selects it.
  The CL delegate cannot fully delegate the llama graph yet;
  partial delegation breaks the KV cache.

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

* `cpp/backend_litert/litert_c.cc` — MLPerf backend C API implementation.
* `cpp/backend_litert/llm_pipeline.h` / `llm_pipeline.cc` — LLM pipeline: tokenizer, prefill, decode, KV cache.
* `cpp/backend_litert/backend_settings/litert_settings_android.pbtxt` — benchmark settings (models, delegates).
* `litert_backend.mk` — make variables and the GPU accelerator download.

Models and tokenizers are downloaded from `mobile.mlcommons-storage.org`
as defined in the settings file.
