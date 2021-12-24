part of 'native.dart';

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

class BackendRun4results {
  final int ok;
  final double latency;
  final String accuracy;
  final int num_samples;
  final double duration_ms;

  BackendRun4results(
      this.ok, this.latency, this.accuracy, this.num_samples, this.duration_ms);
}

typedef _RunBenchmark = Pointer<_RunBackend4out> Function(
    Pointer<_RunBackend4in>);
typedef _RunBenchmarkFree1 = Void Function(Pointer<_RunBackend4out>);
typedef _RunBenchmarkFree2 = void Function(Pointer<_RunBackend4out>);

final _dart_ffi_run_benchmark = _bridge
    .lookupFunction<_RunBenchmark, _RunBenchmark>('dart_ffi_run_benchmark');
final _dart_ffi_run_benchmark_free =
    _bridge.lookupFunction<_RunBenchmarkFree1, _RunBenchmarkFree2>(
        'dart_ffi_run_benchmark_free');

BackendRun4results runBenchmark(RunSettings rs) {
  var backend_in = malloc.allocate<_RunBackend4in>(sizeOf<_RunBackend4in>());

  backend_in.ref.set(rs);
  var backend_out = _dart_ffi_run_benchmark(backend_in);
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

  _dart_ffi_run_benchmark_free(backend_out);

  return res;
}
