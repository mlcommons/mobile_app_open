# Stable Diffusion models for the LiteRT GPU delegate

`convert.py` rewrites the three v5_0 Stable Diffusion exports so the LiteRT GPU
delegate accepts them. The published exports run on CPU only; with these
rewrites the diffusion model and the decoder are fully GPU-accelerated, which is
what makes `stable_diffusion` usable on Metal.

The rewrites are shape and metadata changes plus dead-op removal. No weight is
touched, and the script refuses to write a model whose output is not
bit-identical to the original.

## What blocks the GPU delegate

Four separate things, all of which `convert.py` fixes:

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

The text encoder keeps two `GATHER`s on CPU: its token and position indices are
2-D and the delegate only accepts 1-D indices there. It costs ~13 ms against the
20 x 332 ms the diffusion model spends, so it is not worth reshaping around.

## Usage

```bash
pip install ai-edge-litert numpy flatbuffers
python convert.py --in-dir <dir with the v5_0 exports> --out-dir <dir>
```

The input directory needs the three published exports:

* `sd_text_encoder_dynamic_int8.tflite`
* `sd_diffusion_model_dynamic_int8.tflite`
* `sd_decoder_dynamic_fp16.tflite`

The outputs are named `sd_*_litert.tflite`, matching the convention already used
by `llama_q8_ekv3072_litert.tflite`.

Each conversion runs the original and the rewrite on the same random inputs and
prints `max_abs_diff`; anything other than `0.000e+00` aborts the write.

## Hosting

The backend settings download models over HTTPS, so the converted files have to
be published before `litert_settings_apple.pbtxt` can point at them. The
existing precedent is `llama_q8_ekv3072_litert.tflite` under
`https://storage.googleapis.com/mlperf-mobile-public/litert/`. Until the SD
files are uploaded there, the settings keep pointing at the published v5_0
exports and `stable_diffusion` stays on CPU.
