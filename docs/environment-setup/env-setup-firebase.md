# Firebase Setup Guide

## Firebase Authentication and Storage

This app is configured to use Firebase for authentication and storage.
A Firebase project is required if you need the Firebase related features.
You can use your own Firebase project by creating a `.env` file.

```shell
touch flutter/lib/firebase/firebase_options.env
```

Go to the [Firebase Console](https://console.firebase.google.com/) and create a new project or select an existing one.
Replace the values with your own Firebase project values.
The content of the `firebase_options.env` file should be as follows:

```dotenv
FIREBASE_ANDROID_API_KEY=foo
FIREBASE_ANDROID_APP_ID=foo

FIREBASE_IOS_API_KEY=foo
FIREBASE_IOS_APP_ID=foo
FIREBASE_IOS_CLIENT_ID=foo
FIREBASE_IOS_REVERSED_CLIENT_ID=foo
FIREBASE_IOS_BUNDLE_ID=foo

FIREBASE_PROJECT_ID=foo
FIREBASE_MESSAGING_SENDER_ID=foo
FIREBASE_DATABASE_URL=foo
FIREBASE_STORAGE_BUCKET=foo
```

## Firebase Crashlytics

By default, we disable the upload of mapping file (in Android build) or debug symbol (dSYM) file in iOS build
as described
in [Get readable crash reports in the Crashlytics dashboard](https://firebase.google.com/docs/crashlytics/get-deobfuscated-reports?platform=flutter).

To enable it, you need to set the environment variable:

```shell
export FIREBASE_CRASHLYTICS_ENABLED=true
```

For Android release builds (`make flutter/android/release`), this also builds the Dart code with
`--split-debug-info` and uploads its symbols with `firebase crashlytics:symbols:upload`, so crashes in
`libapp.so` and Dart stack traces are readable in Crashlytics. This needs the
[Firebase CLI](https://firebase.google.com/docs/cli) (preinstalled in the Android Docker image) and
`FIREBASE_ANDROID_APP_ID` set in the environment. The upload does not need a Firebase login.
The symbols stay in `flutter/build/app/outputs/symbols/{apk,aab}`; keep them to symbolicate a stack trace
offline with `flutter symbolize`.

That's it! You can now run the app with your own Firebase project.

## Firebase Extension

Following Firebase extensions are recommended to use when you set up your own Firebase project:

* [Delete User Data](https://extensions.dev/extensions/firebase/delete-user-data)
