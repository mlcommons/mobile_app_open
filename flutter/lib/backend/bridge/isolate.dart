import 'dart:async';
import 'dart:isolate';

import 'package:mlperfbench/backend/bridge/ffi_run.dart';
import 'package:mlperfbench/backend/run_settings.dart';
import 'package:mlperfbench/benchmark/run_result.dart';

class BridgeIsolate {
  late final SendPort _runSendPort;
  Completer<RunResult> _nextResult = Completer();

  BridgeIsolate._();

  static Future<BridgeIsolate> create() async {
    final receivePort = ReceivePort();
    await Isolate.spawn(_isolateRun, receivePort.sendPort);

    var res = BridgeIsolate._();

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
      var r = runBenchmark(rs);
      sendPort.send(RunResult(
        accuracy: r.accuracy,
        numSamples: r.num_samples,
        durationMs: r.duration_ms,
        backendDescription: r.backend_description,
        datasetMode: rs.dataset_mode,
        backendMode: rs.mode,
        score: r.score,
      ));
    }
  }
}
