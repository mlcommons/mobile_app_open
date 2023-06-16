FROM ubuntu:20.04

# Without DEBIAN_FRONTEND=noninteractive arg apt-get waits for user input.
#   Docker desktop shows all previously defined args for each of the commands,
#   which makes reading layer info harder, so we set this env for for individual commands only.
# JDK package downloads ~500 MB from slow mirrors, which can take a lot of time,
#   so a separate layer for it makes image rebuild faster in case we change any other dependencies.
RUN apt-get update >/dev/null && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    openjdk-11-jdk-headless
RUN apt-get update >/dev/null && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential \
    apt-transport-https \
    curl \
    git \
    unzip \
    pwgen \
    gnupg \
    make \
    protobuf-compiler \
    python3 \
    python3-pip
RUN ln -s /usr/bin/python3 /usr/bin/python
RUN python3 -m pip install numpy absl-py

RUN curl --proto '=https' -L https://github.com/bazelbuild/bazelisk/releases/download/v1.16.0/bazelisk-linux-amd64 -o /usr/local/bin/bazel && \
    chmod +x /usr/local/bin/bazel

ENV ANDROID_SDK_ROOT=/opt/android
WORKDIR $ANDROID_SDK_ROOT/cmdline-tools
# sdkmanager expects to be placed into `$ANDROID_SDK_ROOT/cmdline-tools/tools`
RUN curl --proto '=https' -L https://dl.google.com/android/repository/commandlinetools-linux-7583922_latest.zip | jar x && \
    mv cmdline-tools tools && \
    chmod --recursive +x tools/bin
ENV PATH=$PATH:$ANDROID_SDK_ROOT/cmdline-tools/tools/bin

RUN yes | sdkmanager --licenses >/dev/null
RUN yes | sdkmanager \
    "platform-tools" \
    "build-tools;30.0.3" \
    "platforms;android-29" \
    "platforms;android-31" \
    "platforms;android-33"
# Install NDK in a separate layer to decrease max layer size.
RUN yes | sdkmanager "ndk;21.4.7075529"
ENV ANDROID_NDK_ROOT=$ANDROID_SDK_ROOT/ndk/21.4.7075529
ENV ANDROID_NDK_HOME=$ANDROID_NDK_ROOT

ENV HOME=/image-workdir
WORKDIR $HOME

ENV PUB_CACHE=$HOME/.cache/.pub-cache
ENV PATH=$PATH:$HOME/flutter/bin:$HOME/flutter/bin/cache/dart-sdk/bin:$PUB_CACHE/bin
RUN git clone --branch "3.7.6" --single-branch --depth 1 https://github.com/flutter/flutter.git $HOME/flutter
RUN git config --global --add safe.directory $HOME/flutter
RUN flutter config --no-analytics && dart --disable-analytics
RUN flutter doctor -v
RUN dart pub global activate protoc_plugin ^20.0.1 && chmod --recursive a=u $PUB_CACHE

RUN mkdir -p $HOME/.cache/ && chmod 777 $HOME/.cache/
RUN mkdir -p $HOME/.cache/.gradle && chmod 777 $HOME/.cache/.gradle
ENV GRADLE_USER_HOME=$HOME/.cache/.gradle
ENV ANDROID_SDK_HOME=$HOME/.cache/.android

RUN mkdir $ANDROID_SDK_HOME && chmod 777 $HOME/.cache/.android && \
    PASSWD=$(pwgen -s -y 20 1) && keytool -genkey -v -keystore $ANDROID_SDK_HOME/debug.keystore \
    -storepass $PASSWD -alias androiddebugkey -keypass $PASSWD -dname "CN=Android Debug,O=Android,C=US"

RUN chmod 777 $HOME

RUN mkdir -p /mnt/cache && chmod 777 /mnt/cache
RUN mkdir -p /mnt/project/flutter/build && chmod 777 /mnt/project/flutter/build
RUN mkdir -p $HOME/.cache/bazelisk && chmod 777 $HOME/.cache/bazelisk
