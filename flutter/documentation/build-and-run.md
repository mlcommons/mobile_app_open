# Build and run

This file describes how to build and run the Flutter app for different platforms.

## Contents

* [Overview](#overview)
* [Common info](#common-info)
* [Android](#android)
* [iOS](#ios)
* [Windows](#windows)
* [Running tests](#running-tests)

## Overview

[comment]: # (Don't remove spaces at the end of lines, they force line breaks)

All make targets referenced here are defined for the root of the repository.
Flutter/Dart commands, however, are only valid when you are in the `/flutter` directory of the repo.

The Flutter app uses 2 different build systems:
Bazel for backends (and for glue code between the app and backends)
and the Flutter itself.  
Build commands are mostly managed my `make`.

If you want to build the app, you first need to build native libs,
then prepare files that Flutter needs,
and then run the build command for Flutter.

## Common info

The root makefile defines a convenient `flutter` make target, that prepares native libs
and all miscellaneous files for Flutter but doesn't build actual application.  
After you run `make flutter`, you can use any Flutter tools to build the app with parameters you need (release, debug, test, etc.).

The easiest way to just run the app is to run flutter via console:

```bash
flutter run
```

If you want to run or debug the Flutter app for any platform using graphical user interface,
you can use [VS Code with Flutter extension](https://docs.flutter.dev/get-started/editor?tab=vscode).

If you want to test something without spending a lot of time on the benchmark,
you can use flag `--dart-define=fast-mode=true` to speed up the benchmark.
You should not evaluate performance when using this flag.

Add `WITH_<VENDOR>=1` to make commands to build the the app with backends.
For example:

```bash
make WITH_QTI=1 WITH_SAMSUNG=1 WITH_PIXEL=1 WITH_MEDIATEK=1 docker/flutter/android/apk
```

Some of the backends have additional requirements. See command output for details.

The app is built with an unofficial UI (different color and text) by default.
To build with an official UI, you need to set

* the environment variable `OFFICIAL_BUILD=true` if `make` is used (e.g. `OFFICIAL_BUILD=true make flutter/android/apk`), or
* the argument `--dart-define=official-build=true` if `flutter` is used (e.g. `flutter build apk --dart-define=official-build=true`).

## Android

[comment]: # (Don't remove spaces at the end of lines, they force line breaks)
[comment]: # (TODO add information about using Android emulators)

On Linux command `make flutter` builds native libs automatically.  
See [Android environment setup guide](./env-setup-android.md#setting-up-the-environment-on-ubuntu).

On macOS `make flutter` will try to build native libs for iOS. Run `make flutter/android` to build android libs on macOS.  
Unfortunately, there is no complete environment setup guide for now.
See [macOS guide](./env-setup-ios.md#setting-up-the-environment) for general setup and [Ubuntu guide](./env-setup-android.md#setting-up-the-environment-on-ubuntu) for android-specific parts.

On Windows building libs for Android is not available.  
You can copy `.so` files from some Linux or macOS system, then run `make flutter/prepare`, and then run Flutter commands locally.  
For example, it's relatively convenient to use WSL to build native libs.
See [Windows environment setup guide](./env-setup-windows.md#setting-up-the-environment) to set up your system to run Flutter.

Run `make flutter/android/apk` to build APK.

You can build the app using docker.  
Run `make docker/flutter/android/apk` to build release APK.  
Run `make docker/flutter/android/libs` to build just `.so` libs. This command is helpful if you want to build Android version of the app on Windows.

if the build fails with `java.io.IOException: Input/output error`, remove file gradle-wrapper.jar:

```bash
rm -f flutter/android/gradle/wrapper/gradle-wrapper.jar
```

## iOS

[comment]: # (Don't remove spaces at the end of lines, they force line breaks)

Building for iOS is available only on macOS.

See [iOS environment setup guide](./env-setup-ios.md#setting-up-the-environment).

If you want to run the app on a real device, you have to change settings in XCode:

1. Open `/flutter/ios/Runner.xcworkspace`
2. Open `Runner` settings → `Signings & Capabilities`.  
You probably need the Debug mode tab, but you can also change values in other tabs if you need.
3. Make sure checkbox `Automatically manage signing` is checked.
4. Set `Team` to your personal team.
5. Set `Bundle identifier` to some globally unique value (for example, you can add your name to it).

If you want to run the app in iPhone Simulator, make sure that the Simulator is running.  
To launch Simulator for iPhone 12 Pro you can use the following command: `xcrun simctl bootstatus "iPhone 12 Pro" -b`.  
You can find available devices using `xcrun simctl list`.

You may want to use XCode GUI to launch the app, if you are using a Simulator, because for some reason logs from native code are incomplete in console.  
Also Flutter doesn't support running the app on a remote device, without wired connection, but you can still use XCode GUI for this.  
Remember that XCode is used for the final build of the app, but it doesn't update any Flutter-related info.
If you change Flutter sources or configs and then press the `Run` button in XCode, you will build outdated version of the app.  
Run any of the following commands to update Flutter files in XCode project:

* `flutter run` (make sure that you select iOS device during build)
* `flutter build ios`
* `flutter build ipa`

Run `flutter build ipa` to create redistributable archive.
Note that you will need to sign the app using external tools, which are not described here.

## Windows

Building for Windows is available only on Windows.

See [Windows environment setup guide](./env-setup-windows.md#setting-up-the-environment).

On Windows it's pretty easy to use Visual Studio debugger for native libs.

1. Build native libs with debug symbols.  
Add `${debug_flags_windows}` to bazel build command in the corresponding make target and rerun `make flutter`.
2. Use `flutter run -d windows` or `flutter build windows --debug`
to update Visual Studio project after any changes in Flutter files or config.
3. Open VS solution file `/flutter/build/windows/mobile_app_flutter2.sln`,
right-click on `mobile_app_flutter2` project and click on `Set as Startup Project`.
4. Launch the app in debug mode from Visual Studio.

Visual Studio projects generated by Flutter don't have any backend files,
but you can drag and drop files from `cpp` folders to open them in Visual Studio and set breakpoints.

Run `flutter build windows` to build release folder with the app.
The release folder will be located in `flutter/build/windows/runner/Release/`.
To create redistributable copy the following files into the application directory:

```text
msvcp140.dll
vcruntime140.dll
vcruntime140_1.dll
msvcp140_codecvt_ids.dll
```

You can create `.zip` archive with the `Release` folder to redistribute the app.

## Running tests

You can run integration test on your device.
Use the following command to start the test:
`flutter drive --driver=test_driver/integration_test.dart --target=integration_test/first_test.dart`

Remember that running integration test changes Flutter configuration.
If you run this command and then try to launch the app from XCode or Visual Studio,
you will launch the test instead of the app.
Use `flutter run` or `flutter build` commands to switch to launching the app without tests.
