import 'dart:io' show Platform;

import 'package:mlperfbench_common/data/environment/environment_info.dart';

import 'package:mlperfbench/backend/bridge/ffi_match.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;

part 'list.gen.dart';

class BackendInfoHelper {
  BackendInfo findMatching() {
    for (var name in getBackendsList()) {
      final backendSettings = match(name);
      if (backendSettings != null) {
        return BackendInfo._(backendSettings, name);
      }
    }
    throw 'no matching backend found';
  }

  pb.BackendSetting? match(String libName) {
    switch (DeviceInfo.instance.envInfo.deviceType) {
      case EnvDeviceType.android:
        return matchAndroid(libName);
      case EnvDeviceType.ios:
        return matchIos(libName);
      case EnvDeviceType.windows:
        return matchWindows(libName);
      default:
        throw 'unsupported platform';
    }
  }

  pb.BackendSetting? matchAndroid(String libName) {
    final info = DeviceInfo.instance.envInfo.value.android!;
    return backendMatch(
      libName: libName,
      manufacturer: info.manufacturer ?? '',
      model: info.modelCode ?? '',
    );
  }

  pb.BackendSetting? matchIos(String libName) {
    final info = DeviceInfo.instance.envInfo.value.ios!;
    return backendMatch(
      libName: libName,
      manufacturer: 'Apple',
      model: info.modelCode ?? '',
    );
  }

  pb.BackendSetting? matchWindows(String libName) {
    return backendMatch(
      libName: libName,
      manufacturer: '',
      model: '',
    );
  }

  List<String> getBackendsList() {
    if (Platform.isWindows || Platform.isAndroid || Platform.isIOS) {
      return _backendsList.where((element) => element != '').toList();
    } else {
      throw 'current platform is unsupported';
    }
  }
}

class BackendInfo {
  final pb.BackendSetting settings;
  final String libName;

  BackendInfo._(this.settings, this.libName);
}
