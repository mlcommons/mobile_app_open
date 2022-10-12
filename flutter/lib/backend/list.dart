import 'dart:io' show Platform;

import 'package:mlperfbench/backend/bridge/ffi_match.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;

part 'list.gen.dart';

class BackendInfo {
  final pb.BackendSetting settings;
  final String libPath;

  BackendInfo._(this.settings, this.libPath);

  static BackendInfo findMatching() {
    for (var path in getBackendsList()) {
      if (Platform.isWindows) {
        path = '$path.dll';
      } else if (Platform.isAndroid) {
        path = '$path.so';
      } else if (Platform.isIOS) {
        path = '$path.framework/$path';
      } else {
        throw 'unsupported platform';
      }
      final backendSettings = backendMatch(
        libPath: path,
        manufacturer: DeviceInfo.instance.manufacturer ?? '',
        model: DeviceInfo.instance.modelCode ?? '',
      );
      if (backendSettings != null) {
        return BackendInfo._(backendSettings, path);
      }
    }
    throw 'no matching backend found';
  }

  static List<String> getBackendsList() {
    if (Platform.isWindows || Platform.isAndroid || Platform.isIOS) {
      return _backendsList.where((element) => element != '').toList();
    } else {
      throw 'current platform is unsupported';
    }
  }
}
