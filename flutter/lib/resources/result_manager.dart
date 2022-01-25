import 'dart:convert';
import 'dart:io';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/benchmark_result.dart';

class _BriefResult {
  static const _tagId = 'benchmark_id';
  static const _tagPerformance = 'performance';
  static const _tagAccuracy = 'accuracy';

  final String id;
  final BenchmarkResult? performance;
  final BenchmarkResult? accuracy;

  _BriefResult(
      {required this.id, required this.performance, required this.accuracy});

  Map<String, dynamic> toJson() => {
        _tagId: id,
        _tagPerformance: performance,
        _tagAccuracy: accuracy,
      };

  _BriefResult.fromJson(Map<String, dynamic> jsonMap)
      : id = jsonMap[_tagId] as String,
        performance = BenchmarkResult.fromJson(
            jsonMap[_tagPerformance] as Map<String, dynamic>?),
        accuracy = BenchmarkResult.fromJson(
            jsonMap[_tagAccuracy] as Map<String, dynamic>?);
}

class _FullResult {
  final String id;
  final String score;
  final String accuracy;
  // numeric values are saved as string to match old android app behavior
  final String minDuration;
  final String duration;
  final String minSamples;
  final String numSamples;
  final int shardsNum;
  final int batchSize;
  final String mode;
  final String datetime;
  final String backendDescription;

  _FullResult(
      {required this.id,
      required this.score,
      required this.accuracy,
      required this.minDuration,
      required this.duration,
      required this.minSamples,
      required this.numSamples,
      required this.shardsNum,
      required this.batchSize,
      required this.mode,
      required this.datetime,
      required this.backendDescription});

  Map<String, dynamic> toJsonMap() {
    return {
      'benchmark_id': id,
      'configuration': {
        'runtime': '',
      },
      'score': score,
      'accuracy': accuracy,
      'min_duration': minDuration,
      'duration': duration,
      'min_samples': minSamples,
      'num_samples': numSamples,
      'shards_num': shardsNum,
      'batch_size': batchSize,
      'mode': mode,
      'datetime': datetime,
      'backendDescription': backendDescription,
    };
  }
}

class ResultManager {
  static const _jsonResultFileName = 'result.json';
  late final String _jsonResultPath;

  ResultManager(String applicationDirectory) {
    _jsonResultPath = '$applicationDirectory/$_jsonResultFileName';
  }

  // returns brief results in json
  Future<void> writeResults(List<RunResult> results) async {
    final jsonContent = <Map<String, dynamic>>[];

    for (final result in results) {
      var full = _FullResult(
          id: result.id,
          score: result.mode == BenchmarkMode.accuracy
              ? 'N/A'
              : result.score.toString(),
          accuracy: BenchmarkMode.performance_lite == result.mode
              ? 'N/A'
              : result.accuracy,
          minDuration: result.minDuration.toString(),
          duration: result.durationMs.toString(),
          minSamples: result.minSamples.toString(),
          numSamples: result.numSamples.toString(),
          shardsNum: result.threadsNumber,
          batchSize: result.batchSize,
          mode: result.mode.toString(),
          datetime: DateTime.now().toIso8601String(),
          backendDescription: result.backendDescription);
      jsonContent.add(full.toJsonMap());
    }
    await _write(jsonContent);
  }

  String serializeBriefResults(List<Benchmark> benchmarks) {
    final jsonContent = <Map<String, dynamic>>[];

    for (final result in benchmarks) {
      var brief = _BriefResult(
          id: result.id,
          performance: result.performance,
          accuracy: result.accuracy);
      jsonContent.add(brief.toJson());
    }
    return JsonEncoder().convert(jsonContent);
  }

  bool restoreResults(String briefResultsJson, List<Benchmark> benchmarks) {
    if (briefResultsJson == '') return false;

    try {
      for (final resultContent
          in jsonDecode(briefResultsJson) as List<dynamic>) {
        var briefResult =
            _BriefResult.fromJson(resultContent as Map<String, dynamic>);
        final benchmark = benchmarks
            .singleWhere((benchmark) => benchmark.id == briefResult.id);

        benchmark.performance = briefResult.performance;
        benchmark.accuracy = briefResult.accuracy;
      }
    } catch (e, stacktrace) {
      print("can't parse previous results:");
      print(briefResultsJson);
      print(e);
      print(stacktrace);
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
