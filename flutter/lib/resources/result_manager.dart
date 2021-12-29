import 'dart:convert';
import 'dart:io';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/benchmark_result.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;

class ResultManager {
  static const _jsonResultFileName = 'result.json';
  late final String _jsonResultPath;

  ResultManager(String applicationDirectory) {
    _jsonResultPath = '$applicationDirectory/$_jsonResultFileName';
  }

  // returns brief results in json
  Future<String> record(List<RunResult?> results) async {
    final resultContent = <Map<String, dynamic>>[];
    final briefResultContent = <Map<String, dynamic>>[];

    for (final result in results) {
      if (result != null) {
        final score =
            result.mode == BenchmarkMode.accuracy ? null : result.score;

        final benchmarkResult = {
          'benchmark_id': result.id,
          'configuration': {
            'runtime': '',
          },
          'score': score != null ? score.toString() : 'N/A',
          'accuracy': BenchmarkMode.performance_lite == result.mode
              ? 'N/A'
              : result.accuracy,
          // strings are used here to match android app behavior
          'min_duration': result.minDuration.toString(),
          'duration': result.durationMs.toString(),
          'min_samples': result.minSamples.toString(),
          'num_samples': result.numSamples.toString(),
          'shards_num': result.threadsNumber,
          'batch_size': result.batchSize,
          'mode': result.mode.toString(),
          'datetime': DateTime.now().toIso8601String(),
        };
        final benchmarkBriefResult = {
          'benchmark_id': result.id,
          'score': score,
          'accuracy': result.accuracy,
          'shards_num': result.threadsNumber,
          'batch_size': result.batchSize,
        };

        resultContent.add(benchmarkResult);
        briefResultContent.add(benchmarkBriefResult);
      }
    }
    await _write(resultContent);
    return JsonEncoder().convert(briefResultContent);
  }

  bool restoreResults(String briefResultsJson, List<Benchmark> benchmarks) {
    if (briefResultsJson == '') return false;

    try {
      for (final resultContent in jsonDecode(briefResultsJson)) {
        final result = resultContent as Map<String, dynamic>;
        final id = result['benchmark_id'] as String;
        final accuracy = result['accuracy'] as String?;
        final score = result['score'] as double?;
        final threadsNumber = result['shards_num'] as int?;
        final batchSize = result['batch_size'] as int?;
        final benchmark =
            benchmarks.singleWhere((benchmark) => benchmark.id == id);
        benchmark.accuracy = accuracy;
        benchmark.score = score;

        if (benchmark.modelConfig.scenario == 'Offline') {
          benchmark.benchmarkSetting.customSetting.add(
              pb.CustomSetting(id: 'batch_size', value: batchSize.toString()));
          benchmark.benchmarkSetting.customSetting.add(pb.CustomSetting(
              id: 'shards_num', value: threadsNumber.toString()));
          benchmark.benchmarkSetting.writeToBuffer();
        }
      }
    } catch (_) {
      return false;
    }
    return true;
  }

  Future<void> _write(List<Map<String, dynamic>> content) async {
    final jsonFile = File(_jsonResultPath);

    final jsonEncoder = JsonEncoder.withIndent('  ');
    var encoded = jsonEncoder.convert(content);
    // needed to match android behavior
    encoded = encoded.replaceAll('/', '\\/');
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
