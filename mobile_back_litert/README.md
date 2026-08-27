# Mobile backend LiteRT

This backend runs all benchmarks on the
[LiteRT](https://github.com/google-ai-edge/LiteRT) 2.1.5 CompiledModel API:
the `llm-*` benchmarks on a dedicated LLM pipeline, `stable_diffusion` on a
dedicated stable diffusion pipeline, and the vision/NLP benchmarks on a
single-model pipeline.

## Overview

* Android arm64 only. The backend claims the `llm-*` benchmarks,
  `stable_diffusion` and the vision/NLP benchmarks.
* `claim_policy: CLAIM_SHARED`: lower-priority backends (e.g. the TFLite
  fallback) stay selectable next to LiteRT for every claimed benchmark.
* `llm_pipeline.cc` drives a `litert::CompiledModel` with explicit `TensorBuffer`s
  for the prefill and decode signatures.
* `single_model_pipeline.cc` runs the vision/NLP benchmarks on
  `litert::CompiledModel` with the GPU accelerator by default (fp32 model
  exports, automatic CPU fallback); the CPU delegate choice runs the int8
  exports on XNNPACK. NNAPI is not used: it is deprecated since Android 15,
  and the CompiledModel NPU path needs vendor SDKs and AOT-compiled models.
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
  pipeline dispatch (the `pipeline` custom setting selects the LLM or the
  stable diffusion pipeline).
* `cpp/backend_litert/llm_pipeline.h` / `llm_pipeline.cc` — LLM pipeline: tokenizer, prefill, decode, KV cache.
* `cpp/backend_litert/single_model_pipeline.h` / `single_model_pipeline.cc` — CompiledModel pipeline for the vision/NLP benchmarks.
* `cpp/backend_litert/stable_diffusion_pipeline.h` / `stable_diffusion_pipeline.cc`,
  `stable_diffusion_invoker.*`, `sd_utils.*`, `embedding_utils.*` — stable
  diffusion pipeline: text encoder, diffusion loop and decoder. It runs on
  the CPU accelerator; the shipped models are `dynamic_int8` (text encoder,
  diffusion) and `dynamic_fp16` (decoder) exports aimed at CPU/XNNPACK.
* `cpp/backend_litert/backend_settings/litert_settings_android.pbtxt` — benchmark settings (models, delegates).
* `litert_backend.mk` — make variables and the GPU accelerator download.

Models and tokenizers are downloaded from `mobile.mlcommons-storage.org`
as defined in the settings file.
