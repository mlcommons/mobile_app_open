import 'dart:convert';
import 'dart:io';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';

class ResultManager {
  static const _jsonResultFileName = 'result.json';
  late final String _jsonResultPath;

  ResultManager(String applicationDirectory) {
    _jsonResultPath = '$applicationDirectory/$_jsonResultFileName';
  }

  Future<void> writeResults(ExtendedResult results) async {
    await _write(results.toJson());
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

  Future<void> _write(dynamic content) async {
    final jsonFile = File(_jsonResultPath);

    final jsonEncoder = JsonEncoder.withIndent('  ');
    var encoded = jsonEncoder.convert(content);
    await jsonFile.writeAsString(encoded);
  }

  Future<String> read() async {
    final file = File(_jsonResultPath);

    if (await file.exists()) {
      return file.readAsString();
    }

    return '';
  }

  Future<void> delete() async {
    var resultsFile = File(_jsonResultPath);
    if (await resultsFile.exists()) {
      await resultsFile.delete();
    }
  }
}
