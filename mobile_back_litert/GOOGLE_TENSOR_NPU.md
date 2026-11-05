# Google Tensor NPU on Android (Pixel) with LiteRT

This document describes the architecture, implementation, security constraints, and benchmarking of Google Tensor NPU (EdgeTPU) acceleration using LiteRT on Google Pixel devices (Pixel 6 through Pixel 11+).

---

## 1. Architecture Overview

LiteRT delegates machine learning operations to the Google Tensor EdgeTPU via two core components:

```
┌─────────────────────────────────────────────────────────────────┐
│                    LiteRT CompiledModel                         │
└───────────────┬─────────────────────────────────┬───────────────┘
                │ Graph Partitioning & JIT        │ Hardware Dispatch
                ▼                                 ▼
┌──────────────────────────────┐  ┌──────────────────────────────┐
│    Tensor Compiler Plugin    │  │    Tensor Dispatch Library   │
│  (libLiteRtCompilerPlugin_   │  │   (libLiteRtDispatch_        │
│      google_tensor.so)       │  │     GoogleTensor.so)         │
└───────────────┬──────────────┘  └───────────────┬──────────────┘
                │ JIT Compilation                 │ Session Execution
                ▼                                 ▼
┌──────────────────────────────┐  ┌──────────────────────────────┐
│  vendor.google.edgetpu_      │  │      /dev/edgetpu            │
│       app_service            │  │  (com.google.edgetpu.        │
│  (Restricted Allowlist)      │  │       tachyon-service)       │
└──────────────────────────────┘  └──────────────────────────────┘
```

1. **Compiler Plugin (`libLiteRtCompilerPlugin_google_tensor.so`)**:
   - Scans the flatbuffer graph and partitions NPU-supported operators into subgraphs.
   - Invokes the EdgeTPU compiler adapter (`AdapterOdc`) to compile flatbuffers into EdgeTPU bytecode via `libedgetpu_litert.so`.
2. **Dispatch Library (`libLiteRtDispatch_GoogleTensor.so`)**:
   - Binds to Southbound APIs (`thrContextCreate`, `thrLoadSqContainer`, `thrInvokeAsync`).
   - Communicates with `/dev/edgetpu` via the Tachyon service daemon (`com.google.edgetpu.tachyon-service`).
3. **Compilation Cache**:
   - Persists compiled `.tflite` artifacts containing `DispatchDelegate` nodes to disk (`kLiteRtEnvOptionTagCompilerCacheDir`).
   - Skips future compilation passes when cache hits occur.

---

## 2. Dynamic Linker & App Manifest Configuration

Android 12+ enforces strict linker namespaces for untrusted apps (`clns-X`):
1. **App Manifest (`flutter/android/app/src/main/AndroidManifest.xml`)**:
   Vendor libraries in `/vendor/etc/public.libraries.txt` must be explicitly declared:
   ```xml
   <!-- Google Tensor / LiteRT -->
   <uses-native-library android:name="libedgetpu_litert.so"
                        android:required="false"/>
   <uses-native-library android:name="libedgetpu_client.google.so"
                        android:required="false"/>
   ```
2. **Soname Late Binding Patch (`patches/litert-android-edgetpu-soname.diff`)**:
   `dlopen("/vendor/lib64/libedgetpu_litert.so")` fails inside an untrusted app namespace. It must be opened via its bare soname `"libedgetpu_litert.so"`, allowing the Android dynamic linker to resolve it via `public.libraries.txt`.
3. **Bundling (`mobile_back_litert/litert_backend.mk`)**:
   The plugin and dispatch libraries are bundled directly into `flutter/android/app/src/main/jniLibs/arm64-v8a`:
   - `libLiteRtCompilerPlugin_google_tensor.so`
   - `libLiteRtDispatch_GoogleTensor.so`

---

## 3. The Vendor Security Allowlist & App Execution

### Root Cause of Fallback to CPU in Production APKs

When an app attempts on-device JIT compilation, the compiler plugin contacts `vendor.google.edgetpu_app_service@1.0-service`:
```text
E vendor.google.edgetpu_app_service@1.0-service: org.mlcommons.android.mlperfbench is not in the EdgeTPU allowed list or signature mismatched. Please add the app to the edgetpu allowlist.
E libedgetpu_client: Compile failed, error code 16, message: Current application should not be allowed to access EdgeTPU.
E litert: [compiler_plugin.cc:876] Failed to compile model: Compile failed, error code 16
```
On consumer Pixel devices, Google restricts the JIT compilation API to allowlisted packages (e.g. Google Camera, Recorder) and platform/development UIDs. Third-party APKs fail JIT compilation and LiteRT falls back to CPU (`TfLiteXNNPackDelegate`).

### The Workaround: Runtime Execution is NOT Gated

