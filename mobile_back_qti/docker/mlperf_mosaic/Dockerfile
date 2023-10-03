# Docker image name: mlperf_mosaic_docker
#sensitive
FROM ubuntu:focal
LABEL maintainer="quic_mmundhra@quicinc.com"

# Update the apt configuration and set timezone first or image creation waits
# for selection of timezone
RUN apt-get update && apt-get upgrade -y && apt-get autoremove -y && \
    apt-get install -y --no-install-recommends tzdata

RUN apt-get update && apt-get upgrade -y && apt-get autoremove -y && \
    apt-get install -y --no-install-recommends \
       python3 python3-pip libpython3.8-dev python3.8-venv libgl1-mesa-glx libglib2.0-0 cython3 gcc make curl unzip libc++1-8 \
       git locales openssh-client ca-certificates tar gzip parallel \
       zip bzip2 gnupg wget python3-six python3-pip libncurses5 openjdk-11-jdk-headless clang-format-10 golang-1.13-go

RUN pip3 install pip==19.3.1 setuptools==31.0.1
RUN pip3 install tensorflow-cpu==2.6.0
RUN pip3 install protobuf==3.20.3
RUN pip3 install Pillow setuptools matplotlib tensorflow_hub tf-slim \
                 absl-py numpy pyyaml

RUN ln -s /usr/bin/python3 /usr/bin/python
RUN apt-get clean

#sensitive
RUN mkdir -p /home/mlperf && chmod 777 /home/mlperf
ENV HOME /home/mlperf
