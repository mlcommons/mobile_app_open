import 'package:firebase_dart/firebase_dart.dart';
import 'package:path_provider/path_provider.dart';

import 'config.gen.dart';
import 'rest_helper.dart';

class FirebaseManager {
  static late final FirebaseManager _defaultInstance;

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
    _defaultInstance = await _create();
  }

  static Future<FirebaseManager> _create({String? name}) async {
    final app = await Firebase.initializeApp(
        name: name,
        options: FirebaseOptions.fromMap(FirebaseConfig.connectionConfig));
    final auth = FirebaseAuth.instanceFor(app: app);
    var userInfo = await auth.signInAnonymously();

    print(
        'logged into firebase: name=${userInfo.user!.displayName}, uid=${userInfo.user!.uid}');

    return FirebaseManager._(app, auth, userInfo);
  }

  static FirebaseManager? get instance {
    if (!FirebaseConfig.enable) {
      return null;
    }
    return _defaultInstance;
  }
}
