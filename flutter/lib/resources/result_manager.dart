import 'dart:convert';
import 'dart:io';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';

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

class ResultManager {
  static const _jsonResultFileName = 'result.json';
  late final String _jsonResultPath;

  ResultManager(String applicationDirectory) {
    _jsonResultPath = '$applicationDirectory/$_jsonResultFileName';
  }

  // returns brief results in json
  Future<void> writeResults(BenchmarkExportResultList results) async {
    await _write(results.toJson());
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

  Future<void> _write(List<dynamic> content) async {
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
