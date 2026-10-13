# Stable Diffusion models for the LiteRT GPU delegate

`convert.py` rewrites the three v5_0 Stable Diffusion exports so the LiteRT GPU
delegate accepts them. The published exports run on CPU only; with these
rewrites the diffusion model and the decoder are fully GPU-accelerated, which is
what makes `stable_diffusion` usable on Metal.

The rewrites are shape and metadata changes plus dead-op removal. No weight is
touched, and the script refuses to write a model whose output is not
bit-identical to the original.

## What blocks the GPU delegate

Five separate things, all of which `convert.py` fixes -- four that stop the
delegate taking the graph at all, and one that stops the memory option the
delegate needs to fit on a phone:

1. **Dynamic shapes.** Every input declares `-1` on the batch dimension and the
   graphs recompute their own shapes at run time (`SHAPE` ->
   `REDUCE_PROD`/`GATHER`/`PACK`/`CONCATENATION`/`BROADCAST_ARGS` ->
   `RESHAPE`/`BROADCAST_TO`). A `RESHAPE` whose shape operand is computed has a
   dynamic output and the delegate refuses the graph outright:

   ```text
   Attempting to use a delegate that only supports static-sized tensors
   with a graph that has dynamic-sized tensors
   ```

   The pipeline only ever runs batch 1, so those shapes are constants.

2. **`BROADCAST_TO` is not implemented by the GPU delegate.** The group-norm
   blocks use it to materialise both operands of a `MUL`/`SUB` to a common
   shape, which TFLite's binary kernels already do implicitly.

3. **Rank-5 tensors.** The group-norm blocks work in
   `[1, H, W, groups, ch/group]` and the delegate refuses anything above rank 4.
   Every rank-5 tensor in these models has a leading dimension of 1.

4. **A stale `SUB` version.** TFLite raised `SUB` to version 3 because of those
   rank-5 operands; the delegate supports up to version 2. After step 3 nothing
   needs version 3, but the declared version stays and splits the graph into
   partitions too small to delegate. This step is what takes the diffusion and
   decoder graphs from ~7% delegated to fully accelerated.

5. **Per-tensor int8 `FULLY_CONNECTED` weights.** These delegate fine, but they
   make the Metal backend generate a shader that does not compile once constant
   tensor sharing is enabled:

   ```text
   newLibraryWithSource: program_source:29:35:
     error: use of undeclared identifier 'scale'
     half4 w_scale_s0 = half4(float4(scale));
   ```

   The backend carries two templates for dequantising int8 weights -- one that
   reads a per-axis scale tensor, one that takes a scalar -- and it picks the
   scalar form for per-tensor weights without declaring the arguments that form
   references. Single-op models place the fault exactly: per-tensor int8
   `FULLY_CONNECTED` fails, while per-axis `FULLY_CONNECTED`, per-tensor
   `CONV_2D`, per-axis `CONV_2D` and fp16 `CONV_2D` all compile.

   Sharing is not optional here. It decides whether the delegate materialises
   the weights or keeps them stored and dequantises them in the shader, and on
   Metal the diffusion model costs 4409 MiB to compile without it against
   1913 MiB with it -- against roughly 2885 MiB available on an iPhone 16 Pro.
   The accelerator is a prebuilt dylib, so the model is the only side we
   control: re-expressing those weights as per-axis, repeating the one scale
   they already carry, selects the template that compiles. 183 tensors in the
   diffusion model and 72 in the text encoder are rewritten this way; the
   decoder is fp16 and has none.

## Results

Measured on an M-series Mac through the LiteRT `CompiledModel` API, ms per
invocation:

| model | original CPU | rewritten CPU | rewritten Metal |
| --- | --- | --- | --- |
| text encoder | 12.4 | 13.1 | 14.1 |
| diffusion model | 1494.0 | 977.0 | 331.8 |
| decoder | 7702.3 | 2000.5 | 463.1 |

The diffusion model runs once per denoising step (20 by default) and the decoder
once per image, so for one image this is roughly 37.6 s against 7.1 s.

Dropping `BROADCAST_TO` speeds up the CPU path too — it was materialising full
size tensors that the binary kernels now broadcast for free.

That CPU gain does **not** show up end to end, and it does not carry back to
LiteRT 2.1.5 at all. Measured through the C++ backend on a macOS host, one
20-step image on CPU takes 43.84 s with the published models and 43.22 s with
the rewritten ones — a single query each, 1.4% apart, which is noise. The
rewrite is worth adopting for the delegate, not for the CPU path.

These models also need **LiteRT 2.2.0 or newer**. On 2.1.5 the Metal backend
emits invalid shader source for the int8 diffusion weights (`use of undeclared
identifier 'q0'`) and the compile fails whatever the options say.

The text encoder keeps two `GATHER`s on CPU: its token and position indices are
2-D and the delegate only accepts 1-D indices there. It costs ~13 ms against the
20 x 332 ms the diffusion model spends, so it is not worth reshaping around.

## Usage

```bash
uv run convert.py --in-dir <dir with the v5_0 exports> --out-dir <dir>
```

The dependency pins are in the PEP 723 header of `convert.py`, so `uv` builds
the environment itself and nothing is installed into whatever Python the repo
otherwise uses. The `ai-edge-litert` pin is the point: it is the runtime whose
Metal accelerator these models are being made compatible with, and the
conversion should be reproduced against the same one.

The input directory needs the three published exports:

* `sd_text_encoder_dynamic_int8.tflite`
* `sd_diffusion_model_dynamic_int8.tflite`
* `sd_decoder_dynamic_fp16.tflite`

The outputs are named `sd_*_litert.tflite`, matching the convention already used
by `llama_q8_ekv3072_litert.tflite`.

Each conversion runs the original and the rewrite on the same random inputs and
prints `max_abs_diff`; anything other than `0.000e+00` aborts the write.

Running the converter twice, in two separately built environments, produced
byte-identical files, which is why the backend settings can carry these md5s.
Note the limits of that: `ai-edge-litert` is pinned above but `numpy` and
`flatbuffers` are not, so this is evidence of determinism rather than a
guarantee of it. If a regenerated file does not match, download it instead of
assuming the checksum is stale.

Note also what the equivalence check does **not** cover. It runs both models
through the CPU `Interpreter` on one seeded input set per model. That
establishes the rewrite computes the same function on that sample; it does not
exercise the fp16 Metal path these models are actually selected for, and it is
not a proof for all inputs.

| file | md5 |
| --- | --- |
| `sd_text_encoder_litert.tflite` | `dd4041a27340e829dda3eb90928b0804` |
| `sd_diffusion_model_litert.tflite` | `6547cfadc83bd809969dcb90bf754efd` |
| `sd_decoder_litert.tflite` | `8aa94e17f9394958e0c71c653ab1140f` |

## Hosting

The three files are published under
`https://storage.googleapis.com/mlperf-mobile-public/litert/`, alongside
`llama_q8_ekv3072_litert.tflite`, and `litert_settings_apple.pbtxt` points its
`stable_diffusion` Metal choice at them.

Anything republished here has to keep the checksums above in step, and
`listResources` walks *every* delegate choice: a URL that 404s blocks resource
preparation for stable diffusion rather than only degrading the Metal path.
