import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;

const _performanceModeString = 'PerformanceOnly';
const _accuracyModeString = 'AccuracyOnly';

const _perfLogSuffix = 'performance';
const _accuracyLogSuffix = 'accuracy';

class BenchmarkRunMode {
  final LoadgenModeEnum loadgenMode;
  final String readable;
  late final pb.OneDatasetConfig Function(pb.TaskConfig t) chooseDataset;
  late final pb.OneRunConfig Function(pb.TaskConfig t) chooseRunConfig;

  // final int coolDownDuration;

  BenchmarkRunMode._({
    required this.loadgenMode,
    required this.readable,
  });

  @override
  String toString() {
    return readable;
  }
}

enum LoadgenModeEnum {
  performanceOnly,
  accuracyOnly,
}

extension LoadgenModeEnumExtension on LoadgenModeEnum {
  String get name {
    switch (this) {
      case LoadgenModeEnum.performanceOnly:
        return _performanceModeString;
      case LoadgenModeEnum.accuracyOnly:
        return _accuracyModeString;
    }
  }
}

enum BenchmarkRunModeEnum {
  performanceOnly,
  accuracyOnly,
  submissionRun,
  quickRun,
  integrationTestRun
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
      case BenchmarkRunModeEnum.quickRun:
        return true;
      case BenchmarkRunModeEnum.integrationTestRun:
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
      case BenchmarkRunModeEnum.quickRun:
        return false;
      case BenchmarkRunModeEnum.integrationTestRun:
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
      case BenchmarkRunModeEnum.quickRun:
        return l10n.benchModeQuickRun;
      case BenchmarkRunModeEnum.integrationTestRun:
        return l10n.benchModeIntegrationTestRun;
    }
  }

  bool get isHiddenFromUI {
    switch (this) {
      case BenchmarkRunModeEnum.integrationTestRun:
        return true;
      default:
        return false;
    }
  }

  BenchmarkRunMode get performanceRunMode {
    BenchmarkRunMode mode = BenchmarkRunMode._(
      loadgenMode: LoadgenModeEnum.performanceOnly,
      readable: _perfLogSuffix,
    );
    switch (this) {
      case BenchmarkRunModeEnum.performanceOnly:
        mode.chooseDataset = (pb.TaskConfig t) => t.datasets.lite;
        mode.chooseRunConfig = (pb.TaskConfig t) => t.runs.normal;
        break;
      case BenchmarkRunModeEnum.accuracyOnly:
        mode.chooseDataset = (pb.TaskConfig t) => t.datasets.lite;
        mode.chooseRunConfig = (pb.TaskConfig t) => t.runs.normal;
        break;
      case BenchmarkRunModeEnum.submissionRun:
        mode.chooseDataset = (pb.TaskConfig t) => t.datasets.lite;
        mode.chooseRunConfig = (pb.TaskConfig t) => t.runs.normal;
        break;
      case BenchmarkRunModeEnum.quickRun:
        mode.chooseDataset = (pb.TaskConfig t) => t.datasets.lite;
        mode.chooseRunConfig = (pb.TaskConfig t) => t.runs.quick;
        break;
      case BenchmarkRunModeEnum.integrationTestRun:
        mode.chooseDataset = (pb.TaskConfig t) => t.datasets.tiny;
        mode.chooseRunConfig = (pb.TaskConfig t) => t.runs.rapid;
        break;
    }
    return mode;
  }

  BenchmarkRunMode get accuracyRunMode {
    BenchmarkRunMode mode = BenchmarkRunMode._(
      loadgenMode: LoadgenModeEnum.accuracyOnly,
      readable: _accuracyLogSuffix,
    );
    switch (this) {
      case BenchmarkRunModeEnum.performanceOnly:
        mode.chooseDataset = (pb.TaskConfig t) => t.datasets.full;
        mode.chooseRunConfig = (pb.TaskConfig t) => t.runs.normal;
        break;
      case BenchmarkRunModeEnum.accuracyOnly:
        mode.chooseDataset = (pb.TaskConfig t) => t.datasets.full;
        mode.chooseRunConfig = (pb.TaskConfig t) => t.runs.normal;
        break;
      case BenchmarkRunModeEnum.submissionRun:
        mode.chooseDataset = (pb.TaskConfig t) => t.datasets.full;
        mode.chooseRunConfig = (pb.TaskConfig t) => t.runs.normal;
        break;
      case BenchmarkRunModeEnum.quickRun:
        mode.chooseDataset = (pb.TaskConfig t) => t.datasets.full;
        mode.chooseRunConfig = (pb.TaskConfig t) => t.runs.quick;
        break;
      case BenchmarkRunModeEnum.integrationTestRun:
        mode.chooseDataset = (pb.TaskConfig t) => t.datasets.tiny;
        mode.chooseRunConfig = (pb.TaskConfig t) => t.runs.rapid;
        break;
    }
    return mode;
  }

  List<BenchmarkRunMode> get selectedRunModes {
    final modes = <BenchmarkRunMode>[];
    switch (this) {
      case BenchmarkRunModeEnum.performanceOnly:
        modes.add(performanceRunMode);
        break;
      case BenchmarkRunModeEnum.accuracyOnly:
        modes.add(accuracyRunMode);
        break;
      case BenchmarkRunModeEnum.submissionRun:
        modes.add(performanceRunMode);
        modes.add(accuracyRunMode);
        break;
      case BenchmarkRunModeEnum.quickRun:
        modes.add(performanceRunMode);
        break;
      case BenchmarkRunModeEnum.integrationTestRun:
        modes.add(performanceRunMode);
        modes.add(accuracyRunMode);
        break;
    }
    return modes;
  }

  int get cooldownDuration {
    switch (this) {
      case BenchmarkRunModeEnum.performanceOnly:
        return 5 * 60;
      case BenchmarkRunModeEnum.accuracyOnly:
        return 2;
      case BenchmarkRunModeEnum.submissionRun:
        return 5 * 60;
      case BenchmarkRunModeEnum.quickRun:
        return 1 * 60;
      case BenchmarkRunModeEnum.integrationTestRun:
        return 5;
    }
  }
}
