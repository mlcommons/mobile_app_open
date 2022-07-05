# Docker image name: mlcommons/mlperf_mobile
FROM ubuntu:bionic
LABEL maintainer="quic_mcharleb@quicinc.com"

# Update the apt configuration and set timezone first or image creation waits
# for selection of timezone
RUN apt-get update && apt-get upgrade -y && apt-get autoremove -y && \
    apt-get install -y --no-install-recommends tzdata

RUN apt-get update && apt-get upgrade -y && apt-get autoremove -y && \
    apt-get install -y --no-install-recommends \
       python3 python3-pip libpython3.6-dev python3.6-venv libgl1-mesa-glx libglib2.0 cython3 gcc make curl unzip libc++1-8 \
       git locales openssh-client ca-certificates tar gzip parallel \
       zip bzip2 gnupg wget python3-six python3-pip libncurses5 openjdk-11-jdk-headless clang-format-10 golang-1.13-go

RUN pip3 install pip==19.3.1 setuptools==31.0.1
RUN pip3 install tensorflow-cpu==1.15
RUN pip3 install Pillow opencv-python setuptools matplotlib tensorflow_hub tf-slim \
                 absl-py numpy

RUN ln -s /usr/bin/python3 /usr/bin/python
RUN apt-get clean

# Protoc 3.6.1
RUN curl -OL https://github.com/google/protobuf/releases/download/v3.6.1/protoc-3.6.1-linux-x86_64.zip && \
    unzip protoc-3.6.1-linux-x86_64.zip -d protoc3 && \
    mv protoc3/bin/* /usr/local/bin/ && \
    mv protoc3/include/* /usr/local/include/ && \
    rm -rf protoc-3.6.1-linux-x86_64.zip protoc3

# Install bazel
RUN echo "deb [arch=amd64] http://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list
RUN curl https://bazel.build/bazel-release.pub.gpg | apt-key add -
RUN apt-get update && \
    apt-get install -y --allow-unauthenticated bazel-3.7.2
RUN apt-get clean

# Set timezone to UTC by default
RUN ln -sf /usr/share/zoneinfo/Etc/UTC /etc/localtime

# Use unicode
RUN locale-gen en_US.UTF-8 || true
ENV LANG=en_US.UTF-8

ARG android_home=/opt/android/sdk

# Install Android SDK and NDK
RUN mkdir -p ${android_home} && \
    wget -O /tmp/sdk_tools.zip https://dl.google.com/android/repository/commandlinetools-linux-7583922_latest.zip && \
    unzip -q /tmp/sdk_tools.zip -d ${android_home}/cmdline-tools/ && \
    mv ${android_home}/cmdline-tools/cmdline-tools ${android_home}/cmdline-tools/tools && \
    rm /tmp/sdk_tools.zip

# Set environment variables
ENV ANDROID_HOME ${android_home}
ENV ANDROID_NDK_HOME ${android_ndk_home}
ENV ADB_INSTALL_TIMEOUT 120
ENV PATH=${ANDROID_HOME}/emulator:${ANDROID_HOME}/cmdline-tools/tools/bin:${ANDROID_HOME}/tools/bin:${ANDROID_HOME}/build-tools/29.0.3:${PATH}

RUN mkdir ~/.android && echo '### User Sources for Android SDK Manager' > ~/.android/repositories.cfg

# Update SDK manager and install system image, platform and build tools
RUN yes | sdkmanager --licenses && sdkmanager --update
RUN yes | sdkmanager \
    "tools" \
    "platform-tools" \
    "extras;android;m2repository" \
    "extras;google;m2repository" \
    "extras;google;google_play_services" \
    "build-tools;29.0.3"

ARG android_version=30
RUN sdkmanager "platforms;android-${android_version}" "cmake;3.6.4111459"

ARG ndk_version=android-ndk-r21e
ARG android_ndk_home=/opt/android/${ndk_version}

# Install the NDK
# Use wget instead of curl to avoid "Error in the HTTP2 framing layer"
RUN cd /tmp &&  wget -nv https://dl.google.com/android/repository/${ndk_version}-linux-x86_64.zip && \
    unzip -q /tmp/${ndk_version}-linux-x86_64.zip -d /opt/android && \
    rm /tmp/${ndk_version}-linux-x86_64.zip

ENV ANDROID_NDK_HOME ${android_ndk_home}

# Add Java format checker and buildifier
ENV GOBIN /opt/formatters/bin
RUN mkdir -p ${GOBIN} && curl --output `dirname ${GOBIN}`/google-java-format-1.9-all-deps.jar -L https://github.com/google/google-java-format/releases/download/google-java-format-1.9/google-java-format-1.9-all-deps.jar
RUN /usr/lib/go-1.13/bin/go get github.com/bazelbuild/buildtools/buildifier
ENV PATH=${PATH}:${GOBIN}

RUN mkdir -p /home/mlperf && chmod 777 /home/mlperf
ENV HOME /home/mlperf
ENV JAVA_HOME /usr/lib/jvm/java-11-openjdk-amd64