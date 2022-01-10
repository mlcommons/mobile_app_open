import 'dart:convert';
import 'dart:io';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/benchmark_result.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;

class _BriefResult {
  static const _idTag = 'benchmark_id';
  static const _scoreTag = 'score';
  static const _accuracyTag = 'accuracy';
  static const _shardsNumTag = 'shards_num';
  static const _batchSizeTag = 'batch_size';

  final String id;
  final double? score;
  final String? accuracy;
  final int shardsNum;
  final int batchSize;

  _BriefResult(
      {required this.id,
      required this.score,
      required this.accuracy,
      required this.shardsNum,
      required this.batchSize});

  Map<String, dynamic> toJsonMap() {
    return {
      _idTag: id,
      _scoreTag: score,
      _accuracyTag: accuracy,
      _shardsNumTag: shardsNum,
      _batchSizeTag: batchSize,
    };
  }

  static _BriefResult fromJsonMap(Map<String, dynamic> jsonMap) {
    final id = jsonMap[_idTag] as String;
    final score = jsonMap[_scoreTag] as double?;
    final accuracy = jsonMap[_accuracyTag] as String?;
    final shardsNum = jsonMap[_shardsNumTag] as int? ?? 0;
    final batchSize = jsonMap[_batchSizeTag] as int? ?? 0;

    return _BriefResult(
        id: id,
        score: score,
        accuracy: accuracy,
        shardsNum: shardsNum,
        batchSize: batchSize);
  }
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
      required this.datetime});

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
  Future<String> record(List<RunResult?> results) async {
    final resultContent = <Map<String, dynamic>>[];
    final briefResultContent = <Map<String, dynamic>>[];

    for (final result in results) {
      if (result == null) continue;

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
          datetime: DateTime.now().toIso8601String());
      resultContent.add(full.toJsonMap());

      var brief = _BriefResult(
          id: result.id,
          score: result.mode == BenchmarkMode.accuracy ? null : result.score,
          accuracy: result.accuracy,
          shardsNum: result.threadsNumber,
          batchSize: result.batchSize);
      briefResultContent.add(brief.toJsonMap());
    }
    await _write(resultContent);
    return JsonEncoder().convert(briefResultContent);
  }

  bool restoreResults(String briefResultsJson, List<Benchmark> benchmarks) {
    if (briefResultsJson == '') return false;

    try {
      for (final resultContent in jsonDecode(briefResultsJson)) {
        var briefResult =
            _BriefResult.fromJsonMap(resultContent as Map<String, dynamic>);
        final benchmark = benchmarks
            .singleWhere((benchmark) => benchmark.id == briefResult.id);
        benchmark.accuracy = briefResult.accuracy;
        benchmark.score = briefResult.score;

        if (benchmark.modelConfig.scenario == 'Offline') {
          benchmark.benchmarkSetting.customSetting.add(pb.CustomSetting(
              id: 'batch_size', value: briefResult.batchSize.toString()));
          benchmark.benchmarkSetting.customSetting.add(pb.CustomSetting(
              id: 'shards_num', value: briefResult.shardsNum.toString()));
          benchmark.benchmarkSetting.writeToBuffer();
        }
      }
    } catch (e) {
      print("can't parse previous results: $e");
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
