import 'package:firebase_dart/firebase_dart.dart';
import 'package:path_provider/path_provider.dart';

import 'package:mlperfbench/firebase/rest_helper.dart';
import 'config.gen.dart';

class FirebaseManager {
  final FirebaseApp app;
  final FirebaseAuth auth;
  final UserCredential userInfo;
  final RestHelper restHelper;

  FirebaseManager._(this.app, this.auth, this.userInfo)
      : restHelper = RestHelper(userInfo);

  static Future<void> staticInit() async {
    if (!FirebaseConfig.enable) {
      return;
    }
    final firebaseDataDir = (await getApplicationDocumentsDirectory()).path;
    FirebaseDart.setup(isolated: true, storagePath: firebaseDataDir);
  }

  static Future<FirebaseManager?> create() async {
    if (!FirebaseConfig.enable) {
      return null;
    }

    final app = await Firebase.initializeApp(
        options: FirebaseOptions.fromMap(FirebaseConfig.connectionConfig));
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
