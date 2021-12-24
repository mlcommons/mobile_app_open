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

const androidChannel = MethodChannel('org.mlcommons.mlperfbench/android');
Future<String> getNativeLibraryPath() async {
  return await androidChannel.invokeMethod('getNativeLibraryPath') as String;
}

class _Native {
  static final DynamicLibrary lib = _getBackendBridgeLibraryHandle();
  static final dart_ffi_run_benchmark = lib
      .lookupFunction<_RunBenchmark, _RunBenchmark>('dart_ffi_run_benchmark');
  static final dart_ffi_run_benchmark_free =
      lib.lookupFunction<_RunBenchmarkFree1, _RunBenchmarkFree2>(
          'dart_ffi_run_benchmark_free');

  static final backend_match = lib
      .lookupFunction<_BackendMatch, _BackendMatch>('dart_ffi_backend_match');
  static final backend_match_free =
      lib.lookupFunction<_BackendMatchFree1, _BackendMatchFree2>(
          'dart_ffi_backend_match_free');

  static final mlperf_config = lib
      .lookupFunction<_MLPerfConfig, _MLPerfConfig>('dart_ffi_mlperf_config');
  static final mlperf_config_free =
      lib.lookupFunction<_MLPerfConfigFree1, _MLPerfConfigFree2>(
          'dart_ffi_mlperf_config_free');

  static DynamicLibrary _getBackendBridgeLibraryHandle() {
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
}
