import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/backend/loadgen_info.dart';
import 'package:mlperfbench/benchmark/performance_result_validity.dart';
import 'package:mlperfbench/data/generation_helpers/sample_generator.dart';
import 'package:mlperfbench/data/results/benchmark_result.dart';
import 'package:mlperfbench/ui/app_styles.dart';

LoadgenInfo _loadgenInfo({
  required bool minDurationMet,
  required bool minQueryMet,
  required bool earlyStoppingMet,
}) => LoadgenInfo(
  queryCount: 1,
  latencyMean: 0.1,
  latency90: 0.1,
  latencyFirstTokenMean: 0.1,
  latencyFirstToken90: 0.1,
  tokenThroughput: 1,
  isMinDurationMet: minDurationMet,
  isMinQueryMet: minQueryMet,
  isEarlyStoppingMet: earlyStoppingMet,
  isTokenBased: false,
  isResultValid: true,
);

BenchmarkExportResult _exportResult({
  required String benchmarkId,
  required LoadgenInfo? loadgenInfo,
  bool hasPerformanceRun = true,
}) {
  final sample = SampleGenerator();
  final run = sample.runResult;
  final base = sample.exportResult;
  return BenchmarkExportResult(
    benchmarkId: benchmarkId,
    benchmarkName: base.benchmarkName,
    loadgenScenario: base.loadgenScenario,
    backendSettings: base.backendSettings,
    backendInfo: base.backendInfo,
    performanceRun: hasPerformanceRun
        ? BenchmarkRunResult(
            throughput: run.throughput,
            accuracy: run.accuracy,
            accuracy2: run.accuracy2,
            dataset: run.dataset,
            measuredDuration: run.measuredDuration,
            measuredSamples: run.measuredSamples,
            startDatetime: run.startDatetime,
            loadgenInfo: loadgenInfo,
          )
        : null,
    accuracyRun: base.accuracyRun,
    minDuration: base.minDuration,
    minSamples: base.minSamples,
  );
}

PerformanceResultValidityEnum _validity({
  required String benchmarkId,
  required bool minDurationMet,
  required bool minQueryMet,
  required bool earlyStoppingMet,
}) {
  return PerformanceResultValidityEnum.forBenchmarkExportResult(
    benchmarkExportResult: _exportResult(
      benchmarkId: benchmarkId,
      loadgenInfo: _loadgenInfo(
        minDurationMet: minDurationMet,
        minQueryMet: minQueryMet,
        earlyStoppingMet: earlyStoppingMet,
      ),
    ),
  );
}

void main() {
  const valid = PerformanceResultValidityEnum.valid;
  const semivalid = PerformanceResultValidityEnum.semivalid;
  const invalid = PerformanceResultValidityEnum.invalid;

  group('PerformanceResultValidityEnum.forBenchmarkExportResult', () {
    test('missing result is invalid', () {
      expect(
        PerformanceResultValidityEnum.forBenchmarkExportResult(
          benchmarkExportResult: null,
        ),
        invalid,
      );
    });
    test('missing performance run is invalid', () {
      final result = _exportResult(
        benchmarkId: BenchmarkId.imageClassificationV2,
        loadgenInfo: null,
        hasPerformanceRun: false,
      );
      expect(
        PerformanceResultValidityEnum.forBenchmarkExportResult(
          benchmarkExportResult: result,
        ),
        invalid,
      );
    });
    test('missing loadgen info is invalid', () {
      final result = _exportResult(
        benchmarkId: BenchmarkId.imageClassificationV2,
        loadgenInfo: null,
      );
      expect(
        PerformanceResultValidityEnum.forBenchmarkExportResult(
          benchmarkExportResult: result,
        ),
        invalid,
      );
    });

    // (minDurationMet, minQueryMet, earlyStoppingMet) -> expected validity
    final regularCases = <(bool, bool, bool, PerformanceResultValidityEnum)>[
      (true, true, true, valid),
      (true, false, true, semivalid),
      (true, true, false, invalid),
      (true, false, false, invalid),
      (false, true, true, invalid),
      (false, false, true, invalid),
      (false, true, false, invalid),
      (false, false, false, invalid),
    ];
    for (final (duration, query, early, expected) in regularCases) {
      test(
        'duration=$duration query=$query early=$early is ${expected.name}',
        () {
          expect(
            _validity(
              benchmarkId: BenchmarkId.imageClassificationV2,
              minDurationMet: duration,
              minQueryMet: query,
              earlyStoppingMet: early,
            ),
            expected,
          );
        },
      );
    }

    group('stable diffusion ignores the early stopping condition', () {
      final cases = <(bool, bool, bool, PerformanceResultValidityEnum)>[
        (true, true, false, valid),
        (true, false, false, semivalid),
        (false, true, false, invalid),
        (false, false, false, invalid),
      ];
      for (final (duration, query, early, expected) in cases) {
        test(
          'duration=$duration query=$query early=$early is ${expected.name}',
          () {
            expect(
              _validity(
                benchmarkId: BenchmarkId.stableDiffusion,
                minDurationMet: duration,
                minQueryMet: query,
                earlyStoppingMet: early,
              ),
              expected,
            );
          },
        );
      }
    });
  });

  group('PerformanceResultValidityExtension.color', () {
    test('maps each validity to its app color', () {
      expect(valid.color, AppColors.resultValidText);
      expect(semivalid.color, AppColors.resultSemiValidText);
      expect(invalid.color, AppColors.resultInvalidText);
    });
    test('colors are distinct', () {
      final colors = PerformanceResultValidityEnum.values.map((e) => e.color);
      expect(
        colors.toSet().length,
        PerformanceResultValidityEnum.values.length,
      );
    });
  });
}
