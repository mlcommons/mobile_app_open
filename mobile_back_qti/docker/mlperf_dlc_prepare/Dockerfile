# Copyright (c) 2020-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

# Docker image name: mlcommons/mlperf_dlc_prepare
FROM ubuntu:focal
RUN groupadd -r mlperfuser \
    && useradd -r mlperfuser -g mlperfuser
LABEL maintainer="quic_mmundhra@quicinc.com"

# Update the apt configuration and set timezone first or image creation waits
# for selection of timezone
RUN apt-get update && apt-get upgrade -y && apt-get autoremove -y && \
    apt-get install -y --no-install-recommends tzdata

RUN apt-get update && apt-get upgrade -y && apt-get autoremove -y && \
    apt-get install -y --no-install-recommends \
       python3 python3-pip libpython3.8-dev python3.8-venv libgl1-mesa-glx libglib2.0-0 cython3 gcc make curl unzip libc++1-8 \
       git locales openssh-client ca-certificates tar gzip parallel \
       zip bzip2 gnupg wget python3-six python3-pip libncurses5 openjdk-17-jdk-headless clang-format-10 golang-1.13-go build-essential

RUN pip3 install pip==21.3.1 setuptools==31.0.1
RUN pip3 install tensorflow-cpu==2.13.1
RUN pip3 install protobuf==3.20.3
RUN pip3 install Pillow opencv-python==4.3.0.38 setuptools matplotlib tensorflow_hub tf-slim \
                 absl-py numpy pyyaml decorator scipy attrs pytest tflite psutil

RUN ln -s /usr/bin/python3 /usr/bin/python
RUN apt-get clean

RUN mkdir -p /home/mlperf && chmod 754 /home/mlperf
ENV HOME /home/mlperf

USER mlperfuser
