import 'dart:isolate';

import 'package:flutter/foundation.dart';

import 'package:firebase_crashlytics/firebase_crashlytics.dart';

class FirebaseCrashlyticsService {
  SendPort? sendPort;

  void enableCrashlytics() {
    print('Enable Firebase Crashlytics');
    FirebaseCrashlytics.instance.setCrashlyticsCollectionEnabled(true);

    // Pass all uncaught "fatal" errors from the framework to Crashlytics
    FlutterError.onError = (errorDetails) {
      FirebaseCrashlytics.instance.recordFlutterFatalError(errorDetails);
    };

    // Pass all uncaught asynchronous errors that aren't handled by the Flutter framework to Crashlytics
    PlatformDispatcher.instance.onError = (error, stack) {
      FirebaseCrashlytics.instance.recordError(error, stack, fatal: true);
      return true;
    };

    // Pass errors that happen outside of the Flutter context to Crashlytics
    SendPort port = RawReceivePort((pair) async {
      final List<dynamic> errorAndStacktrace = pair as List<dynamic>;
      await FirebaseCrashlytics.instance.recordError(
        errorAndStacktrace.first,
        errorAndStacktrace.last as StackTrace,
        fatal: true,
      );
    }).sendPort;
    Isolate.current.addErrorListener(port);
    sendPort = port;
  }

  void disableCrashlytics() {
    print('Disable Firebase Crashlytics');
    FirebaseCrashlytics.instance.setCrashlyticsCollectionEnabled(false);

    FlutterError.onError = FlutterError.presentError;
    PlatformDispatcher.instance.onError = (error, stack) {
      print(error);
      return false;
    };
    final port = sendPort;
    if (port != null) {
      Isolate.current.removeErrorListener(port);
    }
  }

  void setUserIdentifier(String identifier) {
    FirebaseCrashlytics.instance.setUserIdentifier(identifier);
  }
}
