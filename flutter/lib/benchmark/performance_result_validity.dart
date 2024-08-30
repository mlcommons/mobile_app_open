import 'package:flutter/material.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/data/results/benchmark_result.dart';
import 'package:mlperfbench/ui/app_styles.dart';

enum PerformanceResultValidityEnum {
  valid,
  invalid,
  semivalid;

  static PerformanceResultValidityEnum forBenchmarkExportResult(
      {required BenchmarkExportResult? benchmarkExportResult}) {
    if (benchmarkExportResult == null) {
      return PerformanceResultValidityEnum.invalid;
    }
    final loadgenInfo = benchmarkExportResult.performanceRun?.loadgenInfo;
    if (loadgenInfo == null) {
      return PerformanceResultValidityEnum.invalid;
    }
    final benchmarkId = benchmarkExportResult.benchmarkId;
    final isMinDurationMet = loadgenInfo.isMinDurationMet;
    final isMinQueryMet = loadgenInfo.isMinQueryMet;
    final isEarlyStoppingMet = loadgenInfo.isEarlyStoppingMet;
    return _performanceResultValidity(
      benchmarkId: benchmarkId,
      isMinDurationMet: isMinDurationMet,
      isMinQueryMet: isMinQueryMet,
      isEarlyStoppingMet: isEarlyStoppingMet,
    );
  }

  static PerformanceResultValidityEnum forBenchmark(Benchmark benchmark) {
    final loadgenInfo = benchmark.performanceModeResult?.loadgenInfo;
    if (loadgenInfo == null) {
      return PerformanceResultValidityEnum.invalid;
    }
    final benchmarkId = benchmark.id;
    final isMinDurationMet = loadgenInfo.isMinDurationMet;
    final isMinQueryMet = loadgenInfo.isMinQueryMet;
    final isEarlyStoppingMet = loadgenInfo.isEarlyStoppingMet;
    return _performanceResultValidity(
      benchmarkId: benchmarkId,
      isMinDurationMet: isMinDurationMet,
      isMinQueryMet: isMinQueryMet,
      isEarlyStoppingMet: isEarlyStoppingMet,
    );
  }

  static PerformanceResultValidityEnum _performanceResultValidity({
    required String benchmarkId,
    required bool isMinDurationMet,
    required bool isMinQueryMet,
    required bool isEarlyStoppingMet,
  }) {
    // For stable_diffusion task, we ignore the early-stopping condition.
    if (benchmarkId == BenchmarkId.stableDiffusion) {
      isEarlyStoppingMet = true;
    }
    if (isMinDurationMet == true &&
        isMinQueryMet == true &&
        isEarlyStoppingMet == true) {
      return PerformanceResultValidityEnum.valid;
    } else if (isMinDurationMet == true &&
        isMinQueryMet == false &&
        isEarlyStoppingMet == true) {
      return PerformanceResultValidityEnum.semivalid;
    } else {
      return PerformanceResultValidityEnum.invalid;
    }
  }
}

extension PerformanceResultValidityExtension on PerformanceResultValidityEnum {
  Color get color {
    switch (this) {
      case PerformanceResultValidityEnum.valid:
        return AppColors.resultValidText;
      case PerformanceResultValidityEnum.invalid:
        return AppColors.resultInvalidText;
      case PerformanceResultValidityEnum.semivalid:
        return AppColors.resultSemiValidText;
    }
  }
}
