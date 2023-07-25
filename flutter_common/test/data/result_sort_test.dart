import 'dart:convert';
import 'dart:io';

import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/result_sort.dart';
import 'package:mlperfbench_common/data/sort_by_item.dart';

void main() {
  group('ResultSort', () {
    const file = 'test/data/extended_result_unittest.json';
    final jsonString = File(file).readAsStringSync();
    final data = jsonDecode(jsonString) as Map<String, dynamic>;
    final result = ExtendedResult.fromJson(data);
    final benchmarks = result.results;

    test('sort by Date ASC (from oldest to most recent)', () {
      final sort = ResultSort();
      sort.sortBy = SortByValues.dateAsc;
      expect(sort.apply(benchmarks)[0] == benchmarks[0], isTrue);
    });

    test('sort by Date DESC (from most recent to oldest)', () {
      final sort = ResultSort();
      sort.sortBy = SortByValues.dateDesc;
      expect(sort.apply(benchmarks)[0] == benchmarks[benchmarks.length - 1],
          isTrue);
    });

    test('sort by Task Throughtput DESC (from highest to lowest)', () {
      final sort = ResultSort();
      sort.sortBy = SortByValues.taskThroughputDesc;
      final maxTaskThroughput = benchmarks
          .map((benchmark) => benchmark.performanceRun?.throughput?.value ?? 0)
          .reduce((a, b) => a > b ? a : b);
      expect(
          sort.apply(benchmarks)[0].performanceRun?.throughput?.value ==
              maxTaskThroughput,
          isTrue);
    });
  });
}
