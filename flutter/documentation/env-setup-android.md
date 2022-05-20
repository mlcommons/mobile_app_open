# Android environment setup guide

This file describes how to setup the environment for Android builds on Ubuntu.

Android native libs can not be built on Windows.
If you use Windows as your primary build system you will have to use a helper linux or macos system to build libs.

## Contents

* [Setting up bazel on Ubuntu](#setting-up-bazel-on-ubuntu)
* [Setting up flutter on Ubuntu](#setting-up-flutter-on-ubuntu)
* [Setting up flutter on Windows](#setting-up-flutter-on-windows)

[comment]: # (Don't remove spaces at the end of lines, they force line breaks)

## Setting up bazel on Ubuntu

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
  * Install dependencies via sdkmanager, accept licenses for dependencies:

    ```bash
    sudo $ANDROID_HOME/cmdline-tools/tools/bin/sdkmanager \
      "ndk;21.4.7075529"
    ```

  * Set ANDROID_NDK_HOME: `export ANDROID_NDK_HOME=$ANDROID_HOME/ndk/21.4.7075529`  
  If you use bash: `echo export ANDROID_NDK_HOME=\$ANDROID_HOME/ndk/21.4.7075529 >>~/.bashrc`

## Setting up flutter on Ubuntu

If you use WSL to build native libs for Windows, you don't need these steps in WSL.

Complete all the steps for bazel, and then install few more dependencies:

* Set up Flutter
  * Install Flutter:

    ```bash
    mkdir -p ~/tools && cd ~/tools && curl https://storage.googleapis.com/flutter_infra_release/releases/stable/linux/flutter_linux_2.10.5-stable.tar.xz | tar Jxf -
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

    You will need to restart your WSL instance to apply changes.  
    Run `wsl --shutdown` in Windows, and then reopen WSL.

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

* Enable protobuf plugin: `dart pub global activate protoc_plugin`
* Add Dart pub bin folder to PATH: `export PATH=$PATH:~/.pub-cache/bin`  
If you use bash: `echo export PATH=\$PATH:~/.pub-cache/bin >>~/.bashrc`

## Setting up Flutter on Windows

Bazel can't build native Android libs on Windows.
But you can build them in a VM and then use Flutter to build APK or directly launch the app on an Android phone.

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
