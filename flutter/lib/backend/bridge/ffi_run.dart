// ignore_for_file: non_constant_identifier_names

import 'dart:ffi';

import 'package:ffi/ffi.dart';

import 'package:mlperfbench/backend/bridge/handle.dart';
import 'package:mlperfbench/backend/bridge/run_result.dart';
import 'package:mlperfbench/backend/bridge/run_settings.dart';
import 'package:mlperfbench/data/results/benchmark_result.dart';

class _RunIn extends Struct {
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
  external int model_offset;
  @Int32()
  external int model_num_classes;
  @Int32()
  external int model_image_width;
  @Int32()
  external int model_image_height;

  external Pointer<Utf8> scenario;

  external Pointer<Utf8> mode;
  @Int32()
  external int batch_size;
  @Int32()
  external int min_query_count;
  @Double()
  external double min_duration;
  @Double()
  external double max_duration;
  @Int32()
  external int single_stream_expected_latency_ns;
  external Pointer<Utf8> output_dir;

  void set(RunSettings rs) {
    backend_model_path = rs.backend_model_path.toNativeUtf8();
    final libPath = libPathFromName(rs.backend_lib_name);
    backend_lib_path = libPath.toNativeUtf8();

    final backendSettings = rs.backend_settings.writeToBuffer();
    backend_settings_len = backendSettings.length;
    backend_settings_data = calloc<Uint8>(backendSettings.length);
    backend_settings_data
        .asTypedList(backendSettings.length)
        .setAll(0, backendSettings);

    backend_native_lib_path = rs.backend_native_lib_path.toNativeUtf8();

    dataset_type = rs.dataset_type;
    dataset_data_path = rs.dataset_data_path.toNativeUtf8();
    dataset_groundtruth_path = rs.dataset_groundtruth_path.toNativeUtf8();
    model_offset = rs.model_offset;
    model_num_classes = rs.model_num_classes;
    model_image_width = rs.model_image_width;
    model_image_height = rs.model_image_height;
    scenario = rs.scenario.toNativeUtf8();

    mode = rs.mode.toNativeUtf8();
    batch_size = rs.batch_size;
    min_query_count = rs.min_query_count;
    min_duration = rs.min_duration;
    max_duration = rs.max_duration;
    single_stream_expected_latency_ns = rs.single_stream_expected_latency_ns;

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

class _RunOutAccuracy extends Struct {
  @Float()
  external double normalized;
  external Pointer<Utf8> formatted;

  Accuracy toAccuracy() {
    return Accuracy(
      normalized: normalized,
      formatted: formatted.toDartString(),
    );
  }
}

class _RunOut extends Struct {
  @Bool()
  external bool runOk;
  external Pointer<_RunOutAccuracy> accuracy1;
  external Pointer<_RunOutAccuracy> accuracy2;
  @Int32()
  external int num_samples;
  @Float()
  external double duration;
  external Pointer<Utf8> backend_name;
  external Pointer<Utf8> backend_vendor;
  external Pointer<Utf8> accelerator_name;

  NativeRunResult toRunResult(DateTime startTime) {
    return NativeRunResult(
      accuracy1: accuracy1.address == 0 ? null : accuracy1.ref.toAccuracy(),
      accuracy2: accuracy2.address == 0 ? null : accuracy2.ref.toAccuracy(),
      numSamples: num_samples,
      duration: duration,
      backendName: backend_name.toDartString(),
      backendVendor: backend_vendor.toDartString(),
      acceleratorName: accelerator_name.toDartString(),
      startTime: startTime,
    );
  }
}

const _runName = 'dart_ffi_run_benchmark';
const _freeName = 'dart_ffi_run_benchmark_free';
const _getQueryName = 'dart_ffi_get_query_counter';
const _getDatasetSizeName = 'dart_ffi_get_dataset_size';

typedef _Run = Pointer<_RunOut> Function(Pointer<_RunIn>);
final _run = getBridgeHandle().lookupFunction<_Run, _Run>(_runName);

typedef _Free1 = Void Function(Pointer<_RunOut>);
typedef _Free2 = void Function(Pointer<_RunOut>);
final _free = getBridgeHandle().lookupFunction<_Free1, _Free2>(_freeName);

typedef _GetQuery1 = Int32 Function();
typedef _GetQuery2 = int Function();
final _getQuery =
    getBridgeHandle().lookupFunction<_GetQuery1, _GetQuery2>(_getQueryName);
final _getDatasetSize = getBridgeHandle()
    .lookupFunction<_GetQuery1, _GetQuery2>(_getDatasetSizeName);

NativeRunResult runBenchmark(RunSettings rs) {
  final startTime = DateTime.now();

  late final Pointer<_RunIn> runIn;
  late final Pointer<_RunOut> runOut;
  try {
    runIn = calloc<_RunIn>(sizeOf<_RunIn>());
    runIn.ref.set(rs);

    runOut = _run(runIn);

    if (runOut.address == 0) {
      throw '$_runName result: nullptr';
    }
  } finally {
    runIn.ref.free();
    calloc.free(runIn);
  }

  try {
    if (!runOut.ref.runOk) {
      throw '$_runName result: !runOut.ref.runOk';
    }

    return runOut.ref.toRunResult(startTime);
  } finally {
    _free(runOut);
  }
}

int getQueryCounter() => _getQuery();

int getDatasetSize() => _getDatasetSize();
