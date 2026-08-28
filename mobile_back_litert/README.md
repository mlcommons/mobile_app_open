# Mobile backend LiteRT

This backend runs all benchmarks on the
[LiteRT](https://github.com/google-ai-edge/LiteRT) 2.1.5 CompiledModel API:
the `llm-*` benchmarks on a dedicated LLM pipeline, `stable_diffusion` on a
dedicated stable diffusion pipeline, and the vision/NLP benchmarks on a
single-model pipeline.

## Overview

* Android arm64 and iOS arm64. Android claims all 13 benchmarks: the six
  `llm-*` sizes, `stable_diffusion` and the vision/NLP benchmarks. iOS claims
  nine of them -- everything except `llm-3b*` and `llm-8b*`, which cannot fit in
  the iOS per-process memory limit. `llm-1b*` is claimed but does not fit on
  every device either; see [iOS](#ios).
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
* Minimum iOS is 14.0, which is LiteRT's own floor
  (`LITERT_MIN_IOS_VERSION`). The sibling backend frameworks were raised from
  13.1 to match, so every framework the app embeds declares the same minimum.
  The app itself targets 15.0, so nothing here is the binding constraint.
* The vision/NLP benchmarks offer both CPU and Metal and default to Metal,
  which measured 1.7-3.0x faster than CPU across all six on an iPad mini.
  `LiteRtGpuBackend` has no Metal enumerator: on Apple, Metal is selected by
  `kLiteRtGpuBackendAutomatic` plus compile-time Metal support.
* `stable_diffusion` and the `llm-*` benchmarks are CPU only and offer no Metal
  choice. For the LLMs that is a memory result, not a preference: on an iPad
  mini the Metal delegate is killed by `EXC_RESOURCE` while still initialising
  (`delegate_kernel.cc`, "Initializing Metal-based API from graph"), before the
  model finishes compiling and long before a prefill buffer exists. It spends
  the whole ~3.0 GiB budget materialising a graph that carries every prefill
  signature; XNNPACK allocates lazily per signature instead, which is why the
  CPU path gets much further. No prefill tuning reaches a failure that happens
  that early, so offering the choice would only hand the user a delegate that
  kills the app. Android keeps its GPU choice.
* For `stable_diffusion` the CPU-only choice matches Android: the shipped
  exports are `dynamic_int8` (text encoder, diffusion) and `dynamic_fp16`
  (decoder), aimed at CPU/XNNPACK, and a Metal choice would need dedicated fp32
  exports. Measured on an iPad mini at about 4.4-5.6 s per diffusion step, so
  roughly 100 s for the default 20 steps.
* **A query has two phases and they do not fit in memory together.** The three
  models are compiled up front and stay resident, which is fine on Android but
  not here. Measured on an iPhone 16 Pro, in MiB still available before the
  limit:

  | phase | MiB left | |
  |---|---|---|
  | before compiling models | 2849 | ~527 already used by the app |
  | all three models compiled | 1770 | the three models cost 1079 |
  | diffusion done | 373 | the denoising loop needs ~1405 |
  | transient models released | 2688 | freeing them recovered 2315 |
  | decode done | 1279 | the decoder arena needs ~1409 |

  Each phase needs most of the budget on its own, so on Apple only the models
  the current phase uses are kept compiled and the rest are released and
  rebuilt on demand (`set_phase` in `stable_diffusion_pipeline.h`). Compiling
  all three takes about 1.3 s against a ~100 s query. Android has the headroom,
  keeps everything resident, and its throughput does not move.
* Releasing has to be symmetric, and this is easy to get wrong. Freeing only
  the encoder and the diffusion model got the decode to pass, and then the
  *second* query died: it rebuilt those two on top of the decoder's ~1.4 GiB
  arena and had 541 MiB left for a phase that needs ~1405.
* Destroying a model is not sufficient on its own. `free()` does not
  necessarily shrink the process footprint -- libmalloc keeps the pages on its
  free list -- and that footprint is exactly what `EXC_RESOURCE` measures, so
  the release can be invisible to the limit. `ReturnFreeMemoryToOS()` in
  `apple_support.h` calls `malloc_zone_pressure_relief` to hand the pages back;
  it is worth 2.3 GiB in the table above. This matters because LiteRT defers
  `AllocateTensors` to the first `Run`, so a model's arena -- the largest
  single allocation in the pipeline -- is created while it runs, on top of
  whatever the allocator is still holding.
* When a memory question comes up here, measure it rather than reasoning from
  model sizes -- that reasoning has been wrong more than once, most recently by
  taking a model file's size for its resident cost. `LITERT_LOG_MEM("stage")`
  logs `os_proc_available_memory()`, the budget `EXC_RESOURCE` actually
  enforces, and `LITERT_LOG_NOTE(...)` logs a note beside it; both are no-ops
  off Apple. They also go to `os_log`, because a BrowserStack device-log
  artifact carries only `os_log` entries and drops native stderr entirely --
  without that a CI memory failure gives pass/fail and nothing to explain it.
* **Memory is the binding constraint for the LLM benchmarks, and it is decided
  per device.** iOS kills a process that exceeds a per-process limit measured
  at 3376 MB on an 8 GB device (`EXC_RESOURCE`). Measured for `llm-1b` on an
  iPad mini, in MiB still available:

  | stage | MiB left | consumed |
  |---|---|---|
  | `backend_create` start | 2968 | app baseline 408 |
  | model compiled | 896 | **2072** |
  | decode buffers built | 894 | 2 |
  | prefill buffers built | 653 | 241 |
  | prefill inputs written | 461 | 192 |
  | prefill `Run` | — | more than 461, killed |

  Two things there are worth keeping in mind. The weights cost **2072 MiB
  resident against a 1229 MiB model file** -- XNNPACK repacks the q8 weights,
  so file size is not a useful proxy. And this export publishes exactly **one**
  prefill bucket, 1024, which the run above used on a 381-token prompt: there
  is no smaller configuration to fall back to, so `llm-1b` simply does not fit
  on that device. It does run on an iPhone 16 Pro (9.31 tok/s), which has more
  headroom.
* **`llm-1b` is currently claimed on every iOS device anyway**, which is known
  to be wrong for the iPad mini. A tested-device allowlist is the intended fix.
  Gating on `os_proc_available_memory()` was tried and removed: that value is
  not a device property. `mlperf_backend_matches_hardware` is called repeatedly,
  and on one iPhone run it read anywhere from 3319 MiB down to 2600 MiB
  depending on what had already run, so the benchmark list depended on when the
  question was asked. A threshold picked to separate the two devices also came
  within 9 MiB of excluding an iPhone 16 Pro, where the benchmark works.
* `llm-3b` and `llm-8b` are never offered, on any device: at the ratio above
  their weights alone exceed the limit before a single buffer. `CLAIM_SHARED`
  means the TFLite fallback still offers those four.
* The Apple prefill-bucket cap in `GetSuitablePrefillSignature` is a **no-op for
  this export**, which publishes only the 1024 bucket. It is kept because it is
  correct for any export that publishes several -- a bucket larger than the KV
  cache can never be used, since a longer prompt is rejected outright -- but it
  is not what makes anything fit here. Android passes `SIZE_MAX` and is
  unaffected either way.
* Raising the ceiling instead would need the
  `com.apple.developer.kernel.increased-memory-limit` entitlement. That is not
  enabled here: it must also be turned on for the App ID in the developer
  portal, and an entitlement the provisioning profile does not carry breaks
  signing for every iOS backend. It would not help `llm-8b` in any case -- 9.11
  GiB exceeds the RAM of the devices in question.
* There is no CoreML/ANE path: the LiteRT v2 API does not expose one yet
  (upstream marks ANE "coming soon"). Use the Apple backend for CoreML.

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
* `cpp/backend_litert/litert_env.h` — builds the `litert::Environment` shared by
  the pipelines.
* `cpp/backend_litert/apple_support.h` — locates the accelerator inside the app bundle on iOS.
* `cpp/backend_litert/backend_settings/litert_settings_android.pbtxt` and
  `litert_settings_apple.pbtxt` — benchmark settings (models, delegates).
* `cpp/backend_litert/ios/BUILD` — the `liblitertbackend.xcframework` bundle.
* `litert_backend.mk` — make variables and the GPU accelerator downloads.

Models and tokenizers are downloaded from `mobile.mlcommons-storage.org`
as defined in the settings file.
