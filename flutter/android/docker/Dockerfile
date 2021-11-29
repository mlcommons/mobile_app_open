FROM ubuntu:20.04

# Without noninteractive arg apt-get waits for user input
ARG DEBIAN_FRONTEND=noninteractive
# JDK package downloads hundreds of megabytes,
# so let's install it separately to reuse cache in case we change any other dependencies
RUN apt-get update && apt-get install -y openjdk-11-jdk-headless
RUN apt-get update && apt-get install -y \
    apt-transport-https \
    curl \
    git \
    gnupg \
    make \
    protobuf-compiler \
    python3 \
    python3-pip
RUN ln -s /usr/bin/python3 /usr/bin/python
RUN python3 -m pip install numpy absl-py

# Bazel is not present in the standard repositories
RUN curl -L https://bazel.build/bazel-release.pub.gpg | gpg --dearmor | tee /etc/apt/trusted.gpg.d/bazel.gpg >/dev/null && \
    echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list && \
    apt-get update && apt-get install -y bazel=4.2.1

ENV ANDROID_HOME=/opt/android
WORKDIR $ANDROID_HOME/cmdline-tools
# sdkmanager expects to be placed into `$ANDROID_HOME/cmdline-tools/tools`
RUN curl -L https://dl.google.com/android/repository/commandlinetools-linux-7583922_latest.zip | jar x && \
    mv cmdline-tools tools && \
    chmod --recursive +x tools/bin
ENV PATH=$PATH:$ANDROID_HOME/cmdline-tools/tools/bin

RUN yes | sdkmanager --licenses >/dev/null
# Build tools 29.0.2 are required by Flutter 2.5.3
# Build tools 30 are required by bazel 4.2.1
RUN yes | sdkmanager \
    "tools" \
    "platform-tools" \
    "build-tools;29.0.2" \
    "build-tools;30.0.3" \
    "platforms;android-30" \
    "ndk;21.4.7075529"
ENV ANDROID_NDK_HOME=$ANDROID_HOME/ndk/21.4.7075529

WORKDIR /tools
RUN curl -L https://storage.googleapis.com/flutter_infra_release/releases/stable/linux/flutter_linux_2.5.3-stable.tar.xz | tar Jxf - && \
    chmod --recursive a=u flutter
ENV PUB_CACHE=/tools/flutter/.pub-cache
ENV PATH=$PATH:/tools/flutter/bin:$PUB_CACHE/bin
RUN dart pub global activate protoc_plugin

WORKDIR /workdir
ENV HOME=/workdir
RUN chmod --recursive a=u /workdir
ENV GRADLE_USER_HOME=/workdir/.gradle
