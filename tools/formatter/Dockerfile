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

FROM ubuntu:20.04

ENV DEBIAN_FRONTEND="noninteractive"

# Install dev packages
RUN apt-get update && apt-get upgrade -y && apt-get install -y --no-install-recommends \
    tzdata gcc make git curl wget zip unzip tar gzip

# Set timezone to UTC by default
RUN ln -sf /usr/share/zoneinfo/Etc/UTC /etc/localtime

ENV FORMATTER_HOME /opt/formatters/bin
ENV PATH=${PATH}:${FORMATTER_HOME}
RUN mkdir -p ${FORMATTER_HOME}

# Add Java formatter
RUN apt-get install -y openjdk-11-jdk-headless
RUN curl --output `dirname ${FORMATTER_HOME}`/google-java-format-1.9-all-deps.jar \
         -L https://github.com/google/google-java-format/releases/download/google-java-format-1.9/google-java-format-1.9-all-deps.jar
ENV JAVA_HOME /usr/lib/jvm/java-11-openjdk-amd64

# Add buildifier
RUN apt-get install -y golang-1.16-go
ENV PATH=$PATH:/usr/lib/go-1.16/bin:~/go/bin
RUN /usr/lib/go-1.16/bin/go get github.com/bazelbuild/buildtools/buildifier
RUN cp ~/go/bin/buildifier ${FORMATTER_HOME}/

## Add CLang formatter
RUN apt-get install -y clang-format-10
RUN which clang-format-10
RUN cp $(which clang-format-10) ${FORMATTER_HOME}/clang-format

# Add dos2unix to format line endings
RUN apt-get install -y dos2unix

# Setup permission to run Flutter as non-root user
# this is needed to share Flutter cache with host machine
ARG UNAME=mlperf
ARG UID=1000
ARG GID=1000
RUN groupadd -g $GID -o $UNAME
RUN useradd -m -u $UID -g $GID -o -s /bin/bash $UNAME
USER $UNAME

# Add Flutter SDK
RUN git clone --branch "2.5.3" --single-branch --depth 1 \
    https://github.com/flutter/flutter.git /home/$UNAME/flutter
ENV PATH=$PATH:/home/$UNAME/flutter/bin:/home/$UNAME/flutter/bin/cache/dart-sdk/bin
RUN flutter doctor -v
ENV PUB_CACHE=/home/$UNAME/.pub-cache


CMD /bin/bash
