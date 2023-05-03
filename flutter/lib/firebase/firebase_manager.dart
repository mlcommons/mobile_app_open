import 'package:firebase_auth/firebase_auth.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_storage/firebase_storage.dart';
import 'package:intl/intl.dart';
import 'package:mlperfbench_common/data/extended_result.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/firebase/firebase_auth_service.dart';
import 'package:mlperfbench/firebase/firebase_options.dart';
import 'package:mlperfbench/firebase/firebase_storage_service.dart';
import 'package:mlperfbench/resources/utils.dart';

class FirebaseManager {
  FirebaseManager._();

  static final currentPlatform = DefaultFirebaseOptions.currentPlatform;

  // Firebase should not be required to run this app.
  static final enabled = currentPlatform.apiKey.isNotEmpty;
  static final instance = FirebaseManager._();

  final auth = FirebaseAuthService();
  final storage = FirebaseStorageService();

  Future<FirebaseApp> initialize() async {
    final app = await Firebase.initializeApp(options: currentPlatform);
    auth.firebaseAuth = FirebaseAuth.instance;
    storage.firebaseStorage = FirebaseStorage.instance;
    print('Firebase initialized using projectId: ${app.options.projectId}');
    if (onCI) {
      await auth.signInUsingCICredential();
    } else {
      await auth.signInAnonymously();
    }
    print('User has uid: ${auth.user.uid} and email: ${auth.user.email}');
    return app;
  }

  Future<void> uploadResult(ExtendedResult result) async {
    final DateFormat formatter = DateFormat('yyyy-MM-ddTHH-mm-ss');
    final String datetime = formatter.format(result.meta.creationDate);
    final fileName = '${datetime}_${result.meta.uuid}.json';
    final uid = auth.user.uid;
    final jsonString = jsonToStringIndented(result);
    await storage.upload(jsonString, uid, fileName);
  }
}
