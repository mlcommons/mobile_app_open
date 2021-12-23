import 'dart:async';
import 'dart:io' show Platform;
import 'dart:isolate';
import 'dart:typed_data';

import 'package:mlcommons_ios_app/backend/native.dart' as native_backend;
import 'package:mlcommons_ios_app/benchmark/benchmark.dart';
import 'package:mlcommons_ios_app/benchmark/benchmark_result.dart';

part 'backends_list.gen.dart';

List<String> getBackendsList() {
  if (Platform.isIOS) {
    // on iOS backend is statically linked
    return [];
  } else if (Platform.isWindows || Platform.isAndroid) {
    return _backendsList;
  } else {
    throw 'current platform is unsupported';
  }
}

class RunSettings {
  final String backend_model_path;
  final String backend_lib_path;
  final Uint8List backend_settings;
  final String backend_native_lib_path;
  final int dataset_type; // 0: Imagenet; 1: Coco; 2: Squad; 3: Ade20k
  final String dataset_data_path;
  final String dataset_groundtruth_path;
  final int dataset_offset;
  final String scenario;
  final int batch;
  final int batch_size;
  final int threads_number;
  final String mode; // Submission/Accuracy/Performance
  final int min_query_count;
  final int min_duration;
  final String output_dir;
  final String benchmark_id;
  final DatasetMode dataset_mode;

  RunSettings({
    required this.backend_model_path,
    required this.backend_lib_path,
    required this.backend_settings,
    required this.backend_native_lib_path,
    required this.dataset_type, // 0: Imagenet, 1: Coco, 2: Squad, 3: Ade20k
    required this.dataset_data_path,
    required this.dataset_groundtruth_path,
    required this.dataset_offset,
    required this.scenario,
    required this.batch,
    required this.batch_size,
    required this.threads_number,
    required this.mode, // Submission/Accuracy/Performance
    required this.min_query_count,
    required this.min_duration,
    required this.output_dir,
    required this.benchmark_id,
    required this.dataset_mode,
  });
}

class BackendBridge {
  late final SendPort _runSendPort;
  Completer<RunResult> _nextResult = Completer();

  BackendBridge._();

  static Future<BackendBridge> create() async {
    final receivePort = ReceivePort();
    await Isolate.spawn(_isolateRun, receivePort.sendPort);

    var res = BackendBridge._();

    var sendPortCompleter = Completer<SendPort>();

    receivePort.listen((message) {
      if (message is SendPort) {
        sendPortCompleter.complete(message);
      }
      if (message is RunResult) {
        if (res._nextResult.isCompleted) {
          throw 'unexpected completed run result';
        }
        res._nextResult.complete(message);
      }
    });

    res._runSendPort = await sendPortCompleter.future;

    return res;
  }

  Future<RunResult> run(RunSettings rs) async {
    _runSendPort.send(rs);
    var res = await _nextResult.future;
    _nextResult = Completer();
    return res;
  }

  static void _isolateRun(SendPort sendPort) async {
    var port = ReceivePort();
    sendPort.send(port.sendPort);
    await for (var rs in port.cast<RunSettings>()) {
      var r = native_backend.runBenchmark(rs);
      sendPort.send(RunResult(
          rs.benchmark_id,
          r.accuracy,
          r.num_samples,
          rs.min_query_count,
          r.duration_ms,
          rs.min_duration,
          rs.threads_number,
          rs.batch_size,
          rs.dataset_mode,
          rs.mode,
          r.latency,
          rs.scenario));
    }
  }
}
