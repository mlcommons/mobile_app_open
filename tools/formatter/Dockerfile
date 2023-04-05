# Copyright 2021 The MLPerf Authors. All Rights Reserved.
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

# This Docker image is used to run code formatter

FROM ubuntu:20.04

ENV DEBIAN_FRONTEND="noninteractive"

# Install dev packages
RUN apt-get update && apt-get upgrade -y && apt-get install -y --no-install-recommends \
    gcc make git curl wget zip unzip tar gzip build-essential python3-pip

ENV FORMATTER_HOME /opt/formatters/bin
ENV PATH=${PATH}:${FORMATTER_HOME}
RUN mkdir -p ${FORMATTER_HOME}

# Install Java
RUN apt-get update && apt-get install -y --no-install-recommends \
    openjdk-11-jdk-headless

# Add buildifier
RUN apt-get update && apt-get install -y --no-install-recommends \
    golang-1.16-go
ENV PATH=$PATH:/usr/lib/go-1.16/bin:~/go/bin
RUN /usr/lib/go-1.16/bin/go get github.com/bazelbuild/buildtools/buildifier@6.0.1
RUN cp ~/go/bin/buildifier ${FORMATTER_HOME}/

# Add CLang formatter
RUN pip install clang-format==16.0.0 && clang-format --version

# Add dos2unix to format line endings
RUN apt-get update && apt-get install -y --no-install-recommends dos2unix

# Install Node.js and NPM
ENV NODE_ENV="production"
RUN curl --proto '=https' -fsSL https://deb.nodesource.com/setup_lts.x | bash -
RUN apt-get update && apt-get install -y --no-install-recommends nodejs && node -v
RUN npm install --ignore-scripts --location=global npm@8.13.2 && npm --version
RUN npm install --ignore-scripts -g quicktype
RUN npm install --ignore-scripts -g prettier

# Add Markdown formatter
RUN npm install --ignore-scripts -g markdownlint-cli@0.26.0
RUN npm install --ignore-scripts -g markdown-link-check@3.9.0

# Add YAML linter
RUN pip install yamllint==1.26.3


# Add Flutter SDK
ENV UNAME=mlperf
RUN git clone --branch "3.7.6" --single-branch --depth 1 \
    https://github.com/flutter/flutter.git /home/$UNAME/flutter
RUN git config --global --add safe.directory /home/$UNAME/flutter
# The PUB_CACHE directory will be mounted as a docker volume in format.mk
ENV PUB_CACHE=/home/mlperf/.pub-cache
ENV PATH=$PATH:/home/$UNAME/flutter/bin:/home/$UNAME/flutter/bin/cache/dart-sdk/bin:$PUB_CACHE/bin
RUN flutter doctor -v
RUN dart pub global activate protoc_plugin && chmod --recursive a=u $PUB_CACHE
RUN apt-get install -y --no-install-recommends protobuf-compiler
# The /home/mlperf/mobile_app_open directory will be mounted
RUN git config --global --add safe.directory /home/mlperf/mobile_app_open

CMD /bin/bash
