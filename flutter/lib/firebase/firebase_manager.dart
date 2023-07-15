import 'dart:convert';

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
    // Example fileName: 2023-06-06T13-38-01_125ef847-ca9a-45e0-bf36-8fd22f493b8d.json
    final fileName = '${datetime}_${result.meta.uuid}.json';
    final uid = auth.user.uid;
    final jsonString = jsonToStringIndented(result);
    await storage.upload(jsonString, uid, fileName);
  }

  Future<List<ExtendedResult>> downloadResults(List<String> excluded) async {
    final uid = auth.user.uid;
    final fileNames = await storage.list(uid);
    List<ExtendedResult> results = [];
    for (final fileName in fileNames) {
      // Example fileName: 2023-06-06T13-38-01_125ef847-ca9a-45e0-bf36-8fd22f493b8d.json
      final resultUuid = fileName.replaceAll('.json', '').split('_').last;
      if (excluded.contains(resultUuid)) {
        print('Exclude local existed result [$fileName] from download');
        continue;
      }
      final content = await storage.download(uid, fileName);
      final json = jsonDecode(content) as Map<String, dynamic>;
      final result = ExtendedResult.fromJson(json);
      print('Downloaded result with uuid: ${result.meta.uuid}');
      results.add(result);
    }
    return results;
  }
}
