#!/bin/sh

# This script runs after the repo is cloned and before any action execution.
# Environment variables with `CI_` prefix are set by Xcode Cloud (see https://developer.apple.com/documentation/xcode/environment-variable-reference)
# To avoid collision with other environment variables, we use `MC_` (MLCommons) prefix for our own variables.
# This script can also be run on the local Mac machine for testing and debugging.
# We use [ "$CI" = "TRUE" ] to test if the script is run on local machine or Xcode Cloud.

# abort if an error occurred
set -e

export MC_LOG_PREFIX="[mobile_app_open]"
cd ../../../

echo "$MC_LOG_PREFIX Current working directory is $PWD"
# check if we are in the root directory of the repo, where the WORKSPACE file exists
test -f WORKSPACE
export MC_ROOT_DIR=$PWD

if [ "$CI" = "TRUE" ]; then
  echo "$MC_LOG_PREFIX Running on CI machine"
  # Data in CI_DERIVED_DATA_PATH is persistent between builds.
  export MC_BUILD_HOME=$CI_DERIVED_DATA_PATH
else
  echo "$MC_LOG_PREFIX Running on local machine"
  export MC_BUILD_HOME="$HOME"/mobile_app_open_build
fi

brew --version
python3 --version

echo "$MC_LOG_PREFIX Install dependencies"
export HOMEBREW_NO_AUTO_UPDATE=1
export HOMEBREW_NO_INSTALL_CLEANUP=1
brew install bazelisk && bazel --version
brew install protobuf && protoc --version
brew install cocoapods && pod --version

pip3 install --upgrade pip
pip3 install \
  numpy==1.21.5 \
  absl-py==1.0.0

echo "$MC_LOG_PREFIX Install Flutter"
export MC_FLUTTER_HOME="$MC_BUILD_HOME"/flutter
export PUB_CACHE=$MC_BUILD_HOME/.pub-cache

mkdir -p "$MC_BUILD_HOME"
test ! -d "$MC_FLUTTER_HOME" && git clone --branch 2.10.5 --depth 1 https://github.com/flutter/flutter.git "$MC_FLUTTER_HOME"
export PATH="$PATH:$MC_FLUTTER_HOME/bin:$PUB_CACHE/bin"
flutter config --no-analytics && dart --disable-analytics
dart pub global activate protoc_plugin

echo "$MC_LOG_PREFIX Build app"
export BAZEL_OUTPUT_ROOT_ARG=--output_user_root=$MC_BUILD_HOME/bazel
cd "$MC_ROOT_DIR" && time make
cd "$MC_ROOT_DIR"/flutter && flutter precache --ios
cd "$MC_ROOT_DIR"/flutter/ios && pod install

if [ "$CI_XCODEBUILD_ACTION" = "build-for-testing" ]; then
  cd "$MC_ROOT_DIR"/flutter && flutter build ios --config-only integration_test/first_test.dart
fi

exit 0
