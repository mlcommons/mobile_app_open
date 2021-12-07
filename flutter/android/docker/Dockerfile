FROM ubuntu:20.04

# Without DEBIAN_FRONTEND=noninteractive arg apt-get waits for user input.
#   Docker desktop shows all previously defined args for each of the commands,
#   which makes reading layer info harder, so we set this env for for individual commands only.
# JDK package downloads ~500 MB from slow mirrors, which can take a lot of time,
#   so a separate layer for it makes image rebuild faster in case we cahnge any other dependencies.
RUN apt-get update >/dev/null && DEBIAN_FRONTEND=noninteractive apt-get install -y openjdk-8-jdk-headless
RUN apt-get update >/dev/null && DEBIAN_FRONTEND=noninteractive apt-get install -y \
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
RUN curl -L https://bazel.build/bazel-release.pub.gpg | apt-key add - && \
    echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list && \
    apt-get update >/dev/null && DEBIAN_FRONTEND=noninteractive apt-get install -y bazel=4.2.1

ENV ANDROID_SDK_ROOT=/opt/android
WORKDIR $ANDROID_SDK_ROOT/cmdline-tools
# sdkmanager expects to be placed into `$ANDROID_SDK_ROOT/cmdline-tools/tools`
RUN curl -L https://dl.google.com/android/repository/commandlinetools-linux-7583922_latest.zip | jar x && \
    mv cmdline-tools tools && \
    chmod --recursive +x tools/bin
ENV PATH=$PATH:$ANDROID_SDK_ROOT/cmdline-tools/tools/bin

RUN yes | sdkmanager --licenses >/dev/null
# Build tools 29.0.2 are required by Flutter 2.5.3
# Build tools 30 are required by bazel 4.2.1
RUN yes | sdkmanager \
    "tools" \
    "platform-tools" \
    "build-tools;29.0.2" \
    "build-tools;30.0.3" \
    "platforms;android-30"
# Install NDK in a separate layer to decrease max layer size.
RUN yes | sdkmanager "ndk;21.4.7075529"
ENV ANDROID_NDK_ROOT=$ANDROID_SDK_ROOT/ndk/21.4.7075529
ENV ANDROID_NDK_HOME=$ANDROID_NDK_ROOT

ENV HOME=/image-workdir
WORKDIR $HOME

ENV PUB_CACHE=$HOME/flutter/.pub-cache
ENV PATH=$PATH:$HOME/flutter/bin:$PUB_CACHE/bin
RUN curl -L https://storage.googleapis.com/flutter_infra_release/releases/stable/linux/flutter_linux_2.5.3-stable.tar.xz | tar Jxf -
RUN dart pub global activate protoc_plugin

ENV GRADLE_USER_HOME=$HOME/.gradle
ENV ANDROID_SDK_HOME=$HOME/.android

RUN mkdir $ANDROID_SDK_HOME && \
    keytool -genkey -v -keystore $ANDROID_SDK_HOME/debug.keystore -storepass android -alias androiddebugkey -keypass android -dname "CN=Android Debug,O=Android,C=US"

RUN chmod --recursive a=u $HOME
