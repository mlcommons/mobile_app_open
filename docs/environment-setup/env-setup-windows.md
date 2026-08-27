# Windows environment setup guide

This file describes how to build the app for Windows.

## Contents

* [Setting up the environment](#setting-up-the-environment)
* [Tested environment](#tested-environment)
* [Formatting](#formatting)

## Setting up the environment

[comment]: # (Don't remove spaces at the end of lines, they force line breaks)

* Install Visual Studio: <https://visualstudio.microsoft.com/vs/>  
Visual Studio Community will suffice. Visual Studio 2022 with MSVC toolset 14.38 or newer is required:
the pinned TensorFlow uses C11 `<stdatomic.h>` and `<threads.h>`.  
Install the `C++ ATL` component too; the `desktop_webview_auth` plugin needs `atlstr.h`.
* Install Chocolatey: <https://chocolatey.org>  
This is an optional step. You can download and install all of the dependencies from other sources.  
However, using Chocolatey greatly simplifies installation.
* Install dependencies:
  * `choco install -y git`
  * `choco install -y make`
  * `choco install -y bazelisk`  
  Bazelisk reads the `.bazelversion` file in the repo root and downloads the right Bazel automatically.
  * `choco install -y python3`
  * `choco install -y msys2`
  * `choco install -y protoc --version 23.2`
  * Install Flutter with the exact version pinned in `flutter/pubspec.yaml` (currently 3.44.4).  
  Chocolatey does not publish every Flutter release.
  Download the official Windows SDK zip from
  [the Flutter SDK archive](https://docs.flutter.dev/release/archive) instead
  and add its `bin` folder to `PATH`.
  * `dart pub global activate protoc_plugin ^25.0.0`
* Configure python
  * You must have command `python3` in your PATH.  
  Python installed via Chocolatey provides only `python.exe` file, so you will need to create `python3` yourself.  
  At the moment of writing this instruction the default path for python is `C:/Python39/`.
  You can make a copy of `python.exe` file in the same directory and name it `python3.exe`.
  * Disable python dummy files provided by Windows
    * Go to `Settings` → `Apps` → `Apps & features` → `App execution aliases`
    * Disable `App Installer python.exe` amd `App Installer python3.exe`
  * Set `PYTHON_BIN_PATH` env pointing to your actual python executable  
  Bazel is able to locate python but it resolves it with backslashes in path which causes errors.
  Use forward slashes to mitigate this: `PYTHON_BIN_PATH=C:/Python39/python.exe`
* Set python path env, paths must use forward slashes
* Install python dependencies: `python3 -m pip install --user numpy absl-py`
* Add MSYS2 bin folder to PATH: `C:/tools/msys64/usr/bin`
* Enable protobuf plugin: `dart pub global activate protoc_plugin ^25.0.0`
* Add dart pub cache bin folder to PATH: `%LOCALAPPDATA%/Pub/Cache/bin`
* Turn on the developer mode in Windows settings.
  * This option should be located in `Update & Security` → `For developers`.
  * Or you can open this page from command line: `start ms-settings:developers`

**Note**: Keep the bazel base path short, because some of the commands during tflite compilation
can become very long and throw some error. You can change the path by
using --output_base=<some\short\path>

**Note**: If you have a WSL distro installed on your PC, you may need to set `BAZEL_SH` environment variable.
Without it bazel could call `bash` provided by WSL instead of MSYS2's one.
Put path to the `bash` command from MSYS2 there.
If you installed MSYS2 via Chocolatey, the path would be `C:/tools/msys64/usr/bin/bash.exe`.

**Note**: Flutter may not work correctly if your temp directory is located on a RAM drive.
If you see error messages similar to
`Cannot resolve symbolic links, path = 'Z:\temp\pub_85cf1e20' (OS Error: Incorrect function., errno = 1)`,
move your temp directory location.  
You can do this for current terminal only, if you don't want to move it permanently:

```batch
:: for cmd
set "TEMP=%USERPROFILE%\AppData\Local\Temp"
set "TMP=%USERPROFILE%\AppData\Local\Temp"
```

```powershell
# for PowerShell
$env:TEMP=$env:USERPROFILE+'\AppData\Local\Temp'
$env:TMP=$env:USERPROFILE+'\AppData\Local\Temp'
```

## Tested environment

The GitHub Actions workflow [windows-build-test.yml](../../.github/workflows/windows-build-test.yml)
is a working reference for a complete Windows build, including the integration test and release packaging.
It runs on the `windows-2022` runner image with:

```shell
Windows Server 2022
Microsoft Visual Studio 2022 (MSVC toolset 14.44)

bazel 7.7.0 (via bazelisk, pinned by .bazelversion)
Flutter 3.44.4
libprotoc 23.2
Python 3.9
GNU Make 4.3
msys2 (runner-provided, C:\msys64)
```

## Formatting

You can use `make format` and `make lint` to format and check files.

You need to install dependencies for these commands to work:

* `choco install -y buildifier`
* clang-format:
  * You can install the whole LLVM package `choco install llvm -y`,
  * Or you can download it separately  
  The link can be found at the very bottom of the [LLVM Snapshot Builds page](https://llvm.org/builds/).
  Currently available version is [clang-format-6923b0a7.exe](https://prereleases.llvm.org/win-snapshots/clang-format-6923b0a7.exe).
  Rename downloaded file to clang-format.exe and place it somewhere in your `PATH`.
* `choco install -y nodejs`
* `choco install -y dos2unix`
* `choco install -y markdownlint-cli`
* `pip install --user yamllint`
