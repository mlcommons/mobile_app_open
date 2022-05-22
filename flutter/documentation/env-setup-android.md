# Android environment setup guide

This file describes how to setup the environment for Android builds on Ubuntu.

Android native libs can not be built on Windows.
If you use Windows as your primary build system you will have to use a helper linux or macos system to build libs.

## Contents

* [Setting up Bazel on Ubuntu](#setting-up-bazel-on-ubuntu)
* [Setting up Flutter on Ubuntu](#setting-up-flutter-on-ubuntu)
* [Setting up Flutter on Windows](#setting-up-flutter-on-windows)

[comment]: # (Don't remove spaces at the end of lines, they force line breaks)

## Setting up Bazel on Ubuntu

These steps are required to build native libs only.

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

* Install [bazel](https://bazel.build/install/ubuntu)
  * Install [bazelisk](https://bazel.build/install/bazelisk)

    ```bash
    curl -L https://github.com/bazelbuild/bazelisk/releases/download/v1.11.0/bazelisk-linux-amd64 -o /usr/local/bin/bazel && \
      chmod +x /usr/local/bin/bazel
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
  * Install dependencies via sdkmanager, accept licenses for dependencies:

    ```bash
    sudo $ANDROID_HOME/cmdline-tools/tools/bin/sdkmanager \
      "ndk;21.4.7075529"
    ```

  * Set ANDROID_NDK_HOME: `export ANDROID_NDK_HOME=$ANDROID_HOME/ndk/21.4.7075529`  
  If you use bash: `echo export ANDROID_NDK_HOME=\$ANDROID_HOME/ndk/21.4.7075529 >>~/.bashrc`

## Setting up Flutter on Ubuntu

These steps are required if you want to build and/or debug Flutter using Ubuntu.
If you use WSL to build native libs for Windows, you don't need these steps in WSL.

Flutter requires native libs so you must complete [Setting up Bazel on Ubuntu](#setting-up-bazel-on-ubuntu) first.

* Set up Flutter
  * Install Flutter:

    ```bash
    mkdir -p ~/tools && cd ~/tools && curl https://storage.googleapis.com/flutter_infra_release/releases/stable/linux/flutter_linux_2.10.5-stable.tar.xz | tar Jxf -
    ```

  * Add flutter binary folders to path: `export PATH=$PATH:~/tools/flutter/bin:~/.pub-cache/bin`  
    If you use bash: `echo export PATH=\$PATH:~/tools/flutter/bin:~/.pub-cache/bin >>~/.bashrc`
  * If you run `flutter` or `dart` command in a WSL instance and see an error like `/usr/bin/env: ‘bash\r’: No such file or directory`, remove Windows PATH from WSL path:

    ```bash
    sudo tee /etc/wsl.conf <<EOF
    [interop]
    appendWindowsPath = false
    EOF
    ```

    You will need to restart your WSL instance to apply changes.  
    Run `wsl --shutdown` in Windows, and then reopen WSL.
* Enable protobuf plugin: `dart pub global activate protoc_plugin`

* Install dependencies via sdkmanager, accept licenses for dependencies:

  ```bash
  sudo $ANDROID_HOME/cmdline-tools/tools/bin/sdkmanager \
    "platform-tools" \
    "build-tools;29.0.2" \
    "platforms;android-31"
  ```

  * `platform-tools` is required for `adb`
  * `build-tools;29.0.2` is required by flutter 2.x.  
  * `android-31` is required by flutter 2.10.5 

* Accept Android licenses: `sudo $ANDROID_HOME/cmdline-tools/tools/bin/sdkmanager --licenses`
  * Even after this command `flutter doctor` will tell you that licenses status is unknown.  
    You don't have to do anything with this
    but if you want to fix it, you also need to install android command line tools:

    ```bash
    sudo $ANDROID_HOME/cmdline-tools/tools/bin/sdkmanager "cmdline-tools;latest"
    ```

    With command line tools installed you can run `flutter doctor --android-licenses`

## Setting up Flutter on Windows

Bazel can't build native Android libs on Windows.
But you can build them elsewhere (in a VM, WSL, separate linux PC) and then build and/or debug Flutter on an Android Phone using Windows machine.

To set up your environment to build Flutter for Android:

* Follow steps from the [Windows environment setup guide](./env-setup-windows.md#setting-up-the-environment)
  * You can skip Visual Studio installation
  * You can skip bazel and python
* Install Android dependencies
  * Install [Android Studio](https://developer.android.com/studio/#downloads)
    * You don't need the whole Android Studio but it's the easiest way to install dependencies.
  * After installation open settings and go to `Appearance & Behavior` → `System Settings` → `Android SDK`  
  * Enable `Show package details` in the bottom right corner
  * Install the following items:
    * `SDK Platforms/Android 12.0 (S)/Android SDK Platform 31`
    * `SDK Tools/Android SDK Build-Tools/29.0.2`
    * `SDK Tools/Android SDK Command-line Tools/Android SDK Command-line Tools (latest)`
    * `SDK Tools/Android SDK Platform-Tools`

    You don't need other components which Android Studio might have preinstalled for you. You can safely remove them.

[comment]: # (TODO add info about installing formatting tools)
