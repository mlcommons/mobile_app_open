# escape=`
FROM mcr.microsoft.com/windows:1809

# SHELL [ "powershell", "-command" ]
SHELL ["cmd", "/S", "/C"]

RUN net user /add user && net localgroup Administrators user /add
RUN wmic useraccount where "Name='user'" set PasswordExpires=false

USER user

RUN powershell Set-ExecutionPolicy Bypass -Scope Process -Force; `
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; `
    iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

RUN choco install -y flutter --version 2.5.2
RUN flutter config --enable-windows-desktop

RUN curl -fSLo vs_BuildTools.exe https://aka.ms/vs/16/release/vs_buildtools.exe `
    && start /w vs_BuildTools --quiet --norestart --nocache --wait`
    --add Microsoft.VisualStudio.Workload.VCTools `
    --add Microsoft.VisualStudio.Component.Windows10SDK.19041 `
    --add Microsoft.VisualStudio.Component.VC.CMake.Project `
    && powershell -Command "if ($err = dir $Env:TEMP -Filter dd_setup_*_errors.log | where Length -gt 0 | Get-Content) { throw $err }" `
    && del vs_BuildTools.exe

RUN flutter doctor

# Without /NoUpdate msys2 installation hangs forever,
# because it can't delete a file, which name contains special symbols, from recycle bin.
# See this issue: https://github.com/docker/for-win/issues/5193
#    This issue is already closed, but the bug is still present.
RUN choco install -y msys2 --version 20220319.0.0 --params "/NoUpdate"
RUN setx path "%path%;C:/tools/msys64/usr/bin"

RUN choco install -y git --version 2.33.0.2
RUN choco install -y make --version 4.3
RUN choco install -y bazel --version 4.2.1
RUN choco install -y python3 --version 3.9.0 && copy "C:\Python39\python.exe" "C:\Python39\python3.exe"
RUN choco install -y protoc --version 3.18.1
RUN choco install -y buildifier --version 4.2.2

RUN python3 -m pip install --user numpy absl-py

RUN mkdir "C:/tools/clang-format/" && `
    setx path "%path%;C:/tools/clang-format/" && `
    curl --output "C:/tools/clang-format/clang-format.exe" `
        https://prereleases.llvm.org/win-snapshots/clang-format-6923b0a7.exe

RUN dart --disable-analytics && `
    dart pub global activate protoc_plugin && `
    setx path "%path%;%LOCALAPPDATA%/Pub/Cache/bin"

ENTRYPOINT ["cmd", "/S", "/C"]
