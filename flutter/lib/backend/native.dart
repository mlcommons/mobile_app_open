import 'dart:ffi';
import 'dart:io';

import 'package:flutter/services.dart';

import 'package:device_info/device_info.dart';
import 'package:ffi/ffi.dart';

import 'package:mlcommons_ios_app/protos/backend_setting.pb.dart' as pb;
import 'package:mlcommons_ios_app/protos/mlperf_task.pb.dart' as pb;
import 'bridge.dart';

const androidChannel = MethodChannel('org.mlcommons.mlperfbench/android');
Future<String> getNativeLibraryPath() async {
  return await androidChannel.invokeMethod('getNativeLibraryPath') as String;
}

class _RunBackend4in extends Struct {
  external Pointer<Utf8> backend_model_path;
  external Pointer<Utf8> backend_lib_path;
  external Pointer<Uint8> backend_settings_data;
  @Int32()
  external int backend_settings_len;
  external Pointer<Utf8> backend_native_lib_path;

  @Int32()
  external int dataset_type;
  external Pointer<Utf8> dataset_data_path;
  external Pointer<Utf8> dataset_groundtruth_path;
  @Int32()
  external int dataset_offset;

  external Pointer<Utf8> scenario;
  @Int32()
  external int batch;

  external Pointer<Utf8> mode;
  @Int32()
  external int min_query_count;
  @Int32()
  external int min_duration;
  external Pointer<Utf8> output_dir;

  void set(RunSettings rs) {
    backend_model_path = rs.backend_model_path.toNativeUtf8();
    backend_lib_path = rs.backend_lib_path.toNativeUtf8();

    backend_settings_len = rs.backend_settings.length;
    backend_settings_data = malloc.allocate<Uint8>(rs.backend_settings.length);
    backend_settings_data
        .asTypedList(rs.backend_settings.length)
        .setAll(0, rs.backend_settings);

    backend_native_lib_path = rs.backend_native_lib_path.toNativeUtf8();

    dataset_type = rs.dataset_type;
    dataset_data_path = rs.dataset_data_path.toNativeUtf8();
    dataset_groundtruth_path = rs.dataset_groundtruth_path.toNativeUtf8();
    dataset_offset = rs.dataset_offset;
    scenario = rs.scenario.toNativeUtf8();

    batch = rs.batch;
    mode = rs.mode.toNativeUtf8();
    min_query_count = rs.min_query_count;

    min_duration = rs.min_duration;

    output_dir = rs.output_dir.toNativeUtf8();
  }

  void free() {
    malloc.free(backend_model_path);
    malloc.free(backend_lib_path);
    malloc.free(backend_settings_data);
    malloc.free(backend_native_lib_path);
    malloc.free(dataset_data_path);
    malloc.free(dataset_groundtruth_path);
    malloc.free(scenario);
    malloc.free(mode);
    malloc.free(output_dir);
  }
}

class _RunBackend4out extends Struct {
  @Int32()
  external int ok;
  @Float()
  external double latency;
  external Pointer<Utf8> accuracy;
  @Int32()
  external int num_samples;
  @Float()
  external double duration_ms;
}

typedef _RunBenchmark = Pointer<_RunBackend4out> Function(
    Pointer<_RunBackend4in>);
typedef _RunBenchmarkFree1 = Void Function(Pointer<_RunBackend4out>);
typedef _RunBenchmarkFree2 = void Function(Pointer<_RunBackend4out>);

class BackendRun4results {
  final int ok;
  final double latency;
  final String accuracy;
  final int num_samples;
  final double duration_ms;

  BackendRun4results(
      this.ok, this.latency, this.accuracy, this.num_samples, this.duration_ms);
}

BackendRun4results runBenchmark(RunSettings rs) {
  var backend_in = malloc.allocate<_RunBackend4in>(sizeOf<_RunBackend4in>());

  backend_in.ref.set(rs);
  var backend_out = _Native.dart_ffi_run_benchmark(backend_in);
  backend_in.ref.free();

  if (backend_out.address == 0) {
    throw '_Native.dart_ffi_run_benchmark failed';
  }

  var res = BackendRun4results(
    backend_out.ref.ok,
    backend_out.ref.latency,
    backend_out.ref.accuracy.toDartString(),
    backend_out.ref.num_samples,
    backend_out.ref.duration_ms,
  );

  malloc.free(backend_in);

  _Native.dart_ffi_run_benchmark_free(backend_out);

  return res;
}

class _BackendMatchResult extends Struct {
  @Int32()
  external int matches;
  external Pointer<Utf8> error_message;

  @Int32()
  external int pbdata_size;
  external Pointer<Uint8> pbdata;
}

typedef _BackendMatch = Pointer<_BackendMatchResult> Function(
    Pointer<Utf8> lib_path, Pointer<Utf8> manufacturer, Pointer<Utf8> model);

typedef _BackendMatchFree1 = Void Function(Pointer<_BackendMatchResult>);
typedef _BackendMatchFree2 = void Function(Pointer<_BackendMatchResult>);

Future<pb.BackendSetting?> backendMatch(String lib_path) async {
  Pointer<Utf8> model;
  Pointer<Utf8> manufacturer;

  if (Platform.isIOS) {
    final deviceInfo = DeviceInfoPlugin();
    final iosInfo = await deviceInfo.iosInfo;

    manufacturer = 'Apple'.toNativeUtf8();
    model = iosInfo.name.toNativeUtf8();
  } else if (Platform.isWindows) {
    manufacturer = 'Microsoft'.toNativeUtf8();
    model = 'Unknown PC'.toNativeUtf8();
  } else if (Platform.isAndroid) {
    final deviceInfo = await DeviceInfoPlugin().androidInfo;

    manufacturer = deviceInfo.manufacturer.toNativeUtf8();
    model = deviceInfo.model.toNativeUtf8();
  } else {
    throw 'Could not define platform';
  }

  final lib_path_ = lib_path.toNativeUtf8();
  final matchResult = _Native.backend_match(lib_path_, manufacturer, model);
  if (matchResult.address == 0) {
    return null;
  }
  try {
    if (matchResult.ref.matches != 0 && matchResult.ref.pbdata.address != 0) {
      final view =
          matchResult.ref.pbdata.asTypedList(matchResult.ref.pbdata_size);
      return pb.BackendSetting.fromBuffer(view);
    }
  } finally {
    malloc.free(lib_path_);
    malloc.free(manufacturer);
    malloc.free(model);
    _Native.backend_match_free(matchResult);
  }

  return null;
}

class _MLPerfConfigResult extends Struct {
  @Int32()
  external int size;

  external Pointer<Uint8> data;
}

typedef _MLPerfConfig = Pointer<_MLPerfConfigResult> Function(
    Pointer<Utf8> pb_content);

typedef _MLPerfConfigFree1 = Void Function(Pointer<_MLPerfConfigResult>);
typedef _MLPerfConfigFree2 = void Function(Pointer<_MLPerfConfigResult>);

pb.MLPerfConfig getMLPerfConfig(String pbtxtContent) {
  final pbtxtContent_ = pbtxtContent.toNativeUtf8();
  final pbdata = _Native.mlperf_config(pbtxtContent_);

  pb.MLPerfConfig? config;

  try {
    if (pbdata.ref.data.address != 0) {
      final convertedContent = pbdata.ref.data.asTypedList(pbdata.ref.size);
      config = pb.MLPerfConfig.fromBuffer(convertedContent);
    }
  } finally {
    _Native.mlperf_config_free(pbdata);
    malloc.free(pbtxtContent_);
  }
  if (config == null) {
    throw 'Could not load content from pbtxt file';
  }

  return config;
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
