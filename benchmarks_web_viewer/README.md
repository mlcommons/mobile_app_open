# Web app

## Update Firebase configuration

You need to use your own Firebase project to run this web app.

1. First create a `firebase.env`

```shell
touch firebase.env
```

2. Then copy and paste this to the `firebase.env` file you just created.
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

3. Import the env vars into your shell

```shell
source firebase.env
```

## Deploy the app to Firebase Hosting

To deploy the app using the Firebase Hosting please check these
instructions: https://firebase.google.com/docs/hosting/quickstart
To enable CORS: https://firebase.google.com/docs/storage/web/download-files
