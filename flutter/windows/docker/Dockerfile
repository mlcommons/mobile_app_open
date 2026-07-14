# escape=`
FROM mcr.microsoft.com/windows:1809

# SHELL [ "powershell", "-command" ]
SHELL ["cmd", "/S", "/C"]

ENV chocolateyVersion=1.4.0
RUN powershell Set-ExecutionPolicy Bypass -Scope Process -Force; `
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; `
    iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

# Visual Studio 2022 (vs/17) Build Tools. VS 2019 (vs/16) bundles CMake 3.20,
# but the Firebase C++ SDK (pulled in by firebase_core's Windows support)
# requires CMake >= 3.22. Flutter's Windows build uses the CMake bundled with
# the Visual Studio installation (not one on PATH), so the only way to raise it
# is to install a newer Visual Studio; VS 2022 17.x bundles CMake >= 3.29.
#
# VS 2022's default MSVC toolset (v143, 14.4x) fails to compile nsync (a
# TensorFlow/bazel dependency): its C++11 <chrono> platform header does not
# build with the newer STL. nsync builds fine with the VS 2019-era MSVC v142
# (14.2x) toolset (which is what the Windows build used before this upgrade), so
# we also install v142 and pin bazel to it via BAZEL_VC_FULL_VERSION below. CMake
# still uses VS 2022's default toolset for the Firebase C++ SDK.
#
# We request v142 two ways: the "latest v142" component and the fixed
# 14.29-16.11 component. The online bootstrapper has been observed to silently
# skip the "latest" component without installing a 14.2x toolset directory, so
# the fixed-version component is the reliable source of the concrete 14.29
# payload.
#
# The VC.ATL component provides atlstr.h, which the desktop_webview_auth plugin
# includes. VS 2019's VCTools workload bundled ATL, but VS 2022's does not, so
# it must be added explicitly or the plugin fails with C1083 (cannot open
# 'atlstr.h').
ADD https://aka.ms/vs/17/release/vs_buildtools.exe vs_BuildTools.exe
RUN start /w vs_BuildTools --quiet --norestart --nocache --wait`
    --add Microsoft.VisualStudio.Workload.VCTools `
    --add Microsoft.VisualStudio.Component.Windows10SDK.19041 `
    --add Microsoft.VisualStudio.Component.VC.CMake.Project `
    --add Microsoft.VisualStudio.Component.VC.ATL `
    --add Microsoft.VisualStudio.Component.VC.v142.x86.x64 `
    --add Microsoft.VisualStudio.Component.VC.14.29.16.11.x86.x64 `
    && powershell -Command "if ($err = dir $Env:TEMP -Filter dd_setup_*_errors.log | where Length -gt 0 | Get-Content) { throw $err }" `
    && del vs_BuildTools.exe

# Pin bazel to the MSVC v142 (14.2x) toolset so the native (nsync/TensorFlow)
# build uses it instead of VS 2022's default v143. Detect the installed version
# rather than hard-coding it (v142 == 14.2x; v143 is 14.3x/14.4x). Log every
# installed toolset so a mismatch is diagnosable from the build log, and include
# that list in the error if no v142 toolset is present. Set it machine-wide so
# the build process inherits it.
RUN powershell -NoProfile -Command "$root = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC'; $all = Get-ChildItem $root -Directory | Sort-Object Name; Write-Host ('Installed MSVC toolsets: ' + ($all.Name -join ', ')); $v = ($all | Where-Object { $_.Name -like '14.2*' } | Select-Object -Last 1).Name; if (-not $v) { throw ('MSVC v142 (14.2x) toolset not found. Installed: ' + ($all.Name -join ', ')) }; Write-Host ('Pinning BAZEL_VC_FULL_VERSION=' + $v); [Environment]::SetEnvironmentVariable('BAZEL_VC_FULL_VERSION', $v, 'Machine')"

# --no-progress is required on all `choco install` commands
# because on CI where the output is not a terminal but a file
# choco generates literally megabytes of progress update lines
#
# Without /NoUpdate msys2 installation hangs forever,
# because it can't delete a file, which name contains special symbols, from recycle bin.
# See this issue: https://github.com/docker/for-win/issues/5193
#   The issue is closed, but the bug is still present.
RUN choco install -y --no-progress msys2 --version 20220319.0.0 --params "/NoUpdate"
RUN setx path "%path%;C:/tools/msys64/usr/bin"

# Chocolatey's flutter package does not publish every release (e.g. 3.44.4),
# so install the official Windows SDK archive directly to pin the exact version.
# $ProgressPreference='SilentlyContinue' makes Invoke-WebRequest download the
# ~1GB archive at full speed instead of crawling on progress rendering.
#
# Extract with tar (bsdtar, shipped in Windows since 1809) rather than
# Expand-Archive: Expand-Archive silently drops files from very large archives,
# which left the Flutter SDK incomplete and made the first full compile fail
# with hundreds of "Error when reading '<sdk file>': The system cannot find the
# file specified". tar extracts the archive reliably.
RUN powershell -NoProfile -Command "$ProgressPreference='SilentlyContinue'; [Net.ServicePointManager]::SecurityProtocol=[Net.ServicePointManager]::SecurityProtocol -bor 3072; Invoke-WebRequest -Uri 'https://storage.googleapis.com/flutter_infra_release/releases/stable/windows/flutter_windows_3.44.4-stable.zip' -OutFile C:\flutter.zip; New-Item -ItemType Directory -Force C:\tools | Out-Null; tar -xf C:\flutter.zip -C C:\tools; Remove-Item C:\flutter.zip"
RUN setx path "%path%;C:\tools\flutter\bin"
RUN choco install -y --no-progress git --version 2.33.0.2
# The SDK archive ships a .git dir; allow flutter's git checks to run against it.
RUN git config --global --add safe.directory "*"
RUN choco install -y --no-progress make --version 4.3
RUN choco install -y --no-progress bazelisk --version 1.16.0
RUN choco install -y --no-progress python3 --version 3.11 && copy "C:\Python311\python.exe" "C:\Python311\python3.exe"
RUN choco install -y --no-progress protoc --version 23.2

RUN python3 -m pip install --user numpy absl-py

RUN dart --disable-analytics && `
    dart pub global activate protoc_plugin ^25.0.0 && `
    setx path "%path%;%LOCALAPPDATA%/Pub/Cache/bin"

ENTRYPOINT ["cmd", "/S", "/C"]
