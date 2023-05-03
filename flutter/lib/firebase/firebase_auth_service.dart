import 'package:firebase_auth/firebase_auth.dart';

class FirebaseAuthService {
  late final FirebaseAuth firebaseAuth;

  late final User user;

  signInUsingCICredential() async {
    await _signIn(
      email: const String.fromEnvironment('FIREBASE_CI_USER_EMAIL'),
      password: const String.fromEnvironment('FIREBASE_CI_USER_PASSWORD'),
    );
  }

  signInAnonymously() async {
    final userCredential = await firebaseAuth.signInAnonymously();
    user = await _getUserFromCredential(userCredential);
  }

  _signIn({required String email, required String password}) async {
    try {
      UserCredential userCredential =
          await firebaseAuth.signInWithEmailAndPassword(
        email: email,
        password: password,
      );
      user = await _getUserFromCredential(userCredential);
    } on FirebaseAuthException catch (e) {
      if (e.code == 'user-not-found') {
        print('No user found for email $email');
      } else if (e.code == 'wrong-password') {
        print('Wrong password provided for email $email.');
      }
      rethrow;
    }
  }

  Future<User> _getUserFromCredential(UserCredential? credential) async {
    final user = credential?.user;
    if (user == null) {
      throw Exception('User is not signed in');
    }
    return user;
  }
}
