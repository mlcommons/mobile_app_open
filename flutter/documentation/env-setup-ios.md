
This file describes environment setup for iOS builds on macOS.

# Contents

* [Setting up the environment](#setting-up-the-environment)
* [Tested anvironment](#tested-anvironment)
* [Changing application icon](#changing-application-icon)

# Setting up the environment

* Install XCode via App Store.  
If your Mac uses ARM CPU (MacBook Pro M1, etc.), you may have to enable Rosetta for XCode
if you want to run the app in iOS simulator because Flutter (as of v2.5.2) 
[does not yet support `arm64` iOS simulators](https://flutter.dev/docs/development/add-to-app/ios/project-setup#apple-silicon-arm64-macs). 
Otherwise, you can get errors about missing pods 
(missing includes from pods).
* Install dependencies:
    * `brew install flutter`
    * `brew install bazel`
    * `brew install protobuf`
    * `sudo gem install cocoapods`
* Install python dependencies: `python3 -m pip install --user numpy absl-py`
* Enable protobuf plugin: `dart pub global activate protoc_plugin`
* Add `$HOME/.pub-cache/bin` directory to PATH (the command bellow is for ZSH, adjust your `rc` file if you use something else):
```bash
echo export PATH="$PATH:$HOME/.pub-cache/bin" >>~/.zshrc
```
* Go to `ios` directory and install pods: `pod install`

# Tested anvironment

The app was built and tested successfully in this environment:

```
macOS Big Sur 11.6
Xcode: 12.5.1 (18212)

$ flutter --version
Flutter 2.5.2 • channel stable • https://github.com/flutter/flutter.git
Framework • revision 3595343e20 (2 days ago) • 2021-09-30 12:58:18 -0700
Engine • revision 6ac856380f
Tools • Dart 2.14.3

$ bazel --version
bazel 4.2.1-homebrew

$ protoc --version
libprotoc 3.17.3

$ pod --version
1.11.2

$ python3 --version
Python 3.9.7

```
Note: the current version may crash with EXC_RESOURCE if built with XCode 13.0
(see issue [#303](https://github.com/mlcommons/mobile_app_flutter/issues/303))

# Changing application icon

[comment]: # (TODO move this somewhere?)

After building application, if you would like to set new application icon use following actions:

* Go to `pubspec.yaml` file in root folder
* Find `flutter_icons` configuration and change value of option `image_path` to the path of desirable application icon
* Use `flutter pub run flutter_launcher_icons:main` command to set the icon

Please note that iOS icons should not have any transparency. See more guidelines [here](https://developer.apple.com/design/human-interface-guidelines/ios/icons-and-images/app-icon/).

# Formatting

[comment]: # (TODO add info about installing other tools)

In order to automatically format your files
you must have `clang-format` and `buildifier` in addition to build dependencies.

* `brew install clang-format`,
* `brew install buildifier`
