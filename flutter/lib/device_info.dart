import 'dart:io';

import 'package:flutter/services.dart';

import 'package:device_info_plus/device_info_plus.dart';
import 'package:device_marketing_names/device_marketing_names.dart';
import 'package:mlperfbench_common/data/environment/env_android.dart';
import 'package:mlperfbench_common/data/environment/env_ios.dart';
import 'package:mlperfbench_common/data/environment/env_windows.dart';
import 'package:mlperfbench_common/data/environment/environment_info.dart';
import 'package:process_run/shell.dart';

import 'package:mlperfbench/backend/bridge/ffi_cpuinfo.dart';

class DeviceInfo {
  final EnvironmentInfo envInfo;
  final String nativeLibraryPath;

  static late final DeviceInfo instance;

  DeviceInfo({
    required this.envInfo,
    required this.nativeLibraryPath,
  });

  static Future<void> staticInit() async {
    instance = await createFromEnvironment();
  }

  static Future<DeviceInfo> createFromEnvironment() async {
    DeviceInfo.instance = await createFromEnvironment();

    return DeviceInfo(
      envInfo: await makeEnvInfo(),
      nativeLibraryPath: await _makeNativeLibraryPath(),
    );
  }

  static Future<EnvironmentInfo> makeEnvInfo() async {
    if (Platform.isAndroid) {
      return EnvironmentInfo.makeAndroid(info: await _makeAndroidInfo());
    } else if (Platform.isIOS) {
      return EnvironmentInfo.makeIos(info: await _makeIosInfo());
    } else if (Platform.isWindows) {
      return EnvironmentInfo.makeWIndows(info: await _makeWindowsInfo());
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

  static Future<EnvIos> _makeIosInfo() async {
    final deviceInfo = await DeviceInfoPlugin().iosInfo;
    final deviceNames = DeviceMarketingNames();
    final modelCode = deviceInfo.utsname.machine;
    String? modelName;
    var machine = deviceInfo.utsname.machine;
    if (machine != null) {
      modelName = deviceNames.getSingleNameFromModel(DeviceType.ios, machine);
    }
    return EnvIos(
      osVersion: Platform.operatingSystemVersion,
      modelCode: modelCode,
      modelName: modelName,
      socName: getSocName(),
    );
  }

  static Future<EnvAndroid> _makeAndroidInfo() async {
    final deviceInfo = await DeviceInfoPlugin().androidInfo;
    final deviceNames = DeviceMarketingNames();

    var shell = Shell();
    final propSocModel = await shell.run('getprop ro.soc.model');
    final propSocManufacturer = await shell.run('getprop ro.soc.manufacturer');

    final modelCode = deviceInfo.model;
    String? modelName;
    if (modelCode != null) {
      modelName =
          deviceNames.getSingleNameFromModel(DeviceType.android, modelCode);
    }
    return EnvAndroid(
      osVersion: Platform.operatingSystemVersion,
      // deviceInfo.manufacturer is usually a human-readable string with proper capitalisation
      // deviceInfo.brand seems to be machine-readable string because it tends to be all lower-case letters
      manufacturer: deviceInfo.manufacturer,
      modelCode: modelCode,
      modelName: modelName,
      boardCode: deviceInfo.board,
      procCpuinfoSocName: getSocName(),
      props: [
        EnvAndroidProp(
          type: 'soc_model',
          name: 'ro.soc.model',
          value: propSocModel.outText,
        ),
        EnvAndroidProp(
          type: 'soc_manufacturer',
          name: 'ro.soc.manufacturer',
          value: propSocManufacturer.outText,
        ),
      ],
    );
  }

  static Future<EnvWindows> _makeWindowsInfo() async {
    return EnvWindows(
      osVersion: Platform.operatingSystemVersion,
      cpuFullName: getSocName(),
    );
  }
}

const _androidChannel = MethodChannel('org.mlcommons.mlperfbench/android');
Future<String> _getNativeLibraryPath() async {
  return await _androidChannel.invokeMethod('getNativeLibraryPath') as String;
}
