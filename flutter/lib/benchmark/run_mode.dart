import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;

class BenchmarkRunMode {
  static const _performanceModeString = 'PerformanceOnly';
  static const _accuracyModeString = 'AccuracyOnly';

  static const _perfLogSuffix = 'performance';
  static const _accuracyLogSuffix = 'accuracy';

  final String loadgenMode;
  final String readable;
  final pb.OneDatasetConfig Function(pb.TaskConfig taskConfig) chooseDataset;

  BenchmarkRunMode._({
    required this.loadgenMode,
    required this.readable,
    required this.chooseDataset,
  });

  static BenchmarkRunMode performance = BenchmarkRunMode._(
    loadgenMode: _performanceModeString,
    readable: _perfLogSuffix,
    chooseDataset: (task) => task.datasets.lite,
  );
  static BenchmarkRunMode accuracy = BenchmarkRunMode._(
    loadgenMode: _accuracyModeString,
    readable: _accuracyLogSuffix,
    chooseDataset: (task) => task.datasets.full,
  );

  static BenchmarkRunMode performanceTest = BenchmarkRunMode._(
    loadgenMode: _performanceModeString,
    readable: _perfLogSuffix,
    chooseDataset: (task) => task.datasets.tiny,
  );
  static BenchmarkRunMode accuracyTest = BenchmarkRunMode._(
    loadgenMode: _accuracyModeString,
    readable: _accuracyLogSuffix,
    chooseDataset: (task) => task.datasets.tiny,
  );

  @override
  String toString() {
    return readable;
  }
}

enum BenchmarkRunModeEnum {
  performanceOnly,
  accuracyOnly,
  submissionRun,
}

extension BenchmarkRunModeEnumExtension on BenchmarkRunModeEnum {
  bool get doPerformanceRun {
    switch (this) {
      case BenchmarkRunModeEnum.performanceOnly:
        return true;
      case BenchmarkRunModeEnum.accuracyOnly:
        return false;
      case BenchmarkRunModeEnum.submissionRun:
        return true;
    }
  }

  bool get doAccuracyRun {
    switch (this) {
      case BenchmarkRunModeEnum.performanceOnly:
        return false;
      case BenchmarkRunModeEnum.accuracyOnly:
        return true;
      case BenchmarkRunModeEnum.submissionRun:
        return true;
    }
  }

  String localizedName(AppLocalizations l10n) {
    switch (this) {
      case BenchmarkRunModeEnum.performanceOnly:
        return l10n.benchModePerformanceOnly;
      case BenchmarkRunModeEnum.accuracyOnly:
        return l10n.benchModeAccuracyOnly;
      case BenchmarkRunModeEnum.submissionRun:
        return l10n.benchModeSubmissionRun;
    }
  }
}
