import 'dart:convert';
import 'dart:io';

import 'package:mlperfbench/backend/run_settings.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/run_result.dart';

class _BriefResult {
  static const _tagId = 'benchmark_id';
  static const _tagPerformance = 'performance';
  static const _tagAccuracy = 'accuracy';

  final String id;
  final BenchmarkResult? performanceModeResult;
  final BenchmarkResult? accuracyModeResult;

  _BriefResult(
      {required this.id,
      required this.performanceModeResult,
      required this.accuracyModeResult});

  Map<String, dynamic> toJson() => {
        _tagId: id,
        _tagPerformance: performanceModeResult,
        _tagAccuracy: accuracyModeResult,
      };

  _BriefResult.fromJson(Map<String, dynamic> jsonMap)
      : id = jsonMap[_tagId] as String,
        performanceModeResult = BenchmarkResult.fromJson(
            jsonMap[_tagPerformance] as Map<String, dynamic>?),
        accuracyModeResult = BenchmarkResult.fromJson(
            jsonMap[_tagAccuracy] as Map<String, dynamic>?);
}

class RunInfo {
  final RunResult result;
  final RunSettings settings;
  final BenchmarkRunMode runMode;

  RunInfo(this.result, this.settings, this.runMode);
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
  final String backendName;

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
      required this.backendName});

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
      // format of results.json should be stable,
      // so we keep 'backendDescription' tag here
      'backendDescription': backendName,
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
  Future<void> writeResults(List<RunInfo> results) async {
    final jsonContent = <Map<String, dynamic>>[];

    for (final info in results) {
      var full = _FullResult(
          id: info.settings.benchmark_id,
          score: info.runMode == BenchmarkRunMode.accuracy
              ? 'N/A'
              : info.result.score.toString(),
          accuracy: info.runMode == BenchmarkRunMode.accuracy
              ? info.result.accuracy
              : 'N/A',
          minDuration: info.settings.min_duration.toString(),
          duration: info.result.durationMs.toString(),
          minSamples: info.settings.min_query_count.toString(),
          numSamples: info.result.numSamples.toString(),
          shardsNum: info.settings.threads_number,
          batchSize: info.settings.batch_size,
          mode: info.runMode.getResultModeString(),
          datetime: DateTime.now().toIso8601String(),
          backendName: info.result.backendName);
      jsonContent.add(full.toJsonMap());
    }
    await _write(jsonContent);
  }

  String serializeBriefResults(List<Benchmark> benchmarks) {
    final jsonContent = <Map<String, dynamic>>[];

    for (final result in benchmarks) {
      var brief = _BriefResult(
          id: result.id,
          performanceModeResult: result.performanceModeResult,
          accuracyModeResult: result.accuracyModeResult);
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

        benchmark.performanceModeResult = briefResult.performanceModeResult;
        benchmark.accuracyModeResult = briefResult.accuracyModeResult;
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
