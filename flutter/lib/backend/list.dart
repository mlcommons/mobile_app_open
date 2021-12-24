import 'dart:io' show Platform;

part 'list.gen.dart';

List<String> getBackendsList() {
  if (Platform.isIOS) {
    // on iOS backend is statically linked
    return [];
  } else if (Platform.isWindows || Platform.isAndroid) {
    return _backendsList;
  } else {
    throw 'current platform is unsupported';
  }
}
