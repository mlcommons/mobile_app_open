import 'dart:io' show Platform;

import 'package:mlperfbench/backend/bridge/ffi_match.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;

part 'list.gen.dart';

class BackendInfo {
  final pb.BackendSetting settings;
  final String libPath;

  BackendInfo._(this.settings, this.libPath);

  static BackendInfo findMatching() {
    for (var path in _getBackendsList()) {
      if (path == '') {
        continue;
      }
      if (Platform.isWindows) {
        path = '$path.dll';
      } else if (Platform.isAndroid) {
        path = '$path.so';
      } else if (Platform.isIOS) {
        path = '$path.framework/$path';
      } else {
        throw 'unsupported platform';
      }
      final backendSettings = backendMatch(path);
      if (backendSettings != null) {
        return BackendInfo._(backendSettings, path);
      }
    }
    throw 'no matching backend found';
  }

  static List<String> _getBackendsList() {
    if (Platform.isWindows || Platform.isAndroid || Platform.isIOS) {
      return _backendsList;
    } else {
      throw 'current platform is unsupported';
    }
  }

  static List<String> getExportBackendsList() {
    if (Platform.isWindows || Platform.isAndroid || Platform.isIOS) {
      final result = List<String>.from(_backendsList);
      result.removeWhere((element) => element == '');
      return result;
    } else {
      throw 'current platform is unsupported';
    }
  }
}
