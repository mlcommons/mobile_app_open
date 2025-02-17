FROM ubuntu:20.04

# Without DEBIAN_FRONTEND=noninteractive arg apt-get waits for user input.
#   Docker desktop shows all previously defined args for each of the commands,
#   which makes reading layer info harder, so we set this env for for individual commands only.
# JDK package downloads ~500 MB from slow mirrors, which can take a lot of time,
#   so a separate layer for it makes image rebuild faster in case we change any other dependencies.
RUN apt-get update >/dev/null && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    openjdk-17-jdk-headless && \
    rm -rf /var/lib/apt/lists/*
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
    python3-dev \
    python3-pip && \
    rm -rf /var/lib/apt/lists/*

RUN ln -s /usr/bin/python3 /usr/bin/python
RUN python3 -m pip install numpy absl-py

RUN curl --proto '=https' -L https://github.com/bazelbuild/bazelisk/releases/download/v1.16.0/bazelisk-linux-amd64 -o /usr/local/bin/bazel && \
    chmod +x /usr/local/bin/bazel

ENV ANDROID_SDK_ROOT=/opt/android
ENV ANDROID_HOME=/opt/android
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
    "build-tools;34.0.0" \
    "platforms;android-29" \
    "platforms;android-31" \
    "platforms;android-33" \
    "platforms;android-34"
# Install NDK in a separate layer to decrease max layer size.
RUN yes | sdkmanager "ndk;25.2.9519653"
ENV ANDROID_NDK_ROOT=$ANDROID_SDK_ROOT/ndk/25.2.9519653
ENV ANDROID_NDK_HOME=$ANDROID_NDK_ROOT
ENV ANDROID_NDK_VERSION=25
ENV ANDROID_NDK_API_LEVEL=33
ENV HOME=/image-workdir
WORKDIR $HOME

ENV PUB_CACHE=$HOME/.cache/.pub-cache
ENV PATH=$PATH:$HOME/flutter/bin:$HOME/flutter/bin/cache/dart-sdk/bin:$PUB_CACHE/bin
RUN git clone --branch "3.19.6" --single-branch --depth 1 https://github.com/flutter/flutter.git $HOME/flutter
RUN git config --global --add safe.directory $HOME/flutter
RUN flutter config --no-analytics && dart --disable-analytics
RUN flutter doctor -v
RUN dart pub global activate protoc_plugin ^21.1.2 && chmod --recursive a=u $PUB_CACHE

RUN mkdir -p $HOME/.cache/.gradle
ENV GRADLE_USER_HOME=$HOME/.cache/.gradle
ENV ANDROID_SDK_HOME=$HOME/.cache/.android

RUN mkdir $ANDROID_SDK_HOME && \
    PASSWD=$(pwgen -s -y 20 1) && keytool -genkey -v -keyalg RSA -keystore $ANDROID_SDK_HOME/debug.keystore \
    -storepass $PASSWD -alias androiddebugkey -keypass $PASSWD -dname "CN=Android Debug,O=Android,C=US"

# Git repo will be mounted at '/image-workdir/project'
RUN git config --global --add safe.directory /image-workdir/project

# Add 32-bit support since the adb in our sdk's are 32-bit binaries
RUN dpkg --add-architecture i386
ARG DEBIAN_FRONTEND=noninteractive
ARG APT_KEY_DONT_WARN_ON_DANGEROUS_USAGE=DontWarn

# build-essential: for rest-kit(rest-kit needed for crad-repo)
# file : used by ndk to determine if host is 32 or 64 bit
# libtbb-dev: used for parallelization of postprocessing for pose-estimation
# libcanberra-gtk-module, libgtk2.0-dev: required by opencv runtime
# libstdc++6:i386: to add support for 32 bit binaries.
# libxtst6 : required to run ide(e.g intellij) inside docker
RUN apt-get update && apt-get install --no-install-recommends -y build-essential \
    cmake \
    curl \
    git \
    emacs \
    file \
    less \
    libcanberra-gtk-module \
    libgtk2.0-dev \
    libtbb-dev \
    libstdc++6:i386 \
    libxtst6

# pkg-config: required by opencv runtime
# software-properties-common: for curl
# xterm : to make resize available for avoiding line wrapping
# zip : required when sourcing snpe envsetup
RUN apt-get update && apt-get install --no-install-recommends -y mc \
    meld \
    pkg-config \
    python3-dev \
    software-properties-common \
    sudo \
    tmux \
    tree \
    unzip \
    vim \
    xterm \
    zip \
    ffmpeg \
    libjpeg-dev \
    zlib1g-dev && \
    apt-get clean autoclean

# Get cmake-3.19.3
# apt has 3.10.2 as latest version, so remove it
RUN apt-get remove --no-install-recommends -y cmake
RUN curl --proto '=https' -L https://github.com/Kitware/CMake/releases/download/v3.19.3/cmake-3.19.3-Linux-x86_64.sh -o /opt/cmake-3.19.3-Linux-x86_64.sh && \
    chmod +x /opt/cmake-3.19.3-Linux-x86_64.sh && mkdir -p /opt/cmake && \
    bash /opt/cmake-3.19.3-Linux-x86_64.sh --skip-license --prefix=/opt/cmake && \
    rm -rf /opt/cmake-3.19.3-Linux-x86_64.sh
# Add CMAKE into PATH
ENV PATH "/opt/cmake/bin:${PATH}"

# OpenCV
ENV CMAKE_TOOLCHAIN_FILE "${ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake"
ENV ANDROID_ABI "arm64-v8a"
ENV API_LEVEL "31"
ENV ANDROID_TOOLCHAIN_NAME "aarch64-linux-android-4.9"
ARG COMMON_CMAKE_OPTIONS="-DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=release \
                          -DBUILD_ZLIB=ON -DWITH_FFMPEG=ON -DBUILD_TESTS=OFF \
                          -DWITH_TBB=ON -DBUILD_PERF_TESTS=OFF -DWITH_IPP=OFF \
                          -DWITH_OPENEXR=ON -DWITH_JASPER=ON -WITH_PNG=ON -DBUILD_FAT_JAVA_LIB=OFF WITH_IMGCODEC=ON"
# Ref: https://docs.opencv.org/3.4.0/d7/d9f/tutorial_linux_install.html
RUN curl --proto '=https' -L https://github.com/opencv/opencv/archive/3.4.7/opencv-3.4.7.tar.gz -o /tmp/3.4.7.tar.gz && \
    tar -C /tmp -xvf /tmp/3.4.7.tar.gz  && \
    # First build for arm-android
    cd /tmp/opencv-3.4.7 && mkdir -p /opt/opencv-3.4.7_android/ release_android && cd release_android && \
    cmake -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE}" \
          -DANDROID_NDK="${ANDROID_NDK}" \
          -DANDROID_HOME="${ANDROID_HOME}" \
          -DANDROID_STL=c++_shared  \
          -DBUILD_ANDROID_PROJECTS=OFF \
          -DANDROID_NATIVE_API_LEVEL="${API_LEVEL}" \
          -DANDROID_ABI="${ANDROID_ABI}" \
          -DWITH_CUDA=ON -DWITH_OPENCL=ON \
          $COMMON_CMAKE_OPTIONS \
          -D WITH_ITT=OFF \
          -DCMAKE_INSTALL_PREFIX=/opt/opencv-3.4.7_android/ .. && \
    make -j16  && \
    sudo make -j16 install  && cp -rf ./3rdparty/ /opt/opencv-3.4.7_android/

# Set the variables to be used for actual app development/build
ENV ANDROID_SYSROOT "${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/sysroot"
ENV ANDROID_PLATFORM "${API_LEVEL}"
ENV ANDROID_PLATFORM_TOOLS "${ANDROID_HOME}/platform-tools"
ENV PATH "${ANDROID_PLATFORM_TOOLS}:${PATH}"
ENV LD_LIBRARY_PATH "/usr/local/lib/:${LD_LIBRARY_PATH}"