import 'dart:convert';

import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_storage/firebase_storage.dart';
import 'package:firebase_ui_auth/firebase_ui_auth.dart';

import 'package:mlperfbench/data/extended_result.dart';
import 'package:mlperfbench/data/result_file_name.dart';
import 'package:mlperfbench/firebase/firebase_auth_service.dart';
import 'package:mlperfbench/firebase/firebase_crashlytics_service.dart';
import 'package:mlperfbench/firebase/firebase_options.gen.dart';
import 'package:mlperfbench/firebase/firebase_storage_service.dart';
import 'package:mlperfbench/resources/utils.dart';

import 'package:firebase_auth/firebase_auth.dart'
    hide EmailAuthProvider, AuthProvider;

class FirebaseManager {
  FirebaseManager._();

  // Firebase should not be required to run this app.
  static final enabled = DefaultFirebaseOptions.available();
  static final instance = FirebaseManager._();

  final _authService = FirebaseAuthService();
  final _storageService = FirebaseStorageService();
  final _crashlyticsService = FirebaseCrashlyticsService();

  bool _isInitialized = false;

  Future<FirebaseManager> initialize() async {
    if (_isInitialized) {
      return instance;
    }
    final currentPlatform = DefaultFirebaseOptions.currentPlatform;
    final app = await Firebase.initializeApp(options: currentPlatform);
    print('Firebase initialized using projectId: ${app.options.projectId}');

    await _initAuthentication();
    _initStorage();
    _isInitialized = true;
    return instance;
  }

  Future<void> _initAuthentication() async {
    FirebaseUIAuth.configureProviders(FirebaseAuthService.providers);
    _authService.firebaseAuth = FirebaseAuth.instance;
    FirebaseAuth.instance.userChanges().listen((User? user) {
      print('User did change uid: ${user?.uid} | email: ${user?.email}');
      _crashlyticsService.setUserIdentifier(user?.uid ?? '');
    });
  }

  void _initStorage() {
    _storageService.firebaseStorage = FirebaseStorage.instance;
  }
}

extension Authentication on FirebaseManager {
  List<AuthProvider> get authProviders {
    return FirebaseAuthService.providers;
  }

  bool get isSignedIn {
    return FirebaseAuth.instance.currentUser != null;
  }

  Future<User> signInAnonymously() async {
    return _authService.signInAnonymously();
  }

  Future<User> signIn({required String email, required String password}) async {
    return _authService.signIn(email: email, password: password);
  }

  Future<User> link(AuthCredential authCred) async {
    return _authService.link(authCred);
  }
}

extension Storage on FirebaseManager {
  Future<void> uploadResult(ExtendedResult result) async {
    final resultFile =
        ResultFileName(result.meta.uuid, result.meta.creationDate);
    final jsonString = jsonToStringIndented(result);
    await _storageService.upload(
        jsonString, _authService.currentUser.uid, resultFile.fileName);
  }

  Future<void> deleteResult(String fileName) async {
    final uid = _authService.currentUser.uid;
    await _storageService.delete(uid, fileName);
  }

  Future<ExtendedResult> downloadResult(String fileName) async {
    print('Download online result [$fileName]');
    final uid = _authService.currentUser.uid;
    final content = await _storageService.download(uid, fileName);
    final json = jsonDecode(content) as Map<String, dynamic>;
    final result = ExtendedResult.fromJson(json);
    return result;
  }

  Future<List<ExtendedResult>> downloadResults(List<String> excluded) async {
    final uid = _authService.currentUser.uid;
    final fileNames = await _storageService.list(uid);
    List<ExtendedResult> results = [];
    for (final fileName in fileNames) {
      final resultFile = ResultFileName.fromFileName(fileName);
      if (excluded.contains(resultFile.uuid)) {
        print('Exclude local existed result [$fileName] from download');
        continue;
      }
      final result = await downloadResult(fileName);
      results.add(result);
    }
    return results;
  }

  Future<List<String>> listResults() async {
    final uid = _authService.currentUser.uid;
    final fileNames = await _storageService.list(uid);
    return fileNames;
  }
}

extension Crashlytics on FirebaseManager {
  void configureCrashlytics(bool enabled) {
    if (enabled) {
      _crashlyticsService.enableCrashlytics();
    } else {
      _crashlyticsService.disableCrashlytics();
    }
  }
}
