import 'dart:convert';
import 'dart:io';

import 'package:intl/intl.dart';
import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'utils.dart';

class ResultManager {
  List<ExtendedResult> results = [];

  static const _resultsDirName = 'results';
  final Directory _resultsDir;
  final List<File> _resultsFiles = [];

  ResultManager(String applicationDirectory)
      : _resultsDir = Directory('$applicationDirectory/$_resultsDirName');

  Future<void> init() async {
    if (!await _resultsDir.exists()) {
      await _resultsDir.create(recursive: true);
    }
    final List<FileSystemEntity> entities = await _resultsDir.list().toList();
    final Iterable<File> files = entities.whereType<File>();
    if (files.isEmpty) {
      print('No file found in $_resultsDir');
    }
    for (var file in files) {
      final json =
          jsonDecode(await file.readAsString()) as Map<String, dynamic>;
      try {
        results.add(ExtendedResult.fromJson(json));
        _resultsFiles.add(file);
      } catch (e, trace) {
        print('Unable to parse result from [$file]: $e');
        print(trace);
      }
    }
  }

  Future<void> removeSelected(List<bool> selected) async {
    if (selected.length != results.length) {
      throw 'selected.length != results.length';
    }
    if (_resultsFiles.length != results.length) {
      throw '_resultsFiles.length != results.length';
    }
    for (int i = 0; i < results.length; i++) {
      if (selected[i]) {
        results.removeAt(i);
        await _resultsFiles[i].delete();
      }
    }
  }

  Future<void> saveResult(ExtendedResult value) async {
    results.add(value);
    final DateFormat formatter = DateFormat('yyyy-MM-dd_HHmmss');
    final String formatted = formatter.format(DateTime.now());
    final jsonFile = File('${_resultsDir.path}/$formatted.json');
    await jsonFile.writeAsString(jsonToStringIndented(value));
    print('Result saved to $jsonFile');
  }

  ExtendedResult getLastResult() {
    return results.last;
  }

  void restoreResults(
      List<BenchmarkExportResult> results, List<Benchmark> benchmarks) {
    for (final exportResult in results) {
      final benchmark = benchmarks
          .singleWhere((benchmark) => benchmark.id == exportResult.benchmarkId);

      benchmark.performanceModeResult =
          _parseExportResult(exportResult, exportResult.performanceRun);
      benchmark.accuracyModeResult =
          _parseExportResult(exportResult, exportResult.accuracyRun);
    }
  }

  BenchmarkResult? _parseExportResult(
      BenchmarkExportResult export, BenchmarkRunResult? runResult) {
    if (runResult == null) {
      return null;
    }

    return BenchmarkResult(
      throughput: runResult.throughput ?? 0.0,
      accuracy: runResult.accuracy,
      accuracy2: runResult.accuracy2,
      backendName: export.backendInfo.backendName,
      acceleratorName: export.backendInfo.acceleratorName,
      batchSize: export.backendSettings.batchSize,
      validity: runResult.loadgenInfo?.validity ?? false,
    );
  }
}
