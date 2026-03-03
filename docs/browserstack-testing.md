# BrowserStack App Automate Testing

This document explains how to manually build, upload, and trigger Flutter
integration tests on BrowserStack App Automate for both Android and iOS.

## Prerequisites

Set your BrowserStack credentials:

```bash
export BROWSERSTACK_CREDENTIALS="user_name:access_key"
```

## Android

### 1. Build test APKs

```bash
make flutter/prepare
make flutter/android/release flutter/android/apk flutter/android/test-apk
```

This produces two files in `output/android-apks/`:
- `test-main.apk` — the app under test
- `test-helper.apk` — the instrumentation test suite

### 2. Upload to BrowserStack

```bash
# Upload main app
curl -u "$BROWSERSTACK_CREDENTIALS" \
  -X POST "https://api-cloud.browserstack.com/app-automate/flutter-integration-tests/v2/android/app" \
  -F "file=@output/android-apks/test-main.apk" \
  -F "custom_id=test-main.apk"

# Upload test suite
curl -u "$BROWSERSTACK_CREDENTIALS" \
  -X POST "https://api-cloud.browserstack.com/app-automate/flutter-integration-tests/v2/android/test-suite" \
  -F "file=@output/android-apks/test-helper.apk" \
  -F "custom_id=test-helper.apk"
```

### 3. Trigger the build

```bash
BROWSERSTACK_PROJECT=mobile_app_open \
BROWSERSTACK_APP=test-main.apk \
BROWSERSTACK_TEST_SUITE=test-helper.apk \
BROWSERSTACK_BUILD_TAG="Samsung Galaxy S24-14.0" \
BROWSERSTACK_DEVICES='["Samsung Galaxy S24-14.0"]' \
BROWSERSTACK_LOGS_DIR=/tmp/browserstack-device-logs \
bash .github/workflows/scripts/browserstack-app-automate.sh
```

## iOS

### 1. Build the test package

```bash
# Build iOS libs (runs bazel, installs pods, etc.)
cd flutter/ios/ci_scripts/ && bash ci_post_clone.sh && cd ../../../

# Build and zip the test package
make flutter/ios/test-package
```

This produces `output/ios-test-package/ios_tests.zip` containing the
`Release-iphoneos/` directory and `*.xctestrun` file.

**Note:** `build-for-testing` requires code signing. By default it uses
`CODE_SIGN_IDENTITY="Apple Development"` and the team configured in the
Xcode project. Override via environment variables if needed:

```bash
DEVELOPMENT_TEAM=XXXXXXXXXX make flutter/ios/test-package
```

### 2. Upload to BrowserStack

```bash
curl -u "$BROWSERSTACK_CREDENTIALS" \
  -X POST "https://api-cloud.browserstack.com/app-automate/flutter-integration-tests/v2/ios/test-package" \
  -F "file=@output/ios-test-package/ios_tests.zip" \
  -F "custom_id=ios_tests.zip"
```

### 3. Trigger the build

```bash
BROWSERSTACK_PLATFORM=ios \
BROWSERSTACK_PROJECT=mobile_app_open \
BROWSERSTACK_TEST_PACKAGE=ios_tests.zip \
BROWSERSTACK_BUILD_TAG="iPhone 16 Pro-18" \
BROWSERSTACK_DEVICES='["iPhone 16 Pro-18"]' \
BROWSERSTACK_LOGS_DIR=/tmp/browserstack-device-logs \
bash .github/workflows/scripts/browserstack-app-automate.sh
```

## Environment Variables Reference

| Variable | Required | Platform | Description |
|---|---|---|---|
| `BROWSERSTACK_CREDENTIALS` | Yes | Both | `user_name:access_key` |
| `BROWSERSTACK_PLATFORM` | No | Both | `android` (default) or `ios` |
| `BROWSERSTACK_PROJECT` | Yes | Both | Project name on BrowserStack |
| `BROWSERSTACK_APP` | Yes | Android | Custom ID of the uploaded main app |
| `BROWSERSTACK_TEST_SUITE` | Yes | Android | Custom ID of the uploaded test suite |
| `BROWSERSTACK_TEST_PACKAGE` | Yes | iOS | Custom ID of the uploaded test package |
| `BROWSERSTACK_BUILD_TAG` | Yes | Both | Tag for identifying the build |
| `BROWSERSTACK_DEVICES` | Yes | Both | JSON array of device strings, e.g. `'["iPhone 16 Pro-18"]'` |
| `BROWSERSTACK_LOGS_DIR` | No | Both | Directory to download device logs to |

## Monitoring

The script polls the build status and prints updates every 10 seconds.
It also prints a link to the BrowserStack dashboard where you can monitor
the build in the browser.

Refer to the [BrowserStack App Automate API documentation](https://www.browserstack.com/automate/app-automate/rest-api)
for more details.
