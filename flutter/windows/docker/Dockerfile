# escape=`
FROM mcr.microsoft.com/windows:1809

# SHELL [ "powershell", "-command" ]
SHELL ["cmd", "/S", "/C"]

ENV chocolateyVersion=1.4.0
RUN powershell Set-ExecutionPolicy Bypass -Scope Process -Force; `
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; `
    iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

ADD https://aka.ms/vs/16/release/vs_buildtools.exe vs_BuildTools.exe
RUN start /w vs_BuildTools --quiet --norestart --nocache --wait`
    --add Microsoft.VisualStudio.Workload.VCTools `
    --add Microsoft.VisualStudio.Component.Windows10SDK.19041 `
    --add Microsoft.VisualStudio.Component.VC.CMake.Project `
    && powershell -Command "if ($err = dir $Env:TEMP -Filter dd_setup_*_errors.log | where Length -gt 0 | Get-Content) { throw $err }" `
    && del vs_BuildTools.exe

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

RUN choco install -y --no-progress flutter --version 3.19.6
RUN choco install -y --no-progress git --version 2.33.0.2
RUN choco install -y --no-progress make --version 4.3
RUN choco install -y --no-progress bazelisk --version 1.16.0
RUN choco install -y --no-progress python3 --version 3.11 && copy "C:\Python311\python.exe" "C:\Python311\python3.exe"
RUN choco install -y --no-progress protoc --version 23.2

RUN python3 -m pip install --user numpy absl-py

RUN dart --disable-analytics && `
    dart pub global activate protoc_plugin ^21.1.2 && `
    setx path "%path%;%LOCALAPPDATA%/Pub/Cache/bin"

ENTRYPOINT ["cmd", "/S", "/C"]
