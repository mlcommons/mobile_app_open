# Mobile backend LiteRT

This backend runs all benchmarks on the
[LiteRT](https://github.com/google-ai-edge/LiteRT) 2.2.0 CompiledModel API:
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
  at 2.2.0 and downloaded by `litert_backend.mk`. It is embedded in
  `Runner.app/Frameworks` next to the backend frameworks: LiteRT dlopens it from
  the directory given by `kLiteRtEnvOptionTagRuntimeLibraryDir`, and iOS has no
  dlopen search path to fall back on.
* It ships wrapped in `LiteRtMetalAccelerator.framework`, assembled by
  `flutter/ios/ios.mk`. An iOS app bundle may only embed bundles, and a bare
  Mach-O under `Frameworks/` is rejected by App Store Connect — that was
  `ITMS-90426`, reported against Xcode Cloud build 265. The framework's
  `CFBundleExecutable` is the dylib's original filename, so the path LiteRT
  builds still resolves; `GetAppleRuntimeLibraryDir()` looks inside the
  framework as well as beside the backend binary.
* Minimum iOS is 15.0: LiteRT's own floor (`LITERT_MIN_IOS_VERSION`), what the
  prebuilt accelerator's Mach-O declares, and what the app already targets. The
  sibling backend frameworks were raised from 13.1 to match, so every framework
  the app embeds declares the same minimum. The accelerator framework's
  `MinimumOSVersion` is read back out of the dylib by `ios.mk` rather than
  written by hand: 2.2.0 raised the dylib from 14.0 to 15.0, and the 14.0 left
  behind is what `ITMS-90208` rejected in build 269.
* The vision/NLP benchmarks offer both CPU and Metal and default to Metal,
  which measured 1.7-3.0x faster than CPU across all six on an iPad mini.
  `LiteRtGpuBackend` has no Metal enumerator: on Apple, Metal is selected by
  `kLiteRtGpuBackendAutomatic` plus compile-time Metal support.
* `stable_diffusion` and the `llm-*` benchmarks each offer a Metal choice.
  **The `llm-*` benchmarks select Metal; `stable_diffusion` stays on CPU.**
  The LLM default follows the measurements below -- Metal costs 72 MiB more at
  peak than CPU and decodes 3.0x faster -- and it puts the choice in front of
  the CI iPhone 16 Pro, which is the device those numbers predict should hold:
  `llm-1b` already passes there on CPU.
  The risk is worth stating plainly. An `EXC_RESOURCE` kill cannot be caught,
  so if the budget does not stretch the app goes down rather than falling back,
  and the measurements are from a macOS host where nothing enforces a ceiling.
  The one iOS device Metal was ever tried on (an iPad mini) crashed -- though
  that device also fails `llm-1b` on **CPU**, so it says nothing about this
  choice. If the device run fails on memory, put `delegate_selected` back to
  `CPU`; nothing else has to change.
  `stable_diffusion` on Metal does **not** work at all -- see below -- so it
  stays on CPU, and its choice exists only so that stops being a guess.

### Measuring this without a device

The dev-utils CLI (`mobile_back_apple/dev-utils/Makefile`) runs a backend `.so`
on the host, which is far faster than an app build and is how the numbers below
were produced. macOS has no per-process jetsam cap, so a crash does not
reproduce there -- but `phys_footprint`, the counter `EXC_RESOURCE` compares
against, is reported at every phase, so "this delegate costs N MiB" transfers
straight to the device budget.

```bash
bazel build -c opt --cxxopt=-std=c++17 --host_cxxopt=-std=c++17 \
  --macos_minimum_os=14.0 \
  //flutter/cpp/binary:main \
  //mobile_back_litert/cpp/backend_litert:liblitertbackend.so
# put the macos_arm64 accelerator next to the .so; the loose layout is what
# GetAppleRuntimeLibraryDir expects from a command-line harness
curl -fSL -o <dir>/libLiteRtMetalAccelerator.dylib \
  https://storage.googleapis.com/litert/binaries/2.2.0/macos_arm64/libLiteRtMetalAccelerator.dylib
bazel-bin/flutter/cpp/binary/main EXTERNAL llm-1b --mode=PerformanceOnly \
  --model_file=<dir with the model> --lib_path=<dir>/liblitertbackend.so \
  --input_tfrecord=tinymmlu.tfrecord --sp_path=llama3_1b.spm.model
```

The CLI copies `delegate_selected` out of the settings verbatim and has no flag
to override it, so testing the Metal choice means flipping `delegate_selected`
in `litert_settings_apple.pbtxt` and rebuilding the `.so` (the settings are
compiled into it).

### llm-1b on Metal, measured

Apple GPU via the same Metal accelerator iOS uses, one MMLU query, MiB of
`phys_footprint`:

| stage | CPU | Metal, sharing **on** | Metal, sharing **off** |
|---|---|---|---|
| after compile | 2094 | 2055 | 4497 |
| peak (prefill Run) | 3392 | 3464 | 5839 |
| time per output token | 49.10 ms | **16.28 ms** | -- |
| first token latency | 2.433 s | **1.229 s** | -- |
| TinyMMLU accuracy (100 samples) | 42.00% | 41.00% | -- |

The accuracy line matters as much as the speed one: one sample in a hundred
separates them, which is what an fp16 GPU path against an int8/fp32 CPU path
should look like. Metal is not trading correctness for the 3x.

Two things follow. **Metal costs what CPU costs** once constant-tensor sharing
is on -- 2055 against 2094 after compile, and 72 MiB more at peak -- so a
device that runs `llm-1b` on CPU has the headroom to run it on Metal. And
**Metal is 3.0x faster per output token** and 2.0x faster to first token, which
is the opposite of the Android GPU result (about half of CPU) that the earlier
"nothing is lost" reasoning leaned on.

The third column is the bug. With sharing off the compile alone costs 4497 MiB
against a 3376 MB device cap, which is precisely the iPad mini crash: killed
inside `delegate_kernel.cc` "Initializing Metal-based API from graph", before
the model finished compiling. The log shows exactly two of those lines -- the
prefill and decode subgraphs -- and 4497 - 2055 = 2442 MiB is one extra copy of
the weights. That is what `EnableConstantTensorSharing` collapses.

* Offering the LLM choice costs a download. `listResources()` in
  `benchmark.dart` walks every `delegate_choice`, so the GPU export is fetched
  whether or not Metal is ever selected: +1.25 GB on iOS, once, since `llm-1b`
  and `llm-1b-instruct` name the same URL and the resource set dedupes.
  Android already pays this. The `stable_diffusion` Metal choice costs nothing
  extra -- it names the same four files as its CPU choice, and the
  per-delegate model directory is symlinks into one shared download.
* The LLM Metal choice was withdrawn once and has now been reinstated with a
  fix. What was measured on an iPad mini: the delegate is killed by
  `EXC_RESOURCE` while still initialising (`delegate_kernel.cc`, "Initializing
  Metal-based API from graph"), spending the whole ~3.0 GiB budget in about
  0.3 s, before the model finishes compiling and long before a prefill buffer
  exists. The reading at the time was that the graph carries every prefill
  signature and the delegate pays for all of them, against XNNPACK allocating
  lazily per signature.
  That is the symptom; the mechanism is `EnableConstantTensorSharing`. With it
  off -- which is what the pipeline inherited from LiteRT-LM's Android options
  -- constant tensors are *not* shared between subgraphs, so each signature
  carries its own copy of weights that are 2072 MiB resident. Turning it on
  routes them through LiteRT's mmap-backed `SharedMemoryManager`, and
  `SetMadviseOriginalSharedTensors` lets the kernel drop the pages the layout
  converter has already read. Clean file-backed pages do not count against
  `phys_footprint`, which is what `EXC_RESOURCE` measures. Both are now on for
  Apple only; Android's options are unchanged.
  Note the same iPad mini also fails `llm-1b` on **CPU**, so it is not the
  device to validate this on. An `EXC_RESOURCE` kill cannot be caught, so there
  is no fallback to catch it: if the budget runs out the app goes down. Watch
  the `[mem] llm: before GPU compile` and `[mem] llm: after GPU compile` lines
  to see what the delegate actually costs.
* **`stable_diffusion` cannot use Metal with the models that ship today, but
  the models can be rewritten so that it can.** Each of the three published
  exports fails to compile against the Metal accelerator:

  ```text
  WARNING: Attempting to use a delegate that only supports static-sized tensors
           with a graph that has dynamic-sized tensors
  [probe] text_encoder on GPU: FAILED
  [probe] diffusion   on GPU: FAILED
  [probe] decoder     on GPU: FAILED
  ```

  This is not caused by the GPU options -- compiling with none of them set
  fails identically -- and it is not partial delegation degrading to CPU, it is
  `CompiledModel::Create` returning an error.

  The declared batch dimension looks like the culprit and is not. Each model
  does declare `-1` for batch (`tokens [-1,77]`, `latent [-1,64,64,4]`,
  `input_1 [-1,64,64,4]`), but rewriting `shape_signature` in all three so they
  report **0 dynamic tensors** changes nothing.

  There are four blockers, not one, and `tools/sd_gpu/convert.py` fixes all
  four; see that directory's README for the details and the measurements.

  1. The graphs compute their shapes at run time (`SHAPE` ->
     `REDUCE_PROD`/`GATHER`/`PACK`/`BROADCAST_ARGS` -> `RESHAPE`/`BROADCAST_TO`),
     so a `RESHAPE` output stays dynamic however concrete the inputs are.
     Since the pipeline only ever runs batch 1, those shapes are constants and
     can be folded away.
  2. `BROADCAST_TO` is not implemented by the GPU delegate at all.
  3. The group-norm blocks work in rank 5, and the delegate refuses anything
     above rank 4.
  4. `SUB` is declared at version 3 -- which only the rank-5 operands needed --
     against a delegate that supports version 2, which alone splits the graph
     into partitions too small to delegate.

  With all four fixed the diffusion model and the decoder come out **fully**
  GPU-accelerated and every output stays bit-identical. On an M-series Mac, ms
  per invocation:

  | model | original CPU | rewritten CPU | rewritten Metal |
  |---|---|---|---|
  | text encoder | 12.4 | 13.1 | 14.1 |
  | diffusion model | 1494.0 | 977.0 | 331.8 |
  | decoder | 7702.3 | 2000.5 | 463.1 |

  This needs **LiteRT 2.2.0**, which is why this backend pins it. On 2.1.5 the
  Metal backend emits invalid shader source for the int8 weights,

  ```text
  newLibraryWithSource: program_source:30:30: error: use of undeclared identifier 'q0'
    half4 weight_scale = half4(q0);
  ```

  with `AllowSrcQuantizedFcConvOps` both on and off, so it is a code-generation
  bug rather than something the options control. The text encoder compiles on
  2.1.5; the diffusion model does not. The accelerator cannot be upgraded on
  its own either: a 2.2.0 `libLiteRtMetalAccelerator.dylib` will not load into
  a 2.1.5 runtime, nor the reverse.

  A near-identical bug survives into 2.2.0 and is worth knowing about before
  touching this pipeline's GPU options: with **constant-tensor sharing on**,
  the diffusion model still fails, now with `use of undeclared identifier
  'scale'`. Bisecting the options one at a time shows sharing alone is the
  trigger, so the SD pipeline turns it off, and turns off
  `AllowSrcQuantizedFcConvOps` with it because `litert_gpu_options.h` says
  sharing "must be true to use this" -- and the rewritten models are fully
  delegated without the quantized fc/conv path anyway. Dropping that path also
  stops quantizing inputs to 8 bit, which the header notes costs accuracy.

  Turning sharing off is **not** free, and it would be wrong to claim a single
  signature makes it cheap. `litert_gpu_options.h` says the option reduces
  allocation for constant tensors "even though tensors are not shared with
  other subgraphs", so this gives up their mmap/madvise handling rather than
  only cross-subgraph de-duplication -- on the pipeline already closest to the
  iOS per-process limit. It is accepted because the alternative is not a
  heavier Metal run but no Metal run at all. **The iOS memory cost has not
  been measured**; every number here is from a macOS host. The `llm-*`
  pipeline keeps sharing, where it does the memory work it was added for.

  End to end through this backend on a macOS host, one 20-step image:

  | models | delegate | per image | per step |
  |---|---|---|---|
  | published v5_0 | CPU | 43.84 s | ~2.1 s |
  | rewritten | Metal | 18.8-19.4 s | ~0.82 s |

  Two runs, so the Metal figure is a range rather than a number. "Metal" here
  means the pipeline compiled all three models on the delegate and never took
  its CPU retry path; it is not a claim that no operator ran on CPU, and two
  `GATHER`s in the text encoder do -- see `tools/sd_gpu/README.md`.

  For reference the rewrite is worth nothing on CPU on its own -- 43.22 s
  against 43.84 s, one query each, which is noise -- so it only pays off
  together with the delegate.

  When a GPU compile does fail the pipeline handles it rather than pretending:
  everything built so far is released, the pages are handed back and the three
  recompile on CPU. All three compile on one accelerator or none do, because
  the benchmark reports a single accelerator name. On CPU an iPad mini measures
  about 4.4-5.6 s per diffusion step, so roughly 100 s for 20 steps.
* **Constant tensor sharing is what makes the Metal path fit, and one shader
  bug stood in the way of it.** The option decides whether the delegate
  materialises the weights or keeps them stored and dequantises them in the
  shader. Measured on a macOS host with `phys_footprint`, the counter
  `EXC_RESOURCE` compares against:

  | model | file | sharing off | sharing on |
  |---|---|---|---|
  | text encoder (int8) | 118 MiB | 446 MiB | 118 MiB |
  | diffusion model (int8) | 822 MiB | 4409 MiB | 1913 MiB |
  | decoder (fp16) | 95 MiB | 1022 MiB | 217 MiB |

  About 2885 MiB is available to this benchmark on an iPhone 16 Pro, so with
  sharing off the diffusion model does not fit on its own and no arrangement of
  the pipeline rescues it.

  Sharing could not be turned on because the Metal backend generated a shader
  that does not compile -- `use of undeclared identifier 'scale'`. It carries
  two templates for dequantising int8 weights, one reading a per-axis scale
  tensor and one taking a scalar, and it picks the scalar form for per-tensor
  weights without declaring the arguments that form references. Single-op
  models place the fault exactly: per-tensor int8 `FULLY_CONNECTED` fails,
  while per-axis `FULLY_CONNECTED`, per-tensor `CONV_2D`, per-axis `CONV_2D`
  and fp16 `CONV_2D` all compile. The accelerator is a prebuilt dylib, so the
  model is the only side we control -- `tools/sd_gpu/convert.py` re-expresses
  those weights as per-axis, repeating the one scale they already carry. The
  weight bytes are untouched and the CPU output is bit-identical.
* **On the GPU the three models stay resident, because releasing one does not
  give the memory back.** `ReleaseModel` drops the LiteRT objects and
  `ReturnFreeMemoryToOS` empties libmalloc's free list, but the delegate's
  weights live in Metal buffers libmalloc never owned. Measured on an iPhone 16
  Pro, in MiB still available before the limit:

  | stage | MiB left | |
  |---|---|---|
  | before compiling models | 2885 | |
  | all three models compiled | 1517 | the three cost 1368 |
  | query start, after releasing two of them | 1586 | only 69 recovered |
  | diffusion model rebuilt | 72 | then `ActiveHard 3376 MB (fatal)` |

  Rebuilding stacks a second copy on top of the first. Compiled once and left
  alone the three fit with the working set on top, so `set_phase` is simply not
  installed on the GPU path — which also takes a 20-step image from 45.5 s to
  18.8 s on the macOS host, since nothing is recompiled per query.

  The CPU path keeps the phases: there the weights are ordinary allocations
  that the release does return, and the three models plus a phase's working set
  do not fit together. Android has the headroom, keeps everything resident, and
  its throughput does not move.
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
* Raising the ceiling is the other half of the approach, and
  `flutter/ios/Runner/Runner.entitlements` now requests both
  `com.apple.developer.kernel.increased-memory-limit` (raises the per-process
  cap jetsam enforces) and
  `com.apple.developer.kernel.extended-virtual-addressing` (lifts the
  address-space limit the same allocations meet once the cap is up).
  **They are inert until the matching capability is enabled for the App ID in
  the developer portal**, and a provisioning profile that does not carry them
  fails to sign every iOS backend -- so they are a separate commit, revertible
  on its own if the signing setup is not ready. Neither helps `llm-8b`: 9.11
  GiB exceeds the RAM of the devices in question, which is why it is still not
  claimed.
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