While JIT *compilation* requires the allowlist, **runtime execution on `/dev/edgetpu` does not**. 

LiteRT's `CompiledModel` automatically skips compiler plugins when a model is either:
- **Pre-compiled Ahead-Of-Time (AOT)** with embedded `DispatchDelegate` bytecode.
- **Present in the compilation cache directory** (`/data/user/0/.../cache`).

When `Flatbuffer model initialized from cached model` occurs, LiteRT binds the `DispatchDelegate` directly to `/dev/edgetpu`, achieving full NPU hardware acceleration without triggering allowlist errors.

---

## 4. Benchmarking on Google Pixel 11 Pro XL

### Live App Performance (Quick Run, NPU)

Seeding the app cache directory with pre-compiled artifacts enables all vision and NLP models to run on LiteRT NPU:

| Benchmark | Model | Mode | LiteRT NPU Throughput | Latency |
|---|---|---|---|---|
| **Object Detection** | `mobiledet_qat` | Performance | **86.90 Q/sec** | ~11.5 ms |
| **Image Segmentation v2** | `mosaic` | Performance | **87.45 Q/sec** | ~11.4 ms |
| **Language Understanding** | `mobilebert_int8` | Performance | **117.78 Q/sec** | **~8.49 ms** |
| **Super Resolution** | `edsr` | Performance | **24.10 Q/sec** | ~41.5 ms (vs 123 ms on CPU) |

---

## 5. Step-by-Step Instructions

### Step 1: Pre-compile Models via CLI Harness

The CLI harness runs under ADB shell (`UID 2000`), which is authorized to perform JIT compilation on EdgeTPU:

```bash
# Push binary and models to device
arch -x86_64 adb push bazel-bin/flutter/cpp/binary/main /data/local/tmp/
arch -x86_64 adb push bazel-bin/mobile_back_litert/cpp/backend_litert/liblitertbackend.so /data/local/tmp/

# Compile models on device (populates /data/local/tmp/<model_name>/...)
arch -x86_64 adb shell "cd /data/local/tmp && ./main EXTERNAL mobiledet_qat --mode=PerformanceOnly --model_file=mobiledet_qat.tflite --lib_path=./liblitertbackend.so"
arch -x86_64 adb shell "cd /data/local/tmp && ./main EXTERNAL mobilebert_int8_384_nnapi --mode=PerformanceOnly --model_file=mobilebert_int8_384_nnapi.tflite --lib_path=./liblitertbackend.so"
arch -x86_64 adb shell "cd /data/local/tmp && ./main EXTERNAL edsr_f32b5_full_qint8 --mode=PerformanceOnly --model_file=edsr_f32b5_full_qint8.tflite --lib_path=./liblitertbackend.so"
arch -x86_64 adb shell "cd /data/local/tmp && ./main EXTERNAL mobile_segmenter_r4_quant_argmax_uint8 --mode=PerformanceOnly --model_file=mobile_segmenter_r4_quant_argmax_uint8.tflite --lib_path=./liblitertbackend.so"
```

### Step 2: Seed the App Cache

Copy the compiled artifacts from `/data/local/tmp` into the app's internal cache:

```bash
for model in mobiledet_qat mobilebert_int8_384_nnapi edsr_f32b5_full_qint8 mobile_segmenter_r4_quant_argmax_uint8; do
  arch -x86_64 adb shell "run-as org.mlcommons.android.mlperfbench cp -r /data/local/tmp/$model /data/user/0/org.mlcommons.android.mlperfbench/cache/"
done
```

### Step 3: Run the App

Open the app, select the benchmarks under LiteRT NPU, and tap **GO**. The logcat will confirm hardware offload:

```text
I litert: [compiled_model.cc:1233] Flatbuffer model initialized from cached model.
I tflite: Replacing 1 out of 1 node(s) with delegate (DispatchDelegate) node, yielding 1 partitions for subgraph 0 (main).
I EdgetpuTachyonCore: [Session ...] Final Activity Report: UID: 10337, PID: 10695, Last Cmd# : 1085
```

---

## 6. Official Production Workflow (Ahead-Of-Time Compilation)

For distribution without manual cache seeding, Google provides the **AOT Compilation** workflow using `ai_edge_litert.aot`:

```python
from ai_edge_litert.aot import aot_compile
from ai_edge_litert.aot.vendors import import_vendor

# Target Google Tensor
tensor_target = import_vendor("GOOGLE_TENSOR").create({
    "backend_id": "GOOGLE_TENSOR",
    "soc_model": "Tensor_G5"
}).target

# AOT compile model
aot_compile(
    model_path="model.tflite",
    target=tensor_target,
    output_dir="compiled_models/",
)
```

The resulting `.tflite` model embeds the EdgeTPU bytecode directly, allowing third-party APKs to run on the NPU immediately upon installation without requiring device allowlist permissions.
