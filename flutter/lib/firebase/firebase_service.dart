import 'package:firebase_auth/firebase_auth.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_storage/firebase_storage.dart';
import 'package:intl/intl.dart';
import 'package:mlperfbench_common/data/extended_result.dart';

import 'package:mlperfbench/firebase/firebase_options.dart';
import 'package:mlperfbench/resources/utils.dart';

import 'package:mlperfbench/app_constants.dart';

class FirebaseService {
  FirebaseService._();

  // Firebase should not be required to run this app.
  static final bool enabled =
      DefaultFirebaseOptions.currentPlatform.apiKey.isNotEmpty;

  static final instance = FirebaseService._();

  static const _uidPlaceholder = '<UID>';
  static const _filePlaceholder = '<FILE>';
  static const _pathTemplate = 'user/$_uidPlaceholder/result/$_filePlaceholder';

  Future<FirebaseApp> initialize() async {
    final firebaseApp = await Firebase.initializeApp(
      options: DefaultFirebaseOptions.currentPlatform,
    );
    final projectId = firebaseApp.options.projectId;
    print('Firebase initialized using projectId: $projectId');
    if (onCI) {
      await signInUsingCICredential();
    } else {
      await signInAnonymously();
    }
    return firebaseApp;
  }

  Future<String> uploadResult(ExtendedResult result) async {
    final DateFormat formatter = DateFormat('yyyy-MM-ddTHH-mm-ss');
    final String datetime = formatter.format(result.meta.creationDate);
    final fileName = '${datetime}_${result.meta.uuid}.json';
    final uid = FirebaseAuth.instance.currentUser?.uid;
    if (uid == null) {
      throw 'User is not signed in';
    }
    final location = _pathTemplate
        .replaceAll(_uidPlaceholder, uid)
        .replaceAll(_filePlaceholder, fileName);
    final storageRef = FirebaseStorage.instance.ref();
    final fileRef = storageRef.child(location);
    if (await doesFileExist(fileRef)) {
      print('File $fileName already uploaded');
    } else {
      print('Uploading to $location');
      final metadata = SettableMetadata(contentType: 'text/plain');
      final jsonString = jsonToStringIndented(result);
      await fileRef.putString(jsonString, metadata: metadata);
      print('Uploaded to $location');
    }
    final url = await fileRef.getDownloadURL();
    print('URL is $url');
    return url;
  }

  Future<bool> doesFileExist(Reference ref) async {
    try {
      await ref.getMetadata();
      return true;
    } catch (e) {
      return false;
    }
  }

  Future<User> signIn({required String email, required String password}) async {
    try {
      UserCredential userCredential =
          await FirebaseAuth.instance.signInWithEmailAndPassword(
        email: email,
        password: password,
      );
      final user = userCredential.user;
      if (user == null) {
        throw Exception('User is not signed in');
      }
      print('Signed in with email $email using uid: ${user.uid}');
      return user;
    } on FirebaseAuthException catch (e) {
      if (e.code == 'user-not-found') {
        print('No user found for that email.');
      } else if (e.code == 'wrong-password') {
        print('Wrong password provided for that user.');
      }
      rethrow;
    }
  }

  Future<User> signInUsingCICredential() async {
    return await signIn(
      email: const String.fromEnvironment('FIREBASE_CI_USER_EMAIL'),
      password: const String.fromEnvironment('FIREBASE_CI_USER_PASSWORD'),
    );
  }

  Future<User> signInAnonymously() async {
    final auth = FirebaseAuth.instance;
    final userCredential = await auth.signInAnonymously();
    final user = userCredential.user;
    if (user == null) {
      throw Exception('User is not signed in');
    }
    print('Signed in anonymously using uid: ${user.uid}');
    return user;
  }
}
