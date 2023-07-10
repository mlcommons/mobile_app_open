import 'package:firebase_auth/firebase_auth.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_storage/firebase_storage.dart';
import 'package:intl/intl.dart';
import 'package:mlperfbench_common/data/extended_result.dart';

import 'package:mlperfbench/firebase/firebase_auth_service.dart';
import 'package:mlperfbench/firebase/firebase_options.gen.dart';
import 'package:mlperfbench/firebase/firebase_storage_service.dart';
import 'package:mlperfbench/resources/utils.dart';

class FirebaseManager {
  FirebaseManager._();

  // Firebase should not be required to run this app.
  static final enabled = DefaultFirebaseOptions.available();
  static final instance = FirebaseManager._();

  final auth = FirebaseAuthService();
  final storage = FirebaseStorageService();

  bool _isInitialized = false;

  Future<FirebaseManager> initialize() async {
    if (_isInitialized) {
      return instance;
    }
    final currentPlatform = DefaultFirebaseOptions.currentPlatform;
    final app = await Firebase.initializeApp(options: currentPlatform);
    auth.firebaseAuth = FirebaseAuth.instance;
    storage.firebaseStorage = FirebaseStorage.instance;
    print('Firebase initialized using projectId: ${app.options.projectId}');
    if (DefaultFirebaseOptions.ciUserEmail.isNotEmpty) {
      await auth.signIn(
        email: DefaultFirebaseOptions.ciUserEmail,
        password: DefaultFirebaseOptions.ciUserPassword,
      );
    } else {
      await auth.signInAnonymously();
    }
    print('User has uid: ${auth.user.uid} and email: ${auth.user.email}');
    _isInitialized = true;
    return instance;
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
