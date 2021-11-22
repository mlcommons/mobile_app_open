
This file describes how to build the app for iOS.

# Contents

* [Setting up the environment](#setting-up-the-environment)
* [Build and run](#build-and-run)
* [Changing application icon](#changing-application-icon)
* [Running instrumented test](#running-instrumented-test)
* [Formatting the code](#formatting-the-code)

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

# Build and run

[comment]: # (Don't remove spaces at the end of lines, they force line breaks)
* build backend library: `make`
* build and launch the app: `flutter run`
    * If you want to run the app on a real device, you have to change settings in XCode.  
Open `Runner` settings → `Signings & Capabilities`.  
Make sure checkbox "Automatically manage signing" is checked.
Set `Team` to your personal team.  
Set `Bundle identifier` to some value, that is unique for you (for example, you can add your name to it).
    * If you want to run the app in the simulator, make sure that the simulator is running.  
You can launch the simulator with the following command: `open -a Simulator`  
Or you can choose desired model of an iPhone in XCode and launch the app from XCode.
Simulator will be opened automatically by XCode. 

# Changing application icon

After building application, if you would like to set new application icon use following actions:

* Go to `pubspec.yaml` file in root folder
* Find `flutter_icons` configuration and change value of option `image_path` to the path of desirable application icon
* Use `flutter pub run flutter_launcher_icons:main` command to set the icon

Please note that iOS icons should not have any transparency. See more guidelines [here](https://developer.apple.com/design/human-interface-guidelines/ios/icons-and-images/app-icon/).

# Running instrumented test

* Run simulator `open -a Simulator` or connect physical device
* Define device Id `flutter devices`:
```bash
2 connected devices:

iPhone 12 Pro Max (mobile) • XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX • ios            • com.apple.CoreSimulator.SimRuntime.iOS-14-4 (simulator)
Chrome (web)               • chrome                               • web-javascript • Google Chrome 89.0.4389.128
```
* Run test:
```bash
flutter drive \
  --driver=test_driver/integration_test.dart \
  --target=integration_test/first_test.dart \
  -d "iPhone 12 Pro Max"
```
or
```bash
flutter drive \
  --driver=test_driver/integration_test.dart \
  --target=integration_test/first_test.dart \
  -d XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
```

# Formatting the code

In order to automatically format your files
you must have `clang-format` and `buildifier` in addition to build dependencies.

* clang-format can be installed via `brew install clang-format`,
* buildifier can be installed via `brew install buildifier`

Run `make format` to fix the formatting.  
This will fix C++, bazel and Dart files.  
Alternatively you can use make targets `format-clang`, `format-bazel`, `format-dart`
to format only certain kinds of files.
