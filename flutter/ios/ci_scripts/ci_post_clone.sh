#!/bin/sh

# This script runs after the repo is cloned and before any action execution.
# Environment variables with `CI_` prefix are set by Xcode Cloud
# (see https://developer.apple.com/documentation/xcode/environment-variable-reference)
# To avoid collision with other environment variables, we use `MC_` (MLCommons) prefix for our own variables.
# This script can also be run on the local Mac machine for testing and debugging.
# We use [ "$CI" = "TRUE" ] to test if the script is run on local machine or Xcode Cloud.

# abort if an error occurred
set -e

export MC_LOG_PREFIX="[mobile_app_open]"
cd ../../../

echo "$MC_LOG_PREFIX START ci_post_clone"
echo "$MC_LOG_PREFIX Current working directory is $PWD"
# check if we are in the root directory of the repo, where the WORKSPACE file exists
test -f WORKSPACE
export MC_REPO_HOME=$PWD

if [ "$CI" = "TRUE" ]; then
  echo "$MC_LOG_PREFIX Running on CI machine"
  export MC_BUILD_HOME=$CI_DERIVED_DATA_PATH/mobile_app_open_build
  CACHE_BUCKET=xcodecloud-bazel-ios---mobile-app-build-290400
  GC_CREDS_FILE=$CI_WORKSPACE/mobile-app-build-290400-1b26aafa8afd.json
  echo "$GC_CREDS" | base64 --decode > "$GC_CREDS_FILE"
  export BAZEL_CACHE_ARG="--remote_cache=https://storage.googleapis.com/$CACHE_BUCKET --google_credentials=$GC_CREDS_FILE"
else
  echo "$MC_LOG_PREFIX Running on local machine"
  export MC_BUILD_HOME=$HOME/mobile_app_open_build
fi
echo "$MC_LOG_PREFIX MC_BUILD_HOME is ${MC_BUILD_HOME}"

echo "$MC_LOG_PREFIX ========== Install dependencies =========="

brew update
echo "$MC_LOG_PREFIX brew version:" && brew config

# `brew install` failed often due to connection issue so we try several times
brew install bazelisk || brew install bazelisk || brew install bazelisk
echo "$MC_LOG_PREFIX bazel version:" && bazel --version

brew install protobuf || brew install protobuf || brew install protobuf
echo "$MC_LOG_PREFIX protobuf version:" && protoc --version

brew install cocoapods || brew install cocoapods || brew install cocoapods
echo "$MC_LOG_PREFIX cocoapods version:" && pod --version

echo "$MC_LOG_PREFIX python version:" && python3 --version
pip3 install --upgrade pip
pip3 install \
  numpy==1.21.5 \
  absl-py==1.0.0

echo "$MC_LOG_PREFIX ========== Install Flutter =========="
export MC_FLUTTER_HOME=$MC_BUILD_HOME/flutter
export PUB_CACHE=$MC_BUILD_HOME/.pub-cache

mkdir -p "$MC_BUILD_HOME"
test ! -d "$MC_FLUTTER_HOME" && git clone --branch 2.10.5 --depth 1 https://github.com/flutter/flutter.git "$MC_FLUTTER_HOME"
export PATH="$PATH:$MC_FLUTTER_HOME/bin:$PUB_CACHE/bin"
echo "$MC_LOG_PREFIX flutter version:" && flutter --version
flutter config --no-analytics && dart --disable-analytics
dart pub global activate protoc_plugin
cd "$MC_REPO_HOME"/flutter && flutter precache --ios

echo "$MC_LOG_PREFIX ========== Build app =========="
export BAZEL_OUTPUT_ROOT_ARG=--output_user_root=$MC_BUILD_HOME/bazel

echo "$MC_LOG_PREFIX Build backend and Flutter packages"
# Remember to update the next line if make commands are changed.
cd "$MC_REPO_HOME" && time make flutter/prepare && time make flutter/ios

if [ "$CI_XCODEBUILD_ACTION" = "build-for-testing" ]; then
  cd "$MC_REPO_HOME"/flutter && flutter build ios --config-only integration_test/first_test.dart
else
  cd "$MC_REPO_HOME"/flutter && flutter build ios --config-only --dart-define=official-build="$OFFICIAL_BUILD"
fi

cd "$MC_REPO_HOME"/flutter/ios && pod install

echo "$MC_LOG_PREFIX END ci_post_clone"
exit 0
