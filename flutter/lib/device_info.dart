import 'dart:io';

import 'package:flutter/services.dart';

import 'package:device_info/device_info.dart';
import 'package:device_marketing_names/device_marketing_names.dart';
import 'package:mlperfbench_common/data/environment/environment_info.dart';
import 'package:mlperfbench_common/data/environment/os_enum.dart';
import 'package:process_run/shell.dart';

import 'package:mlperfbench/backend/bridge/ffi_cpuinfo.dart';

class DeviceInfo {
  final String modelCode;
  final String modelName;
  final String manufacturer;
  final EnvAndroidInfo? androidInfo;

  static late final String nativeLibraryPath;
  static late final DeviceInfo instance;
  static late final String cpuinfoSocName;

  DeviceInfo({
    required this.manufacturer,
    required this.modelCode,
    required this.modelName,
    required this.androidInfo,
  });

  static Future<void> staticInit() async {
    DeviceInfo.nativeLibraryPath = await _makeNativeLibraryPath();
    DeviceInfo.instance = await createFromEnvironment();
    DeviceInfo.cpuinfoSocName = getSocName();
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
      androidInfo: null,
    );
  }

  static Future<DeviceInfo> _makeAndroidInfo() async {
    final deviceInfo = await DeviceInfoPlugin().androidInfo;
    final deviceNames = DeviceMarketingNames();

    var shell = Shell();
    final socModel = await shell.run('getprop ro.soc.model');
    final socManufacturer = await shell.run('getprop ro.soc.manufacturer');

    final androidInfo = EnvAndroidInfo(
      propSocModel: socModel.outText,
      propSocManufacturer: socManufacturer.outText,
      buildBoard: deviceInfo.board,
    );

    return DeviceInfo(
      // deviceInfo.manufacturer is usually a human-readable string with proper capitalisation
      // deviceInfo.brand seems to be machine-readable string because it tends to be all lower-case letters
      manufacturer: deviceInfo.manufacturer,
      modelCode: deviceInfo.model,
      modelName: deviceNames.getSingleNameFromModel(
          DeviceType.android, deviceInfo.model),
      androidInfo: androidInfo,
    );
  }

  static Future<DeviceInfo> _makeWindowsInfo() async {
    return DeviceInfo(
      manufacturer: '',
      modelCode: 'Unknown PC',
      modelName: 'Unknown PC',
      androidInfo: null,
    );
  }

  static EnvironmentInfo get environmentInfo {
    return EnvironmentInfo(
      osName: OsName.fromJson(Platform.operatingSystem),
      osVersion: Platform.operatingSystemVersion,
      manufacturer: DeviceInfo.instance.manufacturer,
      modelCode: DeviceInfo.instance.modelCode,
      modelName: DeviceInfo.instance.modelName,
      socInfo: EnvSocInfo(
        cpuinfo: EnvCpuinfo(socName: DeviceInfo.cpuinfoSocName),
        androidInfo: DeviceInfo.instance.androidInfo,
      ),
    );
  }
}

const _androidChannel = MethodChannel('org.mlcommons.mlperfbench/android');
Future<String> _getNativeLibraryPath() async {
  return await _androidChannel.invokeMethod('getNativeLibraryPath') as String;
}
