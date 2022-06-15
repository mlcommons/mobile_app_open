#!/bin/sh

# abort if an error occurred
set -e

export LOG_PREFIX="[mobile_app_open]"
cd ../../../
echo "$LOG_PREFIX Current working directory is $PWD"
# check if we are in the root directory of the repo, where the WORKSPACE file exists
test -f WORKSPACE
export ROOT_DIR=$PWD

echo "$LOG_PREFIX Install dependencies"
export HOMEBREW_NO_AUTO_UPDATE=1
brew extract --version=1.11.0 bazelisk homebrew/cask && brew install bazelisk@1.11.0
brew extract --version=3.19.4 protobuf homebrew/cask && brew install protobuf@3.19.4
brew extract --version=1.11.3 cocoapods homebrew/cask && brew install cocoapods@1.11.3

python3.9 --version
python3.9 -m pip install \
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

exit 0