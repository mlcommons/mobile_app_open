# Linking to firebase

This app supports uploading benchmark results into an online database. The database and related services are backed by Firebase.

This page describes what you need to build the app with result uploading functionality and how to develop the server side for result uploading.

## Prerequisites

You need to have access to a Firebase project with Authentication, Firestore and Functions enabled.

## Linking Flutter to Firebase

1. Go to your Firebase project settings and find the list of connected apps. If the list is empty, create a new Web app.
2. Select your app and look at the `SDK setup and configuration` section. You can get Firebase config from there.
3. Create a new file `config.env` with the following content:

    ```bash
    FIREBASE_FLUTTER_CONFIG_API_KEY=<apiKey from config>
    FIREBASE_FLUTTER_CONFIG_PROJECT_ID=<projectId from config>
    FIREBASE_FLUTTER_CONFIG_MESSAGING_SENDER_ID=<messagingSenderId from config>
    FIREBASE_FLUTTER_CONFIG_APP_ID=<appId from config>
    FIREBASE_FLUTTER_CONFIG_MEASUREMENT_ID=<measurementId>
    FIREBASE_FLUTTER_FUNCTIONS_URL=https://us-central1-<projectId>.cloudfunctions.net
    FIREBASE_FLUTTER_FUNCTIONS_PREFIX=<unique_identifier>
    ```

    Here is an example with fake values:

    ```bash
    FIREBASE_FLUTTER_CONFIG_API_KEY=y40nbPVMXCovcDV-lmqUnBKYAzOSLWZSnu4rPby
    FIREBASE_FLUTTER_CONFIG_PROJECT_ID=my-project-123456
    FIREBASE_FLUTTER_CONFIG_MESSAGING_SENDER_ID=123456789012
    FIREBASE_FLUTTER_CONFIG_APP_ID=1:123456789012:web:jvfplnzhknjirxgzxoxvqu
    FIREBASE_FLUTTER_CONFIG_MEASUREMENT_ID=G-47RV91VIUJ
    FIREBASE_FLUTTER_FUNCTIONS_URL=https://us-central1-my-project-123456.cloudfunctions.net
    FIREBASE_FLUTTER_FUNCTIONS_PREFIX=v0_dev_test
    ```

    If you want to use Firebase emulator, set `FIREBASE_FLUTTER_FUNCTIONS_URL` to the following value instead:
    `http://localhost:5001/<appId>/us-central1`.
    5001 is the default port for Firebase Functions at the moment of writing this, adjust the port if you use a custom port or the default has changed.

4. Run `make flutter/firebase-config` or `make flutter/prepare` in the repository root.
    You must set `FIREBASE_CONFIG_ENV_PATH` when running this command.
    You can set it permanently as an environment variable, or you can set it for current `make` command only.
    For example: `make flutter/firebase-config FIREBASE_CONFIG_ENV_PATH=flutter/google.env`

5. Build the app, it should be able to connect to your Firebase project.

Right now Firebase Auth emulator is not supported in this app. Authentication is always done via the real Google servers.

## Generating JSON parsers

Flutter app is used as a source for structure of JSON data in requests to Firebase Functions.
Whenever you change any of the data classes, you should re-generate JSON schema and JSON parser for Functions.

For this you need Quicktype installed: `npm install -g quicktype`

To generate schema and parser run the following command: `make flutter/result`

## Developing Firebase Functions

There is no default root function name that will be uploaded to Firebase Functions service.
You must set it in Firebase config for Flutter and generate sources from it.

Run `make flutter/firebase-config` to do it. See [Linking Flutter to Firebase](#Linking-Flutter-to-Firebase) section for details.

All `firebase` and `npm` commands for Firebase Functions should be run inside `firebase_functions/functions` directory.

One time setup:

* Run `npm install -g firebase-tools` to enable `firebase` console command.
* Run `firebase login` to connect to your Google account.
* Run `firebase use <project_id>` to switch to your project.
* Run `npm install` to download all local dependencies.

Running the emulator:

* Run `npm run serve` to compile code and launch the Firebase Emulator.
* Run `npm run watch` in background to automatically recompile typescript sources.

Deploying functions on Google servers:

* Run `firebase deploy --only functions`

## Developing website

Flutter generates a single-page application (SPA) that can be deployed as a website.

The website connects to the Firebase web service deployed via Firebase Functions.
This does not imply any connections between hosting and web service.

You can use different Firebase projects for web service and website hosting, or use a completely different hosting.

All `firebase` commands for website should be run inside the `website` directory.

One time setup:

* Run `npm install -g firebase-tools` to enable `firebase` console command.
* Run `firebase login` to connect to your Google account.
* Run `firebase use <project_id>` to switch to your project.

Running the emulator:

Since website is written in Flutter, you can use `flutter run --web-port=59001` command to launch it locally.

`--web-port` is required to keep local storage between runs.
By default Flutter randomly chooses new port each time you launch a web app, which resets local storage in browser.
Particularly this causes the app to lose Firebase authentication token.

Then launching website from VS Code you can edit your `launch.json` config to include additional arguments: add `"args": [ "--web-port=59001" ]` to the website launch target.

Deploying functions on Google servers:

* Run `firebase deploy --only hosting`
