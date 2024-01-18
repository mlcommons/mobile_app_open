# Web App

This directory contains the React web app for viewing the submitted benchmark results.

## Update Firebase configuration

You need to use your own Firebase project to run this web app.

1. First create a `firebase.env`

   ```shell
   touch firebase.env
   ```

1. Then copy and paste this to the `firebase.env` file you just created.
   Remember to update the dummy values with your Firebase project settings.

   ```dotenv
   export REACT_APP_FIREBASE_WEB_API_KEY=abc
   export REACT_APP_FIREBASE_WEB_AUTH_DOMAIN=abc
   export REACT_APP_FIREBASE_WEB_APP_ID=abc
   export REACT_APP_FIREBASE_WEB_MEASUREMENT_ID=abc

   export REACT_APP_FIREBASE_PROJECT_ID=abc
   export REACT_APP_FIREBASE_MESSAGING_SENDER_ID=abc
   export REACT_APP_FIREBASE_DATABASE_URL=abc
   export REACT_APP_FIREBASE_STORAGE_BUCKET=abc
   ```

1. Import the env vars into your shell

   ```shell
   source firebase.env
   ```

## Run and deploy the app to Firebase Hosting

To deploy the app using Firebase Hosting please
visit [Get started with Firebase Hosting](https://firebase.google.com/docs/hosting/quickstart) for more
information.

You will need [Firebase CLI](https://firebase.google.com/docs/cli#install_the_firebase_cli) for the following commands.

To run the web app locally:

```shell
yarn build
firebase serve --only hosting --project <FIREBASE_PROJECT_ID>
```

To deploy the web app to Firebase Hosting

```shell
firebase deploy --only hosting --project <FIREBASE_PROJECT_ID>
```

To enable CORS: <https://firebase.google.com/docs/storage/web/download-files>
