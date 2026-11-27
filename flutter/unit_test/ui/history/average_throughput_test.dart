import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/backend/loadgen_info.dart';
import 'package:mlperfbench/data/generation_helpers/sample_generator.dart';
import 'package:mlperfbench/data/results/benchmark_result.dart';
import 'package:mlperfbench/ui/history/utils.dart';

LoadgenInfo _loadgenInfo({required bool tokenBased}) => LoadgenInfo(
  queryCount: 1,
  latencyMean: 0.1,
  latency90: 0.1,
  latencyFirstTokenMean: 0.1,
  latencyFirstToken90: 0.1,
  tokenThroughput: 1,
  isMinDurationMet: true,
  isMinQueryMet: true,
  isEarlyStoppingMet: true,
  isTokenBased: tokenBased,
  isResultValid: true,
);

// A null throughput mimics a run whose mlperf_log_detail.txt was missing:
// both throughput and loadgen info are then saved as null.
BenchmarkExportResult _result({
  bool hasPerformanceRun = true,
  double? throughput,
  bool tokenBased = false,
}) {
  final sample = SampleGenerator();
  final run = sample.runResult;
  final base = sample.exportResult;
  return BenchmarkExportResult(
    benchmarkId: base.benchmarkId,
    benchmarkName: base.benchmarkName,
    loadgenScenario: base.loadgenScenario,
    backendSettings: base.backendSettings,
    backendInfo: base.backendInfo,
    performanceRun: hasPerformanceRun
        ? BenchmarkRunResult(
            throughput: throughput == null
                ? null
                : Throughput(value: throughput),
            accuracy: run.accuracy,
            accuracy2: run.accuracy2,
            dataset: run.dataset,
            measuredDuration: run.measuredDuration,
            measuredSamples: run.measuredSamples,
            startDatetime: run.startDatetime,
            loadgenInfo: throughput == null
                ? null
                : _loadgenInfo(tokenBased: tokenBased),
          )
        : null,
    accuracyRun: base.accuracyRun,
    minDuration: base.minDuration,
    minSamples: base.minSamples,
  );
}

void main() {
  test('averages only the runs that have a throughput, per kind', () {
    final results = [
      _result(throughput: 100),
      _result(throughput: 300),
      _result(),
      _result(throughput: 40, tokenBased: true),
      _result(hasPerformanceRun: false),
    ];
    expect(calculateAverageThroughput(results), closeTo(200, 1e-9));
    expect(
      calculateAverageThroughput(results, tokenBased: true),
      closeTo(40, 1e-9),
    );
  });

  test('no qualifying run averages to 0', () {
    expect(calculateAverageThroughput([]), 0.0);
    expect(
      calculateAverageThroughput([
        _result(),
        _result(hasPerformanceRun: false),
      ]),
      0.0,
    );
  });
}
