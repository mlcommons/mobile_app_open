#!/bin/sh

# abort if an error occurred
set -e

export LOG_PREFIX="[mobile_app_open]"
cd ../../../
echo "$LOG_PREFIX Current working directory is $PWD"
# check if we are in the root directory of the repo, where the WORKSPACE file exists
test -f WORKSPACE
export ROOT_DIR=$PWD

brew --version
python3 --version

echo "$LOG_PREFIX Install dependencies"
export HOMEBREW_NO_AUTO_UPDATE=1
export HOMEBREW_NO_INSTALL_CLEANUP=1
brew install bazelisk
bazel --version
brew install protobuf
protoc --version
brew install cocoapods
pod --version

pip3 install --upgrade pip
pip3 install \
  numpy==1.21.5 \
  absl-py==1.0.0

echo "$LOG_PREFIX Install Flutter"
export BUILD_HOME="$HOME"/build
export FLUTTER_HOME="$BUILD_HOME"/flutter
mkdir -p "$BUILD_HOME"
test ! -d "$FLUTTER_HOME" && git clone --branch 2.10.5 --depth 1 https://github.com/flutter/flutter.git "$FLUTTER_HOME"
export PATH="$PATH:$FLUTTER_HOME/bin:$HOME/.pub-cache/bin"
dart pub global activate protoc_plugin

echo "$LOG_PREFIX Build app"
cd "$ROOT_DIR" && make
cd "$ROOT_DIR"/flutter && flutter precache --ios
cd "$ROOT_DIR"/flutter/ios && pod install
cd "$ROOT_DIR" && make flutter/test

exit 0