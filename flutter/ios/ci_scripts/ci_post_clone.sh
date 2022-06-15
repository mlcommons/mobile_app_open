#!/bin/sh

cd ../../.. || exit 1
echo "Current working directory is $PWD"
export ROOT_DIR=$PWD

# check if we are in the root directory of the repo, where the WORKSPACE file exists
test -f WORKSPACE || exit 1

echo "Install dependencies"
export HOMEBREW_NO_AUTO_UPDATE=1
brew extract --version=1.11.0 bazelisk homebrew/cask && brew install bazelisk@1.11.0
brew extract --version=3.19.4 protobuf homebrew/cask && brew install protobuf@3.19.4
brew extract --version=1.11.3 cocoapods homebrew/cask && brew install cocoapods@1.11.3

python3 -m pip install --user \
  numpy==1.21.5 \
  absl-py==1.0.0

echo "Install Flutter"
mkdir -p ~/tools && git clone --branch 2.10.5 --depth 1 https://github.com/flutter/flutter.git ~/tools/flutter
export PATH="$PATH:$HOME/tools/flutter/bin:$HOME/.pub-cache/bin"
dart pub global activate protoc_plugin

echo "Build app"
make
cd "$ROOT_DIR"/flutter || exit 1 && flutter precache --ios
cd "$ROOT_DIR"/flutter/ios || exit 1 && pod install

exit 0