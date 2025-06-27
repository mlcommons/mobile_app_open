# Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

from huggingface_hub.hf_api import HfFolder
HfFolder.save_token('<Enter your hugging face token here>')

print("##############Json import########################")
import json
from argparse import Namespace

with open('config.json', 'rt') as f:
    config = Namespace(**json.load(f))

import sys
import os
sys.path.append('../')

from utilities.nsptargets import NspTargets

# Android GEN2 and GEN3 are supported for this notebook
nsp_target = NspTargets.Android.GEN3

# Select quantsim config based on target
config.config_file = f'./quantsim_configs/htp_quantsim_config_{nsp_target.dsp_arch}.json'
print(f"Using {config.config_file}")

# Uncomment the cell below to sanity check the pipeline before doing a full run
# os.environ['SANITY_CHECK_NOTEBOOK_FLOW'] = "True"

if os.environ.get("SANITY_CHECK_NOTEBOOK_FLOW") == "True":
    config.num_calibration_samples = 1
    config.adaround_iter_text_encoder = 1
    config.adaround_samples_text_encoder = 1
    config.adaround_iter_unet = 1
    config.adaround_samples_unet = 1
    config.adaround_iter_vae = 1
    config.adaround_samples_vae = 1



print("##############Package import########################")
import torch
from redefined_modules.transformers.models.clip.modeling_clip import CLIPTextModel
from redefined_modules.diffusers.models.unet_2d_condition import UNet2DConditionModel
from redefined_modules.diffusers.models.vae import AutoencoderKLDecoder
from diffusers import DPMSolverMultistepScheduler
from transformers import CLIPTokenizer

if config.stable_diffusion_variant == "1.5":
    text_encoder_repo = 'benjamin-paine/stable-diffusion-v1-5'
    text_encoder_subfolder = 'text_encoder'
    text_encoder_revision = 'main'
    unet_repo = 'benjamin-paine/stable-diffusion-v1-5'
    unet_subfolder = 'unet'
    unet_revision = 'main'
    vae_repo = 'benjamin-paine/stable-diffusion-v1-5'
    vae_subfolder = 'vae'
    vae_revision = 'main'
    tokenizer_repo = 'openai/clip-vit-large-patch14'
    tokenizer_subfolder = ''
    tokenizer_revision = 'main'
elif config.stable_diffusion_variant == "2.1":
    text_encoder_repo = "stabilityai/stable-diffusion-2-1-base"
    text_encoder_subfolder = 'text_encoder'
    text_encoder_revision = 'main'
    unet_repo = "stabilityai/stable-diffusion-2-1-base"
    unet_subfolder = 'unet'
    unet_revision = 'main'
    vae_repo = "stabilityai/stable-diffusion-2-1-base"
    vae_subfolder = 'vae'
    vae_revision = 'main'
    tokenizer_repo = "stabilityai/stable-diffusion-2-1-base"
    tokenizer_subfolder = 'tokenizer'
    tokenizer_revision = 'main'
else:
    raise Exception(f"config.stable_diffusion_variant must be either '1.5' or '2.1', found {config.stable_diffusion_variant}")



print("############## Hugging face pipeline initialization ########################")
device = 'cuda'
dtype = torch.half if config.half_precision else torch.float

print("Loading pre-trained TextEncoder model")
text_encoder = CLIPTextModel.from_pretrained(text_encoder_repo,
                                             subfolder=text_encoder_subfolder, revision=text_encoder_revision,
                                             torch_dtype=dtype, cache_dir=config.cache_dir).to(device)
text_encoder.config.return_dict = False

print("Loading pre-trained UNET model")
unet = UNet2DConditionModel.from_pretrained(unet_repo,
                                            subfolder=unet_subfolder, revision=unet_revision,
                                            torch_dtype=dtype, cache_dir=config.cache_dir).to(device)

print("Loading pre-trained VAE model")
vae = AutoencoderKLDecoder.from_pretrained(vae_repo,
                                           subfolder=vae_subfolder, revision=vae_revision,
                                           torch_dtype=dtype, cache_dir=config.cache_dir).to(device)
vae.config.return_dict = False

print("Loading scheduler")
scheduler = DPMSolverMultistepScheduler(beta_start=0.00085,
                                        beta_end=0.012,
                                        beta_schedule="scaled_linear",
                                        num_train_timesteps=1000)
scheduler.set_timesteps(config.diffusion_steps)
scheduler.config.prediction_type = 'epsilon'

print("Loading tokenizer")
tokenizer = CLIPTokenizer.from_pretrained(tokenizer_repo,
                                          subfolder=tokenizer_subfolder, revision=tokenizer_revision,
                                          cache_dir=config.cache_dir)



print("############## Floating pt evaluation ########################")
from stable_diff_pipeline import run_the_pipeline, run_tokenizer, run_text_encoder, run_diffusion_steps, run_vae_decoder, save_image

prompt = "decorated modern country house interior, 8 k, light reflections"
image = run_the_pipeline(prompt, unet, text_encoder, vae, tokenizer, scheduler, config, test_name='fp32')
save_image(image.squeeze(0), 'generated.png')

from IPython.display import Image, display
display(Image(filename='generated.png'))



print("############## Calibrating TE ########################")
from aimet_quantsim import apply_adaround_te, calibrate_te

with open(config.calibration_prompts, "rt") as f:
    print(f'Loading prompts from {config.calibration_prompts}')
    prompts = f.readlines()
    prompts = prompts[:config.num_calibration_samples]

tokens = [run_tokenizer(tokenizer, prompt) for prompt in prompts]

text_encoder_sim = calibrate_te(text_encoder, tokens, config)


print("############## Calibrating UNET ########################")
from aimet_quantsim import calibrate_unet, replace_mha_with_sha_blocks

embeddings = [(run_text_encoder(text_encoder, uncond),
               run_text_encoder(text_encoder, cond)) for cond, uncond in tokens]
embeddings = [torch.cat([uncond, cond])for uncond, cond in embeddings]

unet_sim = calibrate_unet(unet, embeddings, scheduler, config)

replace_mha_with_sha_blocks(unet) # convert unet to SHA so it has same expected inputs as unet_sim which is SHA


print("############## Calibrating VAE ########################")
from aimet_quantsim import calibrate_vae
from tqdm.auto import tqdm

latents = [run_diffusion_steps(unet, emb, scheduler, config, randomize_seed=True) for emb in tqdm(embeddings)]
print('Obtained latents using UNET QuantSim')

vae_sim = calibrate_vae(vae, latents, config)



print("############## Running quantized off target inference ########################")
image = run_the_pipeline(prompt, unet_sim.model, text_encoder_sim.model, vae_sim.model, tokenizer, scheduler, config, test_name="quantized")
save_image(image.squeeze(0), 'generated_after_quant.png')

display(Image(filename='generated_after_quant.png'))



print("############## Export all models ########################")
from aimet_quantsim import export_all_models

export_all_models(text_encoder_sim, unet_sim, vae_sim, tokens, embeddings, latents, batch_sizes_unet=[1])


print("############## Generate artifacts ########################")
from utilities.generate_target_artifacts import generate_target_artifacts

generate_target_artifacts(text_encoder, unet, None, tokenizer, scheduler, config, diffusion_steps=[20,50], seed_list=[1], min_seed=633994880, max_seed=633994880)
