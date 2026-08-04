#!/usr/bin/env bash
# Update the template files with value from env

set -e

# FlutterFire auto-configures Firebase on iOS whenever GoogleService-Info.plist
# is bundled, and firebase-ios-sdk kills the app at launch when required options
# are empty or malformed: FIRApp validates the GOOGLE_APP_ID format, and
# FirebaseInstallations separately validates API_KEY and PROJECT_ID on app
# activation.
# Builds without Firebase secrets need well-formed dummies for all three so the
# app can start. The Dart side recognizes the dummy project ID and keeps
# Firebase disabled (see DefaultFirebaseOptions.available()).
FIREBASE_IOS_APP_ID="${FIREBASE_IOS_APP_ID:-1:000000000000:ios:0000000000000000}"
# FirebaseInstallations requires the API key to be 39 chars, start with "A",
# and contain only base64 url-safe characters.
FIREBASE_IOS_API_KEY="${FIREBASE_IOS_API_KEY:-AIzaSyDummyKeyForBuildsWithoutFirebase0}"
FIREBASE_PROJECT_ID="${FIREBASE_PROJECT_ID:-dummy-project}"

files=(
  "flutter/lib/firebase/firebase_options.template.dart:flutter/lib/firebase/firebase_options.gen.dart"
  "flutter/ios/Runner/GoogleService-Info.template.plist:flutter/ios/Runner/GoogleService-Info.plist"
  "flutter/android/app/google-services.template.json:flutter/android/app/google-services.json"
)

for file_mapping in "${files[@]}"; do
  IFS=":" read -r input_file output_file <<< "$file_mapping"

  sed \
    -e "s,FIREBASE_CI_USER_EMAIL,${FIREBASE_CI_USER_EMAIL}," \
    -e "s,FIREBASE_CI_USER_PASSWORD,${FIREBASE_CI_USER_PASSWORD}," \
    -e "s,FIREBASE_ANDROID_API_KEY,${FIREBASE_ANDROID_API_KEY}," \
    -e "s,FIREBASE_ANDROID_APP_ID,${FIREBASE_ANDROID_APP_ID}," \
    -e "s,FIREBASE_ANDROID_CLIENT_ID,${FIREBASE_ANDROID_CLIENT_ID}," \
    -e "s,FIREBASE_IOS_API_KEY,${FIREBASE_IOS_API_KEY}," \
    -e "s,FIREBASE_IOS_APP_ID,${FIREBASE_IOS_APP_ID}," \
    -e "s,FIREBASE_IOS_CLIENT_ID,${FIREBASE_IOS_CLIENT_ID}," \
    -e "s,FIREBASE_IOS_REVERSED_CLIENT_ID,${FIREBASE_IOS_REVERSED_CLIENT_ID}," \
    -e "s,FIREBASE_IOS_BUNDLE_ID,${FIREBASE_IOS_BUNDLE_ID}," \
    -e "s,FIREBASE_PROJECT_ID,${FIREBASE_PROJECT_ID}," \
    -e "s,FIREBASE_MESSAGING_SENDER_ID,${FIREBASE_MESSAGING_SENDER_ID}," \
    -e "s,FIREBASE_DATABASE_URL,${FIREBASE_DATABASE_URL}," \
    -e "s,FIREBASE_STORAGE_BUCKET,${FIREBASE_STORAGE_BUCKET}," \
    "$input_file" > "$output_file"

  echo "Generated: $input_file -> $output_file"

  if [[ "$output_file" == *.dart ]]; then
    dart format "$output_file"
  fi
done
