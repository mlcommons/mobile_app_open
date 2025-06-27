# Copyright (c) 2020-2025 Qualcomm Innovation Center, Inc. All rights reserved.
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
FROM ubuntu:jammy
RUN groupadd -r mlperfuser \
    && useradd -r mlperfuser -g mlperfuser
LABEL maintainer="quic_mmundhra@quicinc.com"

# Update the apt configuration and set timezone first or image creation waits
# for selection of timezone
RUN apt-get update && apt-get upgrade -y && apt-get autoremove -y && \
    apt-get install -y --no-install-recommends tzdata

RUN apt-get update && apt-get upgrade -y && apt-get autoremove -y && \
    apt-get install -y --no-install-recommends \
       python3 python3-pip libpython3.10-dev python3.10-venv libgl1-mesa-glx libglib2.0-0 cython3 gcc make curl unzip libc++1-14 \
       git locales openssh-client ca-certificates tar gzip parallel \
       zip bzip2 gnupg wget python3-six python3-pip libncurses5 openjdk-11-jdk-headless clang-format golang-1.13-go build-essential

RUN pip3 install pip==24.1.1 setuptools==45.0.0
RUN pip3 install numpy==1.26.4 opencv-python tensorflow-cpu==2.10.1
RUN pip3 install protobuf==3.6.0
RUN pip3 install Pillow tensorflow_hub tf-slim \
                 absl-py pyyaml
RUN pip3 install pandas matplotlib
RUN pip3 install onnx==1.12.0 onnxruntime packaging
#RUN pip3 install protobuf==3.19.4

RUN curl -SL http://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz | tar -xJC /usr/local/. && \
    mv /usr/local/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-16.04 /usr/local/clang-9.0.0

RUN apt-get update && apt-get install -y --no-install-recommends bash coreutils
RUN which readlink

ARG ndk_version=android-ndk-r25c
ARG android_ndk_home=/opt/android/${ndk_version}
# Install the NDK
# Use wget instead of curl to avoid "Error in the HTTP2 framing layer"
RUN cd /tmp &&  wget --max-redirect=0 -nv https://dl.google.com/android/repository/${ndk_version}-linux.zip && \
    unzip -q /tmp/${ndk_version}-linux.zip -d /opt/android && \
    rm /tmp/${ndk_version}-linux.zip

ENV ANDROID_NDK_HOME ${android_ndk_home}
ENV ANDROID_NDK_ROOT ${android_ndk_home}

ENV PATH=${ANDROID_NDK_ROOT}:/usr/local/clang-9.0.0/bin:${PATH}

RUN ln -s /usr/bin/python3 /usr/bin/python
RUN apt-get clean

RUN mkdir -p /home/mlperf && chmod 777 /home/mlperf
ENV HOME /home/mlperf

USER mlperfuser