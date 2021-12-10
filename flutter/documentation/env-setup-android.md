
This file describes how to setup the environment for Android builds on Ubuntu.

# Contents

* [Setting up the environment on Ubuntu](#setting-up-the-environment-on-ubuntu)

# Setting up the environment on Ubuntu

[comment]: # (Don't remove spaces at the end of lines, they force line breaks)
* Install dependencies from apt:
  ```bash
  sudo apt install -y \
    apt-transport-https \
    curl \
    gnupg \
    make \
    python3 python3-pip \
    openjdk-11-jdk-headless \
    protobuf-compiler
  ```
* Install bazel using instructions from [bazel documentation](https://docs.bazel.build/versions/main/install-ubuntu.html):
  ```bash
  curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/bazel.gpg >/dev/null && \
      echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list && \
      sudo apt-get update && sudo apt-get install -y bazel=4.2.1
  ```
* Point `python` to `python3`: `sudo ln -s /usr/bin/python3 /usr/bin/python`
* Install python dependencies: `python3 -m pip install --user numpy absl-py`
* Install Android dependencies:
  * Set ANDROID_HOME: `export ANDROID_HOME=/opt/android`  
  If you use bash: `echo export ANDROID_HOME=/opt/android >>~/.bashrc`
  * Download Android SDK command line tools:
    ```bash
    sudo mkdir -p $ANDROID_HOME/cmdline-tools && \
      curl https://dl.google.com/android/repository/commandlinetools-linux-7583922_latest.zip | sudo busybox unzip -q -d $ANDROID_HOME/cmdline-tools - && \
      sudo mv $ANDROID_HOME/cmdline-tools/cmdline-tools $ANDROID_HOME/cmdline-tools/tools && \
      sudo chmod --recursive +x $ANDROID_HOME/cmdline-tools/tools/bin
    ```
  * Add command line tools bin folder to your PATH: `export PATH=$PATH:$ANDROID_HOME/cmdline-tools/tools/bin`  
  If you use bash: `echo export PATH=\$PATH:\$ANDROID_HOME/cmdline-tools/tools/bin >>~/.bashrc`
  * Accept Android licenses: `sdkmanager --licenses`
  * Install dependencies via sdkmanager, accept licenses for dependencies:
    ```bash
    sudo $ANDROID_HOME/cmdline-tools/tools/bin/sdkmanager \
      "tools" \
      "platform-tools" \
      "build-tools;29.0.2" \
      "build-tools;30.0.3" \
      "platforms;android-30" \
      "ndk;21.4.7075529"
    ```
    Build tools 29.0.2 are required by Flutter 2.5.3  
    Build tools 30 are required by bazel 4.2.1
  * Set ANDROID_NDK_HOME: `export ANDROID_NDK_HOME=$ANDROID_HOME/ndk/21.4.7075529`  
  If you use bash: `echo export ANDROID_NDK_HOME=\$ANDROID_HOME/ndk/21.4.7075529 >>~/.bashrc`
* Set up Flutter
  * Install Flutter:
    ```bash
    mkdir -p ~/tools && cd ~/tools && curl https://storage.googleapis.com/flutter_infra_release/releases/stable/linux/flutter_linux_2.5.3-stable.tar.xz | tar Jxf -
    ```
  * Add Flutter bin folder to PATH: `export PATH=$PATH:~/tools/flutter/bin`  
  If you use bash: `echo export PATH=\$PATH:~/tools/flutter/bin >>~/.bashrc`
  * If you run `flutter` or `dart` command in a WSL instance and see an error like `/usr/bin/env: ‘bash\r’: No such file or directory`, remove Windows PATH from WSL path:
    ```bash
    sudo tee /etc/wsl.conf <<EOF
    [interop]
    appendWindowsPath = false
    EOF
    ```
* Enable protobuf plugin: `dart pub global activate protoc_plugin`
* Add Dart pub bin folder to PATH: `export PATH=$PATH:~/.pub-cache/bin`  
If you use bash: `echo export PATH=\$PATH:~/.pub-cache/bin >>~/.bashrc`

[comment]: # (TODO add info about installing formatting tools)
