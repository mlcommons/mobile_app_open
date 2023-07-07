# Firebase setup guide

This app is configured to use Firebase for authentication and storage.
You can use your own Firebase project by creating a `.env` file.

```shell
touch flutter/lib/firebase/firebase_options.env
```

The content of the file should be as follows.
Replace the values with your own Firebase project values.

```shell
FIREBASE_ANDROID_API_KEY=foo
FIREBASE_ANDROID_APP_ID=foo

FIREBASE_IOS_API_KEY=foo
FIREBASE_IOS_APP_ID=foo
FIREBASE_IOS_CLIENT_ID=foo
FIREBASE_IOS_BUNDLE_ID=foo

FIREBASE_PROJECT_ID=foo
FIREBASE_MESSAGING_SENDER_ID=foo
FIREBASE_DATABASE_URL=foo
FIREBASE_STORAGE_BUCKET=foo
```

That's it! You can now run the app with your own Firebase project.
