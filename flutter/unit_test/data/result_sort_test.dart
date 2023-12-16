import 'dart:convert';
import 'dart:io';

import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/data/extended_result.dart';
import 'package:mlperfbench/data/result_sort.dart';

void main() {
  group('ResultSort', () {
    const file = 'unit_test/data/extended_result_unittest.json';
    final jsonString = File(file).readAsStringSync();
    final data = jsonDecode(jsonString) as Map<String, dynamic>;
    final result = ExtendedResult.fromJson(data);
    final benchmarks = result.results;

    test('number of benchmarks', () {
      expect(benchmarks.length, 6);
    });

    final earliestBenchmark = benchmarks.first;
    final latestBenchmark = benchmarks.last;

    final minTaskThroughput = benchmarks
        .map((benchmark) => benchmark.performanceRun?.throughput?.value ?? 0)
        .reduce((a, b) => a < b ? a : b);
    final maxTaskThroughput = benchmarks
        .map((benchmark) => benchmark.performanceRun?.throughput?.value ?? 0)
        .reduce((a, b) => a > b ? a : b);

    test('sort by Date ASC (from oldest to most recent)', () {
      final sort = ResultSort();
      sort.sortBy = SortByEnum.dateAsc;
      expect(sort.apply(benchmarks).first, earliestBenchmark);
      expect(sort.apply(benchmarks).last, latestBenchmark);
    });

    test('sort by Date DESC (from most recent to oldest)', () {
      final sort = ResultSort();
      sort.sortBy = SortByEnum.dateDesc;
      expect(sort.apply(benchmarks).first, latestBenchmark);
      expect(sort.apply(benchmarks).last, earliestBenchmark);
    });

    test('sort by Task Throughput ASC (from lowest to highest)', () {
      final sort = ResultSort();
      sort.sortBy = SortByEnum.taskThroughputAsc;
      expect(sort.apply(benchmarks).first.performanceRun?.throughput?.value,
          minTaskThroughput);
      expect(sort.apply(benchmarks).last.performanceRun?.throughput?.value,
          maxTaskThroughput);
    });

    test('sort by Task Throughput DESC (from highest to lowest)', () {
      final sort = ResultSort();
      sort.sortBy = SortByEnum.taskThroughputDesc;
      expect(sort.apply(benchmarks).first.performanceRun?.throughput?.value,
          maxTaskThroughput);
      expect(sort.apply(benchmarks).last.performanceRun?.throughput?.value,
          minTaskThroughput);
    });
  });
}
