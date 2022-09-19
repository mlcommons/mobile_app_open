// ignore_for_file: non_constant_identifier_names

import 'dart:ffi';

import 'package:ffi/ffi.dart';

import 'package:mlperfbench/backend/bridge/run_settings.dart';
import 'handle.dart';
import 'run_result.dart';

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
  external int dataset_offset;

  external Pointer<Utf8> scenario;

  external Pointer<Utf8> mode;
  @Int32()
  external int min_query_count;
  @Double()
  external double min_duration;
  @Int32()
  external int single_stream_expected_latency_ns;
  external Pointer<Utf8> output_dir;

  void set(RunSettings rs) {
    backend_model_path = rs.backend_model_path.toNativeUtf8();
    backend_lib_path = rs.backend_lib_path.toNativeUtf8();

    final backendSettings = rs.backend_settings.writeToBuffer();
    backend_settings_len = backendSettings.length;
    backend_settings_data = malloc.allocate<Uint8>(backendSettings.length);
    backend_settings_data
        .asTypedList(backendSettings.length)
        .setAll(0, backendSettings);

    backend_native_lib_path = rs.backend_native_lib_path.toNativeUtf8();

    dataset_type = rs.dataset_type;
    dataset_data_path = rs.dataset_data_path.toNativeUtf8();
    dataset_groundtruth_path = rs.dataset_groundtruth_path.toNativeUtf8();
    dataset_offset = rs.dataset_offset;
    scenario = rs.scenario.toNativeUtf8();

    mode = rs.mode.toNativeUtf8();
    min_query_count = rs.min_query_count;
    min_duration = rs.min_duration;
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

class _RunOut extends Struct {
  @Int32()
  external int ok;
  @Float()
  external double accuracyNormalized;
  external Pointer<Utf8> accuracyFormatted;
  @Float()
  external double accuracyNormalized2;
  external Pointer<Utf8> accuracyFormatted2;
  @Int32()
  external int num_samples;
  @Float()
  external double duration_ms;
  external Pointer<Utf8> backend_name;
  external Pointer<Utf8> backend_vendor;
  external Pointer<Utf8> accelerator_name;
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

RunResult runBenchmark(RunSettings rs) {
  var runIn = malloc.allocate<_RunIn>(sizeOf<_RunIn>());
  runIn.ref.set(rs);

  final startTime = DateTime.now();

  var runOut = _run(runIn);

  runIn.ref.free();
  malloc.free(runIn);

  if (runOut.address == 0) {
    throw '$_runName result: nullptr';
  }
  if (runOut.ref.ok != 1) {
    throw '$_runName result: runOut.ref.ok != 1';
  }

  var res = RunResult(
    accuracyNormalized: runOut.ref.accuracyNormalized,
    accuracyFormatted: runOut.ref.accuracyFormatted.toDartString(),
    accuracyNormalized2: runOut.ref.accuracyNormalized2,
    accuracyFormatted2: runOut.ref.accuracyFormatted2.toDartString(),
    numSamples: runOut.ref.num_samples,
    durationMs: runOut.ref.duration_ms,
    backendName: runOut.ref.backend_name.toDartString(),
    backendVendor: runOut.ref.backend_vendor.toDartString(),
    acceleratorName: runOut.ref.accelerator_name.toDartString(),
    startTime: startTime,
  );

  _free(runOut);

  return res;
}

int getQueryCounter() {
  return _getQuery();
}

int getDatasetSize() {
  return _getDatasetSize();
}
