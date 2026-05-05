# iOS BrowserStack Workflow Setup

This guide explains how to set up the `ios-browserstack-test.yml` GitHub Actions
workflow, including where to get and where to save the required secrets and
variables.

## Overview

The workflow has two jobs:

1. **build-ios-test-package** — builds the Flutter iOS app with
   `xcodebuild build-for-testing` on a `macos-15` runner and uploads the
   resulting zip as an artifact.
2. **test-ios-browserstack** — downloads the zip, uploads it to BrowserStack,
   and triggers Flutter integration tests on real iOS devices.

## Where to save secrets and variables

Go to your GitHub repository **Settings > Secrets and variables > Actions**.

* **Secrets** (encrypted) — for credentials, keys, and certificates.
* **Variables** (plaintext) — for non-sensitive configuration.

## Required secrets

### BrowserStack

| Secret | Where to get it |
|---|---|
| `BROWSERSTACK_CREDENTIALS` | BrowserStack [Account Settings](https://www.browserstack.com/accounts/settings). Format: `user_name:access_key`. |

### Apple code signing

These are needed to sign the iOS test package for real device testing.
See the [GitHub docs on signing Xcode applications](https://docs.github.com/en/actions/how-tos/deploy/deploy-to-third-party-platforms/sign-xcode-applications)
for a detailed walkthrough.

| Secret | Where to get it                                                                                                                                                                                                                                                                       |
|---|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `IOS_BUILD_CERTIFICATE_BASE64` | Export a Development certificate (.p12) from Keychain Access or Xcode app, then base64-encode it: `base64 -i build_certificate.p12 \| pbcopy` |
| `IOS_BUILD_CERTIFICATE_PASSWORD` | The password you set when exporting the .p12 certificate. |
| `IOS_KEYCHAIN_PASSWORD` | Any random password for the temporary CI keychain. Generate one with: `openssl rand -base64 32` |
| `APPLE_DEVELOPMENT_TEAM` | Your 10-character Apple Team ID. Find it at [Apple Developer > Membership details](https://developer.apple.com/account#MembershipDetailsCard) or in Xcode under **Signing & Capabilities**. |
| `APP_STORE_CONNECT_API_KEY_BASE64` | Base64-encoded App Store Connect API key (.p8 file). See steps below. |
| `APP_STORE_CONNECT_API_KEY_ID` | The Key ID shown in App Store Connect when creating the API key. |
| `APP_STORE_CONNECT_API_KEY_ISSUER_ID` | The Issuer ID from [App Store Connect > Users and Access > Integrations > App Store Connect API](https://appstoreconnect.apple.com/access/integrations/api). |

### Firebase

Get these from the [Firebase Console](https://console.firebase.google.com/)
under **Project Settings > General > Your apps > iOS app**.

| Secret | Where to get it |
|---|---|
| `FIREBASE_IOS_API_KEY` | `API_KEY` in `GoogleService-Info.plist` |
| `FIREBASE_IOS_APP_ID` | App ID in Firebase Console |
| `FIREBASE_IOS_CLIENT_ID` | `CLIENT_ID` from `GoogleService-Info.plist` |
| `FIREBASE_IOS_REVERSED_CLIENT_ID` | `REVERSED_CLIENT_ID` from `GoogleService-Info.plist` |
| `FIREBASE_IOS_BUNDLE_ID` | `BUNDLE_ID` from `GoogleService-Info.plist` |
| `FIREBASE_PROJECT_ID` | Project Settings > General > Project ID |
| `FIREBASE_MESSAGING_SENDER_ID` | Project Settings > Cloud Messaging > Sender ID |
| `FIREBASE_DATABASE_URL` | Realtime Database > URL |
| `FIREBASE_STORAGE_BUCKET` | Storage > Bucket URL |

## Required variables

| Variable | Description |
|---|---|
| `FLUTTER_BUILD_NUMBER_OFFSET` | Optional offset added to `GITHUB_RUN_NUMBER` to compute the build number. Set this if you need to align build numbers across workflows. Default: `0`. |

## Step-by-step: Creating the Apple certificate and API key

### 1. Create a certificate signing request (CSR)

* Open **Keychain Access** on your Mac
* Go to **Keychain Access > Certificate Assistant > Request a Certificate From a Certificate Authority**
* Enter your email, select **Saved to disk**, and save the CSR file

### 2. Create a Development certificate

* Go to [Apple Developer > Certificates](https://developer.apple.com/account/resources/certificates/list)
* Click **+**, select **Apple Development**, upload the CSR, and download the certificate
* Double-click the downloaded `.cer` file to install it in Keychain Access

### 3. Export as .p12

* In **Keychain Access**, find the installed certificate under **My Certificates**
* Right-click > **Export** and save as `.p12` with a password
* Base64-encode it:

  ```bash
  base64 -i certificate.p12 | pbcopy
  ```

* Save the base64 string as `IOS_BUILD_CERTIFICATE_BASE64` and the password as `IOS_BUILD_CERTIFICATE_PASSWORD`

### 4. Create an App Store Connect API key

* Go to [App Store Connect > Users and Access > Integrations > App Store Connect API](https://appstoreconnect.apple.com/access/integrations/api)
* Click **+** to generate a new key
* Give it a name (e.g. `CI Signing`) and select the **Developer** role
* Download the `.p8` file (you can only download it once)
* Note the **Key ID** and **Issuer ID** shown on the page
* Base64-encode the key:

  ```bash
  base64 -i AuthKey_XXXXXXXXXX.p8 | pbcopy
  ```

* Save the base64 string as `APP_STORE_CONNECT_API_KEY_BASE64`
* Save the Key ID as `APP_STORE_CONNECT_API_KEY_ID`
* Save the Issuer ID as `APP_STORE_CONNECT_API_KEY_ISSUER_ID`

### 5. Set the keychain password

Generate a random password and save it as `IOS_KEYCHAIN_PASSWORD`:

```bash
openssl rand -base64 32
```

### 6. Find your Team ID

* Go to [Apple Developer > Membership details](https://developer.apple.com/account#MembershipDetailsCard)
* Copy the 10-character **Team ID**
* Save as `APPLE_DEVELOPMENT_TEAM`
