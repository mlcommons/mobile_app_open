import 'package:firebase_dart/firebase_dart.dart';

import 'package:mlperfbench/firebase/rest_helper.dart';
import 'config.gen.dart';

class FirebaseManager {
  final FirebaseApp app;
  final FirebaseAuth auth;
  final UserCredential userInfo;
  final RestHelper restHelper;

  FirebaseManager._(this.app, this.auth, this.userInfo)
      : restHelper = RestHelper(userInfo);

  static Future<FirebaseManager> create() async {
    final app = await Firebase.initializeApp(
        options: FirebaseOptions.fromMap(firebaseConfig));
    final auth = FirebaseAuth.instanceFor(app: app);
    var userInfo = await auth.signInAnonymously();
    // await auth.signOut();
    // userInfo = await auth.signInAnonymously();

    print('name: ${userInfo.user!.displayName}');
    print('uid: ' + userInfo.user!.uid);
    print('uid: ' + await userInfo.user!.getIdToken());

    return FirebaseManager._(app, auth, userInfo);
  }
}
