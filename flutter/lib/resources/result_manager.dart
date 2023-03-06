import 'dart:convert';
import 'dart:io';

import 'package:intl/intl.dart';
import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/result_filter.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/resources/utils.dart';

class ResultManager {
  static const _resultsDirName = 'results';

  // A file named `result.json` is expected for the submission.
  // See https://github.com/mlcommons/mobile_open/blob/main/rules/submissionRuleV2_0.adoc
  static const _submissionFileName = 'result.json';

  static Future<ResultManager> create(String applicationDirectory) async {
    var resultManager = ResultManager._create(applicationDirectory);
    await resultManager._loadResultsFromFiles();
    return resultManager;
  }

  final List<ExtendedResult> results = [];
  ResultFilter resultFilter = ResultFilter();
  final List<File> _resultsFiles = [];
  late final Directory _resultsDir;

  ResultManager._create(String applicationDirectory) {
    _resultsDir = Directory('$applicationDirectory/$_resultsDirName');
  }

  Future<void> _loadResultsFromFiles() async {
    if (!await _resultsDir.exists()) {
      await _resultsDir.create(recursive: true);
    }
    final List<FileSystemEntity> entities = await _resultsDir.list().toList();
    final Iterable<File> files = entities.whereType<File>();
    if (files.isEmpty) {
      print('No file found in $_resultsDir');
    }
    for (var file in files) {
      if (file.uri.pathSegments.last == _submissionFileName) {
        continue;
      }
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

  Future<void> deleteResult(ExtendedResult result) async {
    final idx = results.indexOf(result);
    results.removeAt(idx);
    await _resultsFiles[idx].delete();
  }

  Future<void> saveResult(ExtendedResult result) async {
    results.add(result);
    final DateFormat formatter = DateFormat('yyyy-MM-ddTHH-mm-ss');
    final String datetime = formatter.format(result.meta.creationDate);
    final resultFile =
        File('${_resultsDir.path}/${datetime}_${result.meta.uuid}.json');
    await resultFile.writeAsString(jsonToStringIndented(result));
    final submissionFile = File('${_resultsDir.path}/$_submissionFileName');
    await submissionFile.writeAsString(jsonToStringIndented(result));
    print('Result saved to $resultFile and $submissionFile');
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
      throughput: runResult.throughput,
      accuracy: runResult.accuracy,
      accuracy2: runResult.accuracy2,
      backendName: export.backendInfo.backendName,
      acceleratorName: export.backendInfo.acceleratorName,
      batchSize: export.backendSettings.batchSize,
      validity: runResult.loadgenInfo?.validity ?? false,
    );
  }
}
