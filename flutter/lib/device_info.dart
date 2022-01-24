import 'dart:io';

import 'package:flutter/services.dart';

import 'package:device_info/device_info.dart';
import 'package:ios_utsname_ext/extension.dart';

class DeviceInfo {
  static late final String nativeLibraryPath;
  static late final String model;
  static late final String manufacturer;
}

Future<void> initDeviceInfo() async {
  if (Platform.isAndroid) {
    DeviceInfo.nativeLibraryPath = await _getNativeLibraryPath();
  } else {
    DeviceInfo.nativeLibraryPath = '';
  }

  if (Platform.isIOS) {
    final deviceInfo = DeviceInfoPlugin();
    final iosInfo = await deviceInfo.iosInfo;

    DeviceInfo.manufacturer = 'Apple';
    DeviceInfo.model = iosInfo.utsname.machine.iOSProductName;
  } else if (Platform.isWindows) {
    DeviceInfo.manufacturer = 'Microsoft';
    DeviceInfo.model = 'Unknown PC';
  } else if (Platform.isAndroid) {
    final deviceInfo = await DeviceInfoPlugin().androidInfo;

    DeviceInfo.manufacturer = deviceInfo.manufacturer;
    DeviceInfo.model = deviceInfo.model;
  } else {
    throw 'Could not define platform';
  }
}

const _androidChannel = MethodChannel('org.mlcommons.mlperfbench/android');
Future<String> _getNativeLibraryPath() async {
  return await _androidChannel.invokeMethod('getNativeLibraryPath') as String;
}
