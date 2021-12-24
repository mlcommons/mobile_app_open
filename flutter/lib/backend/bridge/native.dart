import 'dart:ffi';
import 'dart:io';

import 'package:flutter/services.dart';

import 'package:device_info/device_info.dart';
import 'package:ffi/ffi.dart';

import 'package:mlcommons_ios_app/backend/run_settings.dart';
import 'package:mlcommons_ios_app/protos/backend_setting.pb.dart' as pb;
import 'package:mlcommons_ios_app/protos/mlperf_task.pb.dart' as pb;

part 'native.run.dart';
part 'native.match.dart';
part 'native.config.dart';

const _androidChannel = MethodChannel('org.mlcommons.mlperfbench/android');
Future<String> getNativeLibraryPath() async {
  return await _androidChannel.invokeMethod('getNativeLibraryPath') as String;
}

DynamicLibrary _getBridgeLibraryHandle() {
  if (Platform.isWindows) {
    return DynamicLibrary.open('./libs/backend_bridge.dll');
  } else if (Platform.isIOS) {
    return DynamicLibrary.process();
  } else if (Platform.isAndroid) {
    return DynamicLibrary.open('libbackendbridge.so');
  } else {
    throw 'unsupported platform';
  }
}

final DynamicLibrary _bridge = _getBridgeLibraryHandle();
