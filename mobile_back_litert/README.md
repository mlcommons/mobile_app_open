# Mobile backend LiteRT

This backend runs all benchmarks on the
[LiteRT](https://github.com/google-ai-edge/LiteRT) 2.1.5 CompiledModel API:
the `llm-*` benchmarks on a dedicated LLM pipeline, and the vision/NLP
benchmarks on a single-model pipeline.

## Overview

* Android arm64 and iOS arm64. The backend claims the `llm-*` benchmarks and the
  vision/NLP benchmarks (stable diffusion is not supported yet).
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

## iOS

```bash
WITH_LITERT=1 make flutter/ios
```

* The GPU accelerator is the prebuilt `libLiteRtMetalAccelerator.dylib`, pinned
  at 2.1.5 and downloaded by `litert_backend.mk`. It is embedded in
  `Runner.app/Frameworks` next to the backend frameworks: LiteRT dlopens it from
  the directory given by `kLiteRtEnvOptionTagRuntimeLibraryDir`, and iOS has no
  dlopen search path to fall back on.
* Minimum iOS is 14.0, which is LiteRT's own floor (`LITERT_MIN_IOS_VERSION`);
  the other backends in this repo still target 13.1.
* Both CPU and Metal are offered. The vision/NLP benchmarks default to Metal,
  which measured 1.7-3.0x faster than CPU across all six on an iPad mini; the
  `llm-*` benchmarks default to CPU. `LiteRtGpuBackend` has no Metal
  enumerator: on Apple, Metal is selected by `kLiteRtGpuBackendAutomatic` plus
  compile-time Metal support.
* The `llm-*` benchmarks do not currently run on iOS. The prefill buffers are
  sized by the selected prefill signature, so a prompt just over 2048 tokens
  picks the 4096 bucket, and that logits buffer alone is several GB; together
  with the ~1.2 GB model it exceeds the per-process memory limit and iOS kills
  the app (`EXC_RESOURCE`). The observed limit is 3376 MB on an 8 GB device, so
  this is not confined to small devices, and both the CPU and the Metal delegate
  hit it. Android is unaffected -- it has far more headroom.
* There is no CoreML/ANE path: the LiteRT v2 API does not expose one yet
  (upstream marks ANE "coming soon"). Use the Apple backend for CoreML.

## Files

* `cpp/backend_litert/litert_c.cc` — MLPerf backend C API implementation and
  pipeline dispatch (the `pipeline` custom setting selects the LLM pipeline).
* `cpp/backend_litert/llm_pipeline.h` / `llm_pipeline.cc` — LLM pipeline: tokenizer, prefill, decode, KV cache.
* `cpp/backend_litert/single_model_pipeline.h` / `single_model_pipeline.cc` — CompiledModel pipeline for the vision/NLP benchmarks.
* The stable-diffusion sources (`sd_utils.cc`, `stable_diffusion_*.cc`,
  `embedding_utils.cc`) are imported but not compiled or wired up yet.
* `cpp/backend_litert/apple_support.h` — locates the accelerator inside the app bundle on iOS.
* `cpp/backend_litert/backend_settings/litert_settings_android.pbtxt` and
  `litert_settings_apple.pbtxt` — benchmark settings (models, delegates).
* `cpp/backend_litert/ios/BUILD` — the `liblitertbackend.xcframework` bundle.
* `litert_backend.mk` — make variables and the GPU accelerator downloads.

Models and tokenizers are downloaded from `mobile.mlcommons-storage.org`
as defined in the settings file.
