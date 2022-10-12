import 'dart:io';

import 'package:flutter/services.dart';

import 'package:device_info/device_info.dart';
import 'package:device_marketing_names/device_marketing_names.dart';
import 'package:mlperfbench_common/data/environment/environment_info.dart';

class DeviceInfo {
  final String modelCode;
  final String modelName;
  final String manufacturer;

  static late final String nativeLibraryPath;
  static late final DeviceInfo instance;

  DeviceInfo({
    required this.manufacturer,
    required this.modelCode,
    required this.modelName,
  });

  static Future<void> staticInit() async {
    DeviceInfo.nativeLibraryPath = await _makeNativeLibraryPath();
    DeviceInfo.instance = await createFromEnvironment();
  }

  static Future<DeviceInfo> createFromEnvironment() async {
    if (Platform.isIOS) {
      return _makeIosInfo();
    } else if (Platform.isWindows) {
      return _makeWindowsInfo();
    } else if (Platform.isAndroid) {
      return _makeAndroidInfo();
    } else {
      throw 'Could not define platform';
    }
  }

  static Future<String> _makeNativeLibraryPath() async {
    if (Platform.isAndroid) {
      return await _getNativeLibraryPath();
    } else {
      return '';
    }
  }

  static Future<DeviceInfo> _makeIosInfo() async {
    final deviceInfo = await DeviceInfoPlugin().iosInfo;
    final deviceNames = DeviceMarketingNames();

    return DeviceInfo(
      manufacturer: 'Apple',
      modelCode: deviceInfo.utsname.machine,
      modelName: deviceNames.getSingleNameFromModel(
          DeviceType.ios, deviceInfo.utsname.machine),
    );
  }

  static Future<DeviceInfo> _makeAndroidInfo() async {
    final deviceInfo = await DeviceInfoPlugin().androidInfo;
    final deviceNames = DeviceMarketingNames();

    return DeviceInfo(
      manufacturer: deviceInfo.manufacturer,
      modelCode: deviceInfo.model,
      modelName: deviceNames.getSingleNameFromModel(
          DeviceType.android, deviceInfo.model),
    );
  }

  static Future<DeviceInfo> _makeWindowsInfo() async {
    return DeviceInfo(
      manufacturer: '',
      modelCode: 'Unknown PC',
      modelName: 'Unknown PC',
    );
  }

  static EnvironmentInfo get environmentInfo {
    return EnvironmentInfo(
      osName: EnvironmentInfo.parseOs(Platform.operatingSystem),
      osVersion: Platform.operatingSystemVersion,
      manufacturer: DeviceInfo.instance.manufacturer,
      modelCode: DeviceInfo.instance.modelCode,
      modelName: DeviceInfo.instance.modelName,
    );
  }
}

const _androidChannel = MethodChannel('org.mlcommons.mlperfbench/android');
Future<String> _getNativeLibraryPath() async {
  return await _androidChannel.invokeMethod('getNativeLibraryPath') as String;
}
