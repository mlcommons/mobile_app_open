# Regenerating the QTI models for a submission

How to regenerate every Qualcomm model for a submission: the classic DLCs, Stable
Diffusion and Llama-3.1-8B. It records the hardware, commands, run times and the fixes
the upstream instructions needed for v6.0.2, so the next round can follow it directly.

Versions used for v6.0.2:

| Item | Version |
|---|---|
| QAIRT SDK | 2.51.200.261021 (2.45 has no `hexagon-v85`, needed for SD 8 Elite Gen 6) |
| Llama tutorial | `Tutorial_for_Llama3_1_Compute` v1.0.1.260219 (Qualcomm QPM) |
| AIMET image | `artifacts.codelinaro.org/codelinaro-aimet/aimet-dev:latest.torch-gpu` |

## 1. Hardware

A Linux x86_64 host (Ubuntu 20.04/22.04) with Docker and the NVIDIA container runtime.

| Job | Hardware | Wall time |
|---|---|---|
| Classic DLCs | 8 vCPU, 32 GB RAM, no GPU | a few hours |
| Stable Diffusion (AIMET + QNN, 8 targets) | 1x A100 40 GB, 85 GB RAM | ~10 h |
| Llama Step-1 (AdaScale + quantize/export) | 1x A100 80 GB, >= 170 GB RAM plus ~375 GB swap | ~27 h (AdaScale ~25 h: 32 blocks x ~47 min) |
| Llama Step-2 (per target) | 32 vCPU, 256 GB RAM, no GPU | ~3 h per target |

Sizing notes (none of this is in the upstream docs):

* Llama Step-1 peaks above 172 GB host RAM during the ONNX export (cell 80 of
  `llama3_1.ipynb`); 85 GB is not enough even for AdaScale. Add swap on fast local
  storage, e.g. `sudo mkswap -f /dev/nvme0n1 && sudo swapon /dev/nvme0n1`.
* Llama Step-2 writes ~66 GB or more per AR/CL variant (10 variants). Give the Llama
  run >= 3 TB of disk; ~500 GB is enough for the DLCs and SD.
* Step-2 needs no GPU. Running it on a CPU-only machine after Step-1 cuts cost; drop
  `--gpus` from the `docker run`.

## 2. Classic DLCs

```bash
cd mobile_back_qti/DLC
make docker/htp-dlc SNPE_SDK=$PWD/../qairt/2.51.200.261021
```

* If a previous run left `datasets/output` owned by root, `sudo chown -R $USER datasets/output` first.
* Output: `output/DLC/mlperf_models/*.dlc` (13 files including the `_O2` variants).

## 3. Stable Diffusion

Follow `mobile_back_qti/DLC/util/StableDiffusion/README.md`, with these changes:

* The AIMET `Makefile`'s `docker run` has no `--gpus` flag, so CUDA is invisible. Use a
  copy with `--gpus all` added (e.g. `Makefile.gpu`) and run
  `sudo make -f Makefile.gpu aimet_calibration`, then `make stable_diffusion_qnn SNPE_SDK=...`.
* `aimet_calibration` runs `clean` first and deletes all stamps, so the NVIDIA toolkit
  recipe reruns and `gpg --dearmor` stops at a hidden "overwrite?" prompt. Run inside
  `tmux` and answer `y`.
* Output: 8 folders `stable_diffusion*/` (6 files each). Check one sample image from
  the AIMET quantized model against FP32 before the QNN step.

## 4. Llama-3.1-8B

Follow `mobile_back_qti/DLC/util/LLM/NPU_MODEL_GENERATION/README.md`, run inside the AIMET
container, with these fixes:

1. **Pin dependencies.** The notebooks `pip install --upgrade` unpinned packages. Build a
   constraints file from `Step-1/requirements.txt` (drop the aimet-torch, torch,
   transformers, triton and nvidia/cuda lines) and add **`onnx==1.18.0`**:
   onnx >= 1.19 removes `onnx.mapping` (MHA2SHA fails silently, Step-2 later dies with
   `FileNotFoundError .../sha_output/*.onnx`) and onnx >= 1.22 removes `onnx.version`
   (QAIRT 2.51 converter fails at Step-1 cell 46). Apply it with
   `printf '[install]\nconstraint = /path/pip-constraints.txt\n' > /etc/pip.conf` inside the container.
