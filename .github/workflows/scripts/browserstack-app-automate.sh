#!/bin/bash

# This script is used to trigger a build on BrowserStack App Automate and monitor its status.

set -e

# Build parameters
DEVICES='["Samsung Galaxy S24-14.0"]'
PROJECT="mobile-app-build-290400"
DEVICE_LOGS=true
RETRY_INTERVAL=10

# API URLs
TRIGGER_URL="https://api-cloud.browserstack.com/app-automate/flutter-integration-tests/v2/android/build"
STATUS_URL="https://api-cloud.browserstack.com/app-automate/flutter-integration-tests/v2/android/builds"

# Retrieve vars from environment variables
CREDENTIALS="${BROWSERSTACK_CREDENTIALS:-}"
APP="${BROWSERSTACK_APP:-}"
TEST_SUITE="${BROWSERSTACK_TEST_SUITE:-}"
BUILD_TAG="${BROWSERSTACK_BUILD_TAG:-}"

# Validate required environment variables
if [[ -z "$CREDENTIALS" ]]; then
  echo "Error: Environment variable BROWSERSTACK_CREDENTIALS must be set in the format 'user_name:access_key'."
  exit 1
fi

if [[ -z "$APP" || -z "$TEST_SUITE" || -z "$BUILD_TAG" ]]; then
  echo "Error: Environment variables BROWSERSTACK_APP, BROWSERSTACK_TEST_SUITE and BROWSERSTACK_BUILD_TAG must be set."
  exit 1
fi

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
    exit 1
  fi

  echo "$build_id"
}

# Function to check build status
check_build_status() {
  local build_id=$1
  local response=$(curl -s -u "$CREDENTIALS" -X GET "$STATUS_URL/$build_id")
  local status=$(echo "$response" | jq -r '.status')
  echo "$(date +'%Y-%m-%d %H:%M:%S') Build Status: $status"
  if [[ "$status" == "passed" ]]; then
    echo "Build completed successfully!"
    exit 0
  elif [[ "$status" == "failed" ]]; then
    echo "Build failed."
    exit 1
  fi
}

# Main
BUILD_ID=$(trigger_build)
echo "Build triggered successfully. Build ID: $BUILD_ID"
echo "See the build status at: https://app-automate.browserstack.com/dashboard/v2/builds/$BUILD_ID"

echo "Checking build status..."
while true; do
  check_build_status "$BUILD_ID"
  sleep "$RETRY_INTERVAL"
done
