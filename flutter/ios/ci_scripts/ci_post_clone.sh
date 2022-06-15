#!/bin/sh

echo "Navigate from ($PWD) to ($CI_WORKSPACE)"
cd "$CI_WORKSPACE" || exit 1

echo "Install dependencies"
HOMEBREW_NO_AUTO_UPDATE=1 brew install bazelisk protobuf cocoapods
python3 -m pip install --user numpy absl-py

echo "Install Flutter"
mkdir -p ~/tools && git clone --branch 2.10.5 --depth 1 https://github.com/flutter/flutter.git ~/tools/flutter
export PATH="$PATH:$HOME/tools/flutter/bin:$HOME/.pub-cache/bin"
dart pub global activate protoc_plugin

echo "Build app"
make
cd "$CI_WORKSPACE"/flutter || exit 1 && flutter precache --ios
cd "$CI_WORKSPACE"/flutter/ios || exit 1 && pod install

exit 0