2. **apt prompt.** The Step-1 notebook runs `sudo apt-get install libtinfo5` without
   `-y` and hangs. Pre-install it in the container: `apt-get install -y libtinfo5`.
3. **Leftovers.** Before a rerun delete `Step-1/c4-train.00000-of-01024.json`
   (gunzip asks to overwrite and hangs) and any partial `Step-1/output_dir`.
4. **Step-2 parallelism.** `Step-2/host_linux/qnn_model_prepare.ipynb` starts
   `max_workers=len(tasks)` processes (up to 60, 25-100 GB each) and swaps the machine to
   a halt. Cap them: 1 worker for the whole-model cells (`gen_ar`, `thread_split`) and 4
   for the per-split cells (MHA2SHA, convert, quantize, context binary).
5. **Gen 6 targets.** `utilities/nsptargets.py` in tutorial v1.0.1.260219 has no Android
   GEN5 entry, so Step-2 for `sd8eliteEG6`/`sd8eliteG6` dies at the context-binary cell
   with `int(None)`. Add `GEN5: 103`, `GEN5: "v85"` and `GEN5: "V85"` to the
   `_Android` soc_ids, dsp_archs and qnn_htp_lib_name maps.
6. **G6 equals EG6.** `run_npu_bin.sh` builds `sd8eliteEG6` and `sd8eliteG6` with identical
   settings (Android, gen 5, soc 103, v85), so the outputs match; build EG6 and copy it
   to `llm_sd8eliteG6` to save ~3 h.
7. `run_npu_bin.sh` is resumable through its `*.stamp` files; Step-1's AdaScale stamp
   lets a rerun skip the 25 h step.

The container command used (GPU for Step-1; drop `--gpus` for a Step-2-only rerun):

```bash
cd mobile_back_qti/DLC/util/LLM/NPU_MODEL_GENERATION/Tutorial_for_Llama3.1_Compute
docker run --rm --gpus '"device=0"' --name=aimet-dev-torch-gpu -v $PWD:$PWD -w $PWD \
  --network=host --ipc=host --shm-size=8G \
  artifacts.codelinaro.org/codelinaro-aimet/aimet-dev:latest.torch-gpu \
  bash -c "apt-get install -y libtinfo5; \
    printf '[install]\nconstraint = $PWD/pip-constraints.txt\n' > /etc/pip.conf && \
    pip install -r requirements.txt && bash run_npu_bin.sh" 2>&1 | tee llm.log
```

Output: `mlperf_models/llm_sd8eliteG5`, `llm_sd8eliteEG6`, `llm_sd8eliteG6` (6 `.bin` +
2 `.json`, ~4.2 GB each). `tokenizer.json` and `llama3_1b.spm.model` are not generated;
the settings reuse the v6.0 copies.

## 5. Publish and wire up

1. Record checksums: `md5sum` every file into `dlc-md5.txt`, `sd-md5.txt`, `llm-md5.txt`.
2. Upload the files to the app's public model storage under
   `app-resources/models/v<N>_<N>_<N>/qualcomm/` (served at
   `https://mobile.mlcommons-storage.org/`) and check each file's size over HTTPS.
3. In `mobile_back_qti/cpp/backend_qti/settings/*.pbtxt` replace every
   `local:///mlperf_models/...` with the public URL and set `model_checksum` to the md5.
4. Bump `QTI_SDK` in `.github/workflows/android-build-test-linux.yml` to the QAIRT
   version the models were built with, and provide that SDK zip where the workflow
   downloads it from.

Regenerated DLC and SD binaries do not match Qualcomm's `settings/checksums.txt`
byte for byte (the build is not deterministic); only `betas.bin`/`lambdas.bin` do. Use
the checksums of the files you publish.
