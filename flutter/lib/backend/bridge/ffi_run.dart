import 'dart:ffi';

import 'package:ffi/ffi.dart';

import 'package:mlperfbench/backend/run_settings.dart';
import 'handle.dart';

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

class _RunOut extends Struct {
  @Int32()
  external int ok;
  @Float()
  external double latency;
  external Pointer<Utf8> accuracy;
  @Int32()
  external int num_samples;
  @Float()
  external double duration_ms;
  external Pointer<Utf8> backend_description;
}

class RunBenchmarkResult {
  final int ok;
  final double latency;
  final String accuracy;
  final int num_samples;
  final double duration_ms;
  final String backend_description;

  RunBenchmarkResult(this.ok, this.latency, this.accuracy, this.num_samples,
      this.duration_ms, this.backend_description);
}

const _runName = 'dart_ffi_run_benchmark';
const _freeName = 'dart_ffi_run_benchmark_free';

typedef _Run = Pointer<_RunOut> Function(Pointer<_RunIn>);
final _run = getBridgeHandle().lookupFunction<_Run, _Run>(_runName);

typedef _Free1 = Void Function(Pointer<_RunOut>);
typedef _Free2 = void Function(Pointer<_RunOut>);
final _free = getBridgeHandle().lookupFunction<_Free1, _Free2>(_freeName);

RunBenchmarkResult runBenchmark(RunSettings rs) {
  var runIn = malloc.allocate<_RunIn>(sizeOf<_RunIn>());
  runIn.ref.set(rs);

  var runOut = _run(runIn);

  runIn.ref.free();
  malloc.free(runIn);

  if (runOut.address == 0) {
    throw '$_runName result: nullptr';
  }

  var res = RunBenchmarkResult(
    runOut.ref.ok,
    runOut.ref.latency,
    runOut.ref.accuracy.toDartString(),
    runOut.ref.num_samples,
    runOut.ref.duration_ms,
    runOut.ref.backend_description.toDartString(),
  );

  _free(runOut);

  return res;
}
