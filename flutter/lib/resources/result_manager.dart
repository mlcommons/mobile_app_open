import 'dart:convert';
import 'dart:io';

import 'package:mlperfbench_common/data/extended_result.dart';

import 'utils.dart';

class ResultManager {
  static const _resultsFileName = 'results.json';

  final File jsonFile;
  final List<ExtendedResult> results;

  ResultManager({
    required this.results,
    required this.jsonFile,
  });

  static Future<ResultManager> create({required String resultDir}) async {
    final jsonFile = File('$resultDir/$_resultsFileName');

    List<dynamic> json = [];
    if (await jsonFile.exists()) {
      try {
        json = jsonDecode(await jsonFile.readAsString()) as List<dynamic>;
      } catch (e, trace) {
        print('unable to read saved results: $e');
        print(trace);
      }
    }
    final results = parseResultJson(json);

    return ResultManager(
      jsonFile: jsonFile,
      results: results,
    );
  }

  static List<ExtendedResult> parseResultJson(List<dynamic> json) {
    final list = <ExtendedResult>[];
    for (var item in json) {
      try {
        list.add(ExtendedResult.fromJson(item as Map<String, dynamic>));
      } catch (e, trace) {
        print('unable to parse result from history: $e');
        print(trace);
      }
    }
    return list;
  }

  Future<void> _saveResults() async {
    await jsonFile.writeAsString(jsonToStringIndented(results));
  }

  Future<void> deleteResults() async {
    if (await jsonFile.exists()) {
      await jsonFile.delete();
    }
  }

  Future<void> removeSelected(List<bool> selected) async {
    if (selected.length != results.length) {
      throw 'selected.length != results.length';
    }
    var newResults = <ExtendedResult>[];
    for (int i = 0; i < results.length; i++) {
      if (!selected[i]) {
        newResults.add(results[i]);
      }
    }
    results.clear();
    results.addAll(newResults);
    await _saveResults();
  }

  Future<void> addResult(ExtendedResult value) async {
    results.add(value);
    await _saveResults();
  }

  ExtendedResult getLastResult() {
    return results.last;
  }
}
