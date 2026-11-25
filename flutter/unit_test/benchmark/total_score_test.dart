// The analyzer does not treat this non-standard test dir as a test context.
// ignore_for_file: invalid_use_of_visible_for_testing_member

import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/backend/list.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/data/results/benchmark_result.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as task_pb;

Benchmark _benchmark(String id, double maxThroughput, {double? throughput}) {
  final settings = pb.BenchmarkSetting(benchmarkId: id);
  final benchmark = Benchmark(
    backends: [
      BenchmarkBackend(
        info: BackendInfo.forTest(
          pb.BackendSetting(benchmarkSetting: [settings]),
          'libtflitebackend',
        ),
        settings: settings,
      ),
    ],
    taskConfig: task_pb.TaskConfig(id: id, maxThroughput: maxThroughput),
    isActive: true,
  );
  if (throughput != null) {
    benchmark.performanceModeResult = BenchmarkResult(
      throughput: Throughput(value: throughput),
      accuracy: null,
      accuracy2: null,
      backendName: '',
      acceleratorName: '',
      delegateName: '',
      batchSize: 1,
      loadgenInfo: null,
    );
  }
  return benchmark;
}

// Mirrors the shipped tasks.pbtxt: one task at 1000, twelve at 2000.
List<Benchmark> _store({required Set<int> ran, double fraction = 0.5}) {
  return List.generate(13, (i) {
    final max = i == 0 ? 1000.0 : 2000.0;
    return _benchmark(
      'task$i',
      max,
      throughput: ran.contains(i) ? max * fraction : null,
    );
  });
}

void main() {
  test('no benchmark ran scores 0', () {
    expect(totalScore(_store(ran: {})), 0);
  });

  test('full run at 50% of max scores 0.5', () {
    final all = Set<int>.from(List.generate(13, (i) => i));
    expect(totalScore(_store(ran: all)), closeTo(0.5, 1e-9));
  });

  // https://github.com/mlcommons/mobile_app_open/issues/1182
  test('partial run is scored only over the benchmarks that ran', () {
    expect(totalScore(_store(ran: {11, 12})), closeTo(0.5, 1e-9));
    expect(totalScore(_store(ran: {0})), closeTo(0.5, 1e-9));
    expect(
      totalScore(_store(ran: {0, 5, 7}, fraction: 0.25)),
      closeTo(0.25, 1e-9),
    );
  });

  test('a run without a throughput is left out of both halves', () {
    final benchmarks = _store(ran: {1, 2});
    benchmarks[3].performanceModeResult = BenchmarkResult(
      throughput: null,
      accuracy: null,
      accuracy2: null,
      backendName: '',
      acceleratorName: '',
      delegateName: '',
      batchSize: 1,
      loadgenInfo: null,
    );
    expect(totalScore(benchmarks), closeTo(0.5, 1e-9));
  });

  test('geometric mean of per-benchmark ratios', () {
    final benchmarks = [
      _benchmark('a', 1000, throughput: 800), // 0.8
      _benchmark('b', 2000, throughput: 400), // 0.2
      _benchmark('c', 2000),
    ];
    expect(totalScore(benchmarks), closeTo(0.4, 1e-9));
  });
}
