import 'package:firebase_auth/firebase_auth.dart'
    hide EmailAuthProvider, AuthProvider;

import 'package:firebase_ui_auth/firebase_ui_auth.dart';

class FirebaseAuthService {
  static final List<AuthProvider> providers = [
    EmailAuthProvider(),
  ];

  late final FirebaseAuth firebaseAuth;

  User get currentUser {
    final user = FirebaseAuth.instance.currentUser;
    if (user == null) {
      throw 'FirebaseAuth.instance.currentUser is null';
    }
    print('currentUser: $user');
    return user;
  }

  Future<User> signInAnonymously() async {
    final userCredential = await firebaseAuth.signInAnonymously();
    final user = await _getUserFromCredential(userCredential);
    await user.updateDisplayName('Anonymous');
    return user;
  }

  Future<User> signIn({required String email, required String password}) async {
    try {
      final userCredential = await firebaseAuth.signInWithEmailAndPassword(
        email: email,
        password: password,
      );
      return _getUserFromCredential(userCredential);
    } on FirebaseAuthException catch (e) {
      if (e.code == 'user-not-found') {
        print('No user found for email $email');
      } else if (e.code == 'wrong-password') {
        print('Wrong password provided for email $email.');
      }
      rethrow;
    }
  }

  Future<User> link(AuthCredential authCred) async {
    try {
      final userCredential = await currentUser.linkWithCredential(authCred);
      print('userCredential: $userCredential');
      return _getUserFromCredential(userCredential);
    } on FirebaseAuthException catch (e) {
      switch (e.code) {
        case 'provider-already-linked':
          throw ('The provider has already been linked to the user.');
        case 'invalid-credential':
          throw ("The provider's credential is not valid.");
        case 'credential-already-in-use':
          throw ('The account corresponding to the credential already exists, '
              'or is already linked to a Firebase User.');
        // See the API reference for the full list of error codes.
        default:
          rethrow;
      }
    }
  }

  Future<User> _getUserFromCredential(UserCredential? credential) async {
    final user = credential?.user;
    if (user == null) {
      throw 'User is not signed in';
    }
    return user;
  }
}
