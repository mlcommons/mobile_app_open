# iOS environment setup guide

This file describes environment setup for iOS builds on macOS.

## Contents

* [Setting up the environment](#setting-up-the-environment)
* [Tested environment](#tested-environment)
* [Changing application icon](#changing-application-icon)

## Setting up the environment

* If your Mac uses ARM CPU (MacBook Pro M1, etc.), you have to install Rosetta: `sudo softwareupdate --install-rosetta --agree-to-license`
* Install XCode via App Store.  
  * You might need to enable Rosetta for XCode if you want to run the app in iOS simulator because Flutter (as of v2.5.2)
[does not yet support `arm64` iOS simulators](https://flutter.dev/docs/development/add-to-app/ios/project-setup#apple-silicon-arm64-macs).
Otherwise, you can get errors about missing pods
(missing includes from pods).
* Install dependencies:
  * `brew install bazelisk`
  * `brew install protobuf`
  * `sudo gem install cocoapods`
  * `python3 -m pip install --user numpy absl-py`
* Install Flutter:
  * Download flutter repo:

    ```bash
    mkdir -p ~/tools && git clone --branch 3.7.2 --depth 1 https://github.com/flutter/flutter.git ~/tools/flutter
    ```

  * Add flutter binary folders to path: `export PATH="$PATH:$HOME/tools/flutter/bin:$HOME/.pub-cache/bin"`  
    If you use zsh: `echo export PATH="\$PATH:\$HOME/tools/flutter/bin:\$HOME/.pub-cache/bin" >>~/.zshrc`
  * Enable protobuf plugin: `dart pub global activate protoc_plugin`
* Go to `ios` directory and install pods: `pod install`

## Tested environment

The app was built and tested successfully in this environment:

```shell
macOS 12.6
Xcode 14.0.1

$ flutter --version
Flutter 3.7.2 • channel stable • https://github.com/flutter/flutter.git
Framework • revision 32fb2f948e (4 weeks ago) • 2023-02-08 07:30:10 -0800
Engine • revision f40b73f8a4
Tools • Dart 2.19.2 • DevTools 2.20.1

$ bazel --version
bazel 5.0.0

$ protoc --version
libprotoc 3.21.6

$ pod --version
1.11.3

$ python3 --version
Python 3.9.14
```

Note: the current version may crash with EXC_RESOURCE if built with XCode 13.0
<!-- markdown-link-check-disable-next-line -->
(see issue [#303](https://github.com/mlcommons/mobile_app_flutter/issues/303))

## Changing application icon

[comment]: # (TODO move this somewhere?)

After building the application, if you would like to set a new application icon, use the following actions:

* Go to `pubspec.yaml` file in root folder
* Find `flutter_icons` configuration and change value of option `image_path` to the path of desirable application icon
* Use `flutter pub run flutter_launcher_icons:main` command to set the icon

Please note that iOS icons should not have any transparency. See more guidelines [here](https://developer.apple.com/design/human-interface-guidelines/ios/icons-and-images/app-icon/).

## Formatting

[comment]: # (TODO add info about installing other tools)

To automatically format your files you must have `clang-format` and `buildifier` in addition to build dependencies.

* `brew install clang-format`,
* `brew install buildifier`

Install GNU utils:

* `brew install findutils`
  * Add GNU utils to path: `PATH="/usr/local/opt/findutils/libexec/gnubin:$PATH"`  
    If you use zsh: `echo export PATH="/usr/local/opt/findutils/libexec/gnubin:\$PATH" >>~/.zshrc`
