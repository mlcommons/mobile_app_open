#!/bin/bash

# This script is used to trigger a build on BrowserStack App Automate and monitor its status.

# Build parameters
DEVICE_LOGS=true
RETRY_INTERVAL=10

# API URLs
TRIGGER_URL="https://api-cloud.browserstack.com/app-automate/flutter-integration-tests/v2/android/build"
STATUS_URL="https://api-cloud.browserstack.com/app-automate/flutter-integration-tests/v2/android/builds"
DEVICES_URL="https://api-cloud.browserstack.com/app-automate/devices"

# Retrieve vars from environment variables
CREDENTIALS="${BROWSERSTACK_CREDENTIALS:-}"
PROJECT="${BROWSERSTACK_PROJECT:-}"
APP="${BROWSERSTACK_APP:-}"
TEST_SUITE="${BROWSERSTACK_TEST_SUITE:-}"
BUILD_TAG="${BROWSERSTACK_BUILD_TAG:-}"
DEVICES="${BROWSERSTACK_DEVICES:-}"

# Validate required environment variables
if [[ -z "$CREDENTIALS" ]]; then
  echo "Error: Environment variable BROWSERSTACK_CREDENTIALS must be set in the format 'user_name:access_key'."
  exit 1
fi

if [[ -z "$PROJECT" ||  -z "$APP" || -z "$TEST_SUITE" || -z "$BUILD_TAG" || -z "$DEVICES" ]]; then
  echo "Error: Environment variables"\
  "BROWSERSTACK_PROJECT, BROWSERSTACK_APP, BROWSERSTACK_TEST_SUITE, BROWSERSTACK_BUILD_TAG and BROWSERSTACK_DEVICES"\
  "must be set."
  exit 1
fi

# Function to get a list of available devices
get_available_devices() {
  local response=$(curl -s -u "$CREDENTIALS" -X GET "$DEVICES_URL")
  if [[ -z "$response" ]]; then
    echo "Failed to fetch available devices."
    return 1
  fi

  echo "Available devices:"
  echo "$response" | jq -r '.[] | "Device: " + .device + ", OS Version: " + .os_version'
}

# Function to trigger the build
trigger_build() {
  local response=$(curl -s -u "$CREDENTIALS" \
    -X POST "$TRIGGER_URL" \
    -d "{
          \"devices\": $DEVICES,
          \"deviceLogs\": $DEVICE_LOGS,
          \"project\": \"$PROJECT\",
          \"app\": \"$APP\",
          \"testSuite\": \"$TEST_SUITE\",
          \"buildTag\": \"$BUILD_TAG\"
        }" \
    -H "Content-Type: application/json")

  local build_id=$(echo "$response" | jq -r '.build_id')

  if [[ "$build_id" == "null" || -z "$build_id" ]]; then
    echo "Failed to trigger the build. Response: $response"
    return 1
  else
    echo "$build_id"
    return 0
  fi
}

# Function to check build status
check_build_status() {
  local build_id=$1
  local response=$(curl -s -u "$CREDENTIALS" -X GET "$STATUS_URL/$build_id")
  local status=$(echo "$response" | jq -r '.status')

  echo "$(date +'%Y-%m-%d %H:%M:%S') Build Status: $status"

  # Display device status
  if [[ "$status" != "running" ]]; then
    echo "$response" | jq -r '
      "App: " + .input_capabilities.app +
      ( .devices[] |
        ", Device: " + .device +
        ", OS Version: " + .os_version +
        ", Duration: " + (.sessions[0].duration | tostring) + "s" +
        ", Status: " + (.sessions[0].status)
      )
    '
  fi

  # Display build status
  if [[ "$status" == "passed" ]]; then
    echo "Build completed successfully!"
    exit 0
  elif [[ "$status" == "failed" ]]; then
    echo "Build failed."
    exit 1
  elif [[ "$status" == "skipped" ]]; then
    echo "Build skipped."
    exit 1
  elif [[ "$status" == "error" ]]; then
    echo "Build has error."
    exit 9
  fi
}

# Main
if [[ "$ACTIONS_RUNNER_DEBUG" == "true" ]]; then
  get_available_devices
fi

if ! BUILD_ID=$(trigger_build); then
  echo "Trigger build failed. Message: $BUILD_ID"
  exit 9
fi

echo "Build triggered successfully. Build ID: $BUILD_ID"
echo "See the build status at: https://app-automate.browserstack.com/dashboard/v2/builds/$BUILD_ID"

echo "Checking build status..."
while true; do
  check_build_status "$BUILD_ID"
  sleep "$RETRY_INTERVAL"
done
