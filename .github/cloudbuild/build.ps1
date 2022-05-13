
echo "disabling windows defender realtime protection..."
Set-MpPreference -DisableRealtimeMonitoring $true
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
echo "windows defender realtime protection disabled successfully"

echo "installing ops agent..."
(New-Object Net.WebClient).DownloadFile("https://dl.google.com/cloudagents/add-google-cloud-ops-agent-repo.ps1", "${env:UserProfile}\add-google-cloud-ops-agent-repo.ps1")
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
Invoke-Expression "${env:UserProfile}\add-google-cloud-ops-agent-repo.ps1 -AlsoInstall"
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
echo "ops agent installed successfully"

echo "installing chocolatey..."
Set-ExecutionPolicy Bypass -Scope Process -Force
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
echo "chocolatey installed successfully"

choco install -y git --version 2.33.0.2
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }

# we need to update PATH to access git command
# this method is from https://stackoverflow.com/a/56033268
function resetEnv {
    Set-Item `
        -Path (('Env:', $args[0]) -join '') `
        -Value ((
            [System.Environment]::GetEnvironmentVariable($args[0], "Machine"),
            [System.Environment]::GetEnvironmentVariable($args[0], "User")
        ) -match '.' -join ';')
}
resetEnv Path
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }

echo "configuring docker for GCR access..."
cmd /S /C "gcloud auth configure-docker <NUL"
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
echo "docker configured successfully"

echo "obtaining environment info..."

$curDir = (Get-Item .).FullName;
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
echo "curDir is $curDir"

$projectID = gcloud config get-value project;
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
echo "projectID is $projectID"

# Usually google authentication in cloudbuild containers or in compute engine VMs works by accessing LAN.
# Container inside VM doesn't have access to VM's LAN.
# Typically you would just add `--network host` arg to docker to pass LAN into container
# but it's not supported on Windows so we have to manually provide credentials.
echo "obtaining google credentials..."
$localCredentials = "output/auto-copy/google-credentials/credentials.json"
gsutil cp gs://mobile-app-build-290400-helper-files/compute-engine/container-credentials.json $localCredentials
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
echo "successfully obtained google credentials"

$dockerfileCommit = git log -n 1 --pretty=format:%H -- flutter/windows/docker/Dockerfile
$imageTag = "gcr.io/$projectID/flutter-windows-ci:$dockerfileCommit"
echo "using image: $imageTag"

docker manifest inspect $imageTag
if ($?) {
    echo "image management: prebuilt image found"
    echo "image management: pulling"
    docker pull $imageTag
    if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
    echo "image management: pulled successfully"
} else {
    echo "image management: image was not found in registry"
    echo "image management: building"
    docker build -t $imageTag flutter/windows/docker
    if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
    echo "image management: pushing"
    docker push $imageTag
    if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
    echo "image management: pushed successfully"
}

$cacheBucket = "mobile-app-build-290400-bazel-cache-windows"
echo "using bazel cache bucket: $cacheBucket"

echo "launching docker..."
docker run -i `
    --rm `
    --memory 6G `
    --volume $curDir':C:/mnt/project' `
    --workdir C:/mnt/project `
    --env MSYS=nonativeinnerlinks `
    --env "BAZEL_CACHE_ARG=--remote_cache=https://storage.googleapis.com/$cacheBucket --google_default_credentials" `
    --env GOOGLE_APPLICATION_CREDENTIALS=$localCredentials `
    $imageTag `
    make flutter/windows/ci
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
