import 'dart:convert';
import 'dart:io';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';

class ResultManager {
  static const _lastResultFileName = 'result.json';
  late final String _lastResultPath;

  ResultManager(String applicationDirectory) {
    _lastResultPath = '$applicationDirectory/$_lastResultFileName';
  }

  Future<void> writeLastResult(ExtendedResult results) async {
    final jsonFile = File(_lastResultPath);

    final jsonEncoder = JsonEncoder.withIndent('  ');
    var encoded = jsonEncoder.convert(results);
    await jsonFile.writeAsString(encoded);
  }

  Future<String> readLastResult() async {
    final file = File(_lastResultPath);

    if (await file.exists()) {
      return file.readAsString();
    }

    return '';
  }

  Future<void> deleteLastResult() async {
    var resultsFile = File(_lastResultPath);
    if (await resultsFile.exists()) {
      await resultsFile.delete();
    }
  }

  void restoreResults(
      List<BenchmarkExportResult> results, List<Benchmark> benchmarks) {
    for (final exportResult in results) {
      final benchmark = benchmarks
          .singleWhere((benchmark) => benchmark.id == exportResult.benchmarkId);

      benchmark.performanceModeResult =
          _parseExportResult(exportResult, exportResult.performance);
      benchmark.accuracyModeResult =
          _parseExportResult(exportResult, exportResult.accuracy);
    }
  }

  BenchmarkResult? _parseExportResult(
      BenchmarkExportResult export, BenchmarkRunResult? runResult) {
    if (runResult == null) {
      return null;
    }

    var threadsNumber = 0;
    for (var item in export.backendSettingsInfo.extraSettings.list) {
      if (item.id == 'shards_num') {
        threadsNumber = int.parse(item.value);
        break;
      }
    }

    return BenchmarkResult(
        throughput: runResult.throughput ?? 0.0,
        accuracy: runResult.accuracy?.toString() ?? '',
        backendName: export.backendInfo.name,
        acceleratorName: export.backendInfo.accelerator,
        batchSize: export.backendSettingsInfo.batchSize,
        threadsNumber: threadsNumber,
        validity: runResult.loadgenValidity);
  }
}
