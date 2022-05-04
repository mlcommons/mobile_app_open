
# Set-MpPreference -DisableRealtimeMonitoring $true
# if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }

echo "installing ops agent..."
(New-Object Net.WebClient).DownloadFile("https://dl.google.com/cloudagents/add-google-cloud-ops-agent-repo.ps1", "${env:UserProfile}\add-google-cloud-ops-agent-repo.ps1")
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
Invoke-Expression "${env:UserProfile}\add-google-cloud-ops-agent-repo.ps1 -AlsoInstall"
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
echo "ops agent installed successfully"

cmd /S /C "gcloud auth configure-docker <NUL"
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }

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
$localCredentials = "output/auto-copy/google-credentials/credentials.json"
gsutil cp gs://mobile-app-build-290400-helper-files/compute-engine/container-credentials.json $localCredentials
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }

$cacheBucket = "mobile-app-build-290400-bazel-cache-windows"
$image = "gcr.io/$projectID/test-win-1"

echo "launching docker..."
docker run -i `
    --rm `
    --memory 6G `
    --volume $curDir':C:/mnt/project' `
    --workdir C:/mnt/project `
    --env MSYS=nonativeinnerlinks `
    --env "BAZEL_CACHE_ARG=--remote_cache=https://storage.googleapis.com/$cacheBucket --google_default_credentials" `
    --env GOOGLE_APPLICATION_CREDENTIALS=$localCredentials `
    $image `
    make flutter/windows/ci
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
