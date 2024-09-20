FROM artifacts.codelinaro.org/codelinaro-aimet/aimet-dev:1.29.0.torch-gpu

RUN pip install --upgrade huggingface_hub
RUN python3 -m pip install --quiet --upgrade jax ftfy diffusers==0.30.1 transformers==4.30.1 tokenizers==0.11.1 onnx==1.11.0 onnxsim==0.4.33 safetensors==0.3.3 accelerate
RUN pip install --quiet --upgrade https://github.com/quic/aimet/releases/download/1.29.0/AimetCommon-torch_gpu_1.29.0-cp38-cp38-linux_x86_64.whl
RUN pip install --quiet --upgrade https://github.com/quic/aimet/releases/download/1.29.0/AimetTorch-torch_gpu_1.29.0-cp38-cp38-linux_x86_64.whl