import 'dart:convert';
import 'dart:io';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'utils.dart';

class _ExtendedResultList {
  final List<ExtendedResult> list;

  _ExtendedResultList(this.list);

  static _ExtendedResultList fromJson(List<dynamic> json) {
    final list = <ExtendedResult>[];
    for (var item in json) {
      try {
        list.add(ExtendedResult.fromJson(item as Map<String, dynamic>));
      } catch (e) {
        // for now just skip result that fail to decode
      }
    }
    return _ExtendedResultList(list);
  }

  List<dynamic> toJson() {
    var result = <dynamic>[];
    for (var item in list) {
      result.add(item);
    }
    return result;
  }
}

class ResultManager {
  static const _resultsFileName = 'results.json';
  final File jsonFile;
  _ExtendedResultList _results = _ExtendedResultList([]);

  ResultManager(String applicationDirectory)
      : jsonFile = File('$applicationDirectory/$_resultsFileName');

  Future<void> init() async {
    try {
      _results = await _restoreResults();
    } catch (e) {}
  }

  Future<void> _saveResults() async {
    await jsonFile.writeAsString(jsonToStringIndented(_results));
  }

  Future<_ExtendedResultList> _restoreResults() async {
    if (!await jsonFile.exists()) {
      return _ExtendedResultList([]);
    }

    return _ExtendedResultList.fromJson(
        jsonDecode(await jsonFile.readAsString()) as List<dynamic>);
  }

  Future<void> deleteResults() async {
    if (await jsonFile.exists()) {
      await jsonFile.delete();
    }
  }

  Future<void> saveResult(ExtendedResult value) async {
    _results.list.add(value);
    await _saveResults();
  }

  ExtendedResult getLastResult() {
    return _results.list.last;
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
        accuracy: runResult.accuracy,
        backendName: export.backendInfo.name,
        acceleratorName: export.backendInfo.accelerator,
        batchSize: export.backendSettingsInfo.batchSize,
        threadsNumber: threadsNumber,
        validity: runResult.loadgenValidity);
  }
}
