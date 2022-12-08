#!/bin/sh

# This script runs after the repo is cloned and before any action execution in Xcode Cloud
# Environment variables with `CI_` prefix are set by Xcode Cloud
# (see https://developer.apple.com/documentation/xcode/environment-variable-reference)
# Environment variables with `GITHUB_` prefix are set by GitHub Actions
# (see https://docs.github.com/en/actions/learn-github-actions/environment-variables#default-environment-variables)
# To avoid collision with other environment variables, we use `MC_` (MLCommons) prefix for our own variables.
# This script can also be run on the local Mac machine for testing and debugging.

# abort if an error occurred, show cmd executed
set -ex

export MC_LOG_PREFIX="[mobile_app_open]"
cd ../../../

echo "$MC_LOG_PREFIX START ci_post_clone"
echo "$MC_LOG_PREFIX Current working directory is $PWD"
# check if we are in the root directory of the repo, where the WORKSPACE file exists
test -f WORKSPACE
export MC_REPO_HOME=$PWD

XCODE_CLOUD="xcode-cloud"
GITHUB_ACTIONS="github-actions"
LOCAL="local"
runner=""

if [ -n "$CI_DERIVED_DATA_PATH" ]; then
  echo "$MC_LOG_PREFIX Running on XCode Cloud"
  export MC_BUILD_HOME=$CI_DERIVED_DATA_PATH/mobile_app_open_build
  CACHE_BUCKET=xcodecloud-bazel-ios---mobile-app-build-290400
  # Required that a secret env existed in the XCode Cloud workflow
  # with key=GC_CREDS and value=<output of cmd `base64 mobile-app-build-290400-1b26aafa8afd.json`>
  GC_CREDS_FILE=$CI_WORKSPACE/mobile-app-build-290400-1b26aafa8afd.json
  echo "$GC_CREDS" | base64 --decode >"$GC_CREDS_FILE"
  export BAZEL_CACHE_ARG="--remote_cache=https://storage.googleapis.com/$CACHE_BUCKET --google_credentials=$GC_CREDS_FILE"
  runner=$XCODE_CLOUD
elif [ -n "$GITHUB_WORKSPACE" ]; then
  echo "$MC_LOG_PREFIX Running on GitHub Actions"
  export MC_BUILD_HOME=$GITHUB_WORKSPACE/mobile_app_open_build
  export BAZEL_CACHE_ARG="--disk_cache=$BAZEL_CACHE_PATH"
  runner=$GITHUB_ACTIONS
else
  echo "$MC_LOG_PREFIX Running on local machine"
  export MC_BUILD_HOME=$HOME/mobile_app_open_build
  runner=$LOCAL
fi

echo "$MC_LOG_PREFIX MC_BUILD_HOME is ${MC_BUILD_HOME}"
echo "$MC_LOG_PREFIX runner is ${runner}"

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
  numpy==1.23.4 \
  absl-py==1.3.0

if [ $runner = $GITHUB_ACTIONS ]; then
  echo "$MC_LOG_PREFIX ========== Install Android SDK =========="
  export ANDROID_HOME=$HOME/Library/Android/sdk
  mkdir -p $ANDROID_HOME/cmdline-tools
  # sdkmanager expects to be placed into `$ANDROID_HOME/cmdline-tools/tools`
  curl -L https://dl.google.com/android/repository/commandlinetools-mac-7583922_latest.zip | jar x &&
    mv cmdline-tools $ANDROID_HOME/cmdline-tools/tools &&
    chmod -R +x $ANDROID_HOME/cmdline-tools/tools/bin
  export PATH=$PATH:$ANDROID_HOME/cmdline-tools/tools/bin
  yes | sdkmanager --licenses >/dev/null
  yes | sdkmanager \
    "platform-tools" \
    "build-tools;30.0.3" \
    "platforms;android-29" \
    "platforms;android-31" \
    "ndk;21.4.7075529"
  export ANDROID_NDK_HOME=$ANDROID_HOME/ndk/21.4.7075529
fi

echo "$MC_LOG_PREFIX ========== Install Flutter =========="
export MC_FLUTTER_HOME=$MC_BUILD_HOME/flutter
export PUB_CACHE=$MC_BUILD_HOME/.pub-cache

mkdir -p "$MC_BUILD_HOME"
test ! -d "$MC_FLUTTER_HOME" && git clone --branch 3.3.5 --depth 1 https://github.com/flutter/flutter.git "$MC_FLUTTER_HOME"
export PATH="$PATH:$MC_FLUTTER_HOME/bin:$PUB_CACHE/bin"
echo "$MC_LOG_PREFIX flutter version:" && flutter --version
flutter config --no-analytics && dart --disable-analytics
dart pub global activate protoc_plugin
cd "$MC_REPO_HOME"/flutter && flutter precache --ios

echo "$MC_LOG_PREFIX ========== Build app =========="
export BAZEL_OUTPUT_ROOT_ARG=--output_user_root=$MC_BUILD_HOME/bazel
export WITH_TFLITE=0
export WITH_APPLE=1

echo "$MC_LOG_PREFIX Build backend and Flutter packages"
# Remember to update the next line if make commands are changed.
cd "$MC_REPO_HOME" && time make flutter/prepare && time make flutter/ios

if [ $runner = $XCODE_CLOUD ]; then
  if [ "$CI_XCODEBUILD_ACTION" = "build-for-testing" ]; then
    cd "$MC_REPO_HOME"/flutter && flutter build ios --config-only integration_test/first_test.dart
  else
    cd "$MC_REPO_HOME"/flutter && flutter build ios --config-only --dart-define=official-build="$OFFICIAL_BUILD" --build-number "$CI_BUILD_NUMBER"
  fi
fi

cd "$MC_REPO_HOME"/flutter/ios && pod install

if [ $runner = $GITHUB_ACTIONS ]; then
  open -a Simulator
  cd "$MC_REPO_HOME" && make flutter/test
fi

echo "$MC_LOG_PREFIX END ci_post_clone"
exit 0
