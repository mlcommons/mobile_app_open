param (
    [string]$cacheBucket,
    [string]$credentialsPath,
    [string]$artifactUploadPath,
    [string]$dockerImageName,
    [string]$dockerTagFile
)

echo "using bazel cache bucket: $cacheBucket"
echo "using credentials path: $credentialsPath"
echo "using artifact upload path: $artifactUploadPath"
echo "using docker image name: $dockerImageName"
echo "using docker tag file: $dockerTagFile"

$startTime = $(get-date)
$stepTime = $startTime

echo "disabling windows defender realtime protection..."
Set-MpPreference -DisableRealtimeMonitoring $true
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
echo "windows defender realtime protection disabled successfully"
echo "script run time: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $startTime).Ticks))"
echo "previous step took: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $stepTime).Ticks))"
$stepTime = $(get-date)

echo "installing ops agent..."
(New-Object Net.WebClient).DownloadFile("https://dl.google.com/cloudagents/add-google-cloud-ops-agent-repo.ps1", "${env:UserProfile}\add-google-cloud-ops-agent-repo.ps1")
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
Invoke-Expression "${env:UserProfile}\add-google-cloud-ops-agent-repo.ps1 -AlsoInstall"
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
echo "ops agent installed successfully"
echo "script run time: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $startTime).Ticks))"
echo "previous step took: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $stepTime).Ticks))"
$stepTime = $(get-date)

echo "configuring docker for GCR access..."
cmd /S /C "gcloud auth configure-docker <NUL"
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
echo "docker configured successfully"
echo "script run time: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $startTime).Ticks))"
echo "previous step took: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $stepTime).Ticks))"
$stepTime = $(get-date)

echo "obtaining environment info..."

$curDir = (Get-Item .).FullName;
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
echo "curDir is $curDir"

$projectID = gcloud config get-value project;
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
echo "projectID is $projectID"
echo "script run time: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $startTime).Ticks))"
echo "previous step took: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $stepTime).Ticks))"
$stepTime = $(get-date)

# Usually google authentication in cloudbuild containers or in compute engine VMs works by accessing LAN.
# Container inside VM doesn't have access to VM's LAN.
# Typically you would just add `--network host` arg to docker to pass LAN into container
# but it's not supported on Windows so we have to manually provide credentials.
echo "obtaining google credentials..."
$localCredentials = "output/auto-copy/google-credentials/credentials.json"
gsutil cp $credentialsPath $localCredentials
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
echo "successfully obtained google credentials"
echo "script run time: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $startTime).Ticks))"
echo "previous step took: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $stepTime).Ticks))"
$stepTime = $(get-date)

$dockerfileCommit = [IO.File]::ReadAllText($dockerTagFile)
$imageTag = "gcr.io/$projectID/$dockerImageName:$dockerfileCommit"
echo "using image: $imageTag"

$env:DOCKER_CLI_EXPERIMENTAL = "enabled"
docker manifest inspect $imageTag
if ($?) {
    echo "image management: prebuilt image found"
    echo "image management: pulling"
    docker pull $imageTag
    if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
    echo "image management: pulled successfully"
    echo "script run time: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $startTime).Ticks))"
    echo "previous step took: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $stepTime).Ticks))"
    $stepTime = $(get-date)
} else {
    echo "image management: image was not found in registry"
    echo "image management: building"
    docker build -t $imageTag flutter/windows/docker
    if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
    echo "script run time: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $startTime).Ticks))"
    echo "previous step took: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $stepTime).Ticks))"
    $stepTime = $(get-date)
    echo "image management: pushing"
    docker push $imageTag
    if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
    echo "image management: pushed successfully"
    echo "script run time: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $startTime).Ticks))"
    echo "previous step took: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $stepTime).Ticks))"
    $stepTime = $(get-date)
}

echo "launching docker..."
docker run -i `
    --rm `
    --memory 6G `
    --volume $curDir':C:/mnt/project' `
    --workdir C:/mnt/project `
    --env "BAZEL_CACHE_ARG=--remote_cache=https://storage.googleapis.com/$cacheBucket --google_default_credentials" `
    --env GOOGLE_APPLICATION_CREDENTIALS=$localCredentials `
    $imageTag `
    make flutter/windows/ci
if (!$?) { echo "error code: $($LastExitCode)"; [System.Environment]::Exit($LastExitCode) }
echo "script run time: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $startTime).Ticks))"
echo "previous step took: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $stepTime).Ticks))"
$stepTime = $(get-date)

echo "uploading release archive..."
gsutil cp output/flutter-windows-releases/ci-build.zip $artifactUploadPath
echo "release archive uploaded successfully"
echo "script run time: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $startTime).Ticks))"
echo "previous step took: $("{0:HH:mm:ss}" -f ([datetime]$($(get-date) - $stepTime).Ticks))"
$stepTime = $(get-date)
