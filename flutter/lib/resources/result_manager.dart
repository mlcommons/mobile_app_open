import 'dart:convert';
import 'dart:io';

import 'package:intl/intl.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/data/extended_result.dart';
import 'package:mlperfbench/data/result_filter.dart';
import 'package:mlperfbench/data/result_sort.dart';
import 'package:mlperfbench/data/results/benchmark_result.dart';
import 'package:mlperfbench/firebase/firebase_manager.dart';
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

  ResultFilter resultFilter = ResultFilter();
  ResultSort resultSort = ResultSort();
  final List<ExtendedResult> localResults = [];
  final List<ExtendedResult> remoteResults = [];
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
        localResults.add(ExtendedResult.fromJson(json));
        _resultsFiles.add(file);
      } catch (e, trace) {
        print('Unable to parse result from [$file]: $e');
        print(trace);
      }
    }
  }

  Future<void> deleteResult(ExtendedResult result) async {
    final idx = localResults.indexOf(result);
    localResults.removeAt(idx);
    await _resultsFiles[idx].delete();
  }

  Future<void> saveResult(ExtendedResult result) async {
    localResults.add(result);
    final DateFormat formatter = DateFormat('yyyy-MM-ddTHH-mm-ss');
    final String datetime = formatter.format(result.meta.creationDate);
    final resultFile =
        File('${_resultsDir.path}/${datetime}_${result.meta.uuid}.json');
    await resultFile.writeAsString(jsonToStringIndented(result));
    final submissionFile = File('${_resultsDir.path}/$_submissionFileName');
    await submissionFile.writeAsString(jsonToStringIndented(result));
    print('Result saved to $resultFile and $submissionFile');
  }

  File getSubmissionFile() {
    return File('${_resultsDir.path}/$_submissionFileName');
  }

  Future<void> deleteLastResult() async {
    if (localResults.isNotEmpty) {
      await deleteResult(localResults.last);
    }
    final submissionFile = getSubmissionFile();
    if (await submissionFile.exists()) {
      await submissionFile.delete();
    }
  }

  ExtendedResult getLastResult() {
    return localResults.last;
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
      delegateName: export.backendSettings.delegate,
      batchSize: export.backendSettings.batchSize,
      validity: runResult.loadgenInfo?.validity ?? false,
    );
  }

  Future<void> uploadLastResult() async {
    await FirebaseManager.instance.uploadResult(localResults.last);
  }

  Future<void> downloadRemoteResults() async {
    final excluded = localResults.map((e) => e.meta.uuid).toList();
    final downloaded = await FirebaseManager.instance.downloadResults(excluded);
    remoteResults.clear();
    remoteResults.addAll(downloaded);
  }

  Future<void> clearRemoteResult() async {
    remoteResults.clear();
  }
}
