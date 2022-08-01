import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;

class BenchmarkRunMode {
  static const _performanceModeString = 'PerformanceOnly';
  static const _accuracyModeString = 'AccuracyOnly';

  static const _perfLogSuffix = 'performance';
  static const _accuracyLogSuffix = 'accuracy';

  final String mode;
  final String logSuffix;
  final pb.DatasetConfig Function(pb.TaskConfig taskConfig) chooseDataset;

  BenchmarkRunMode._({
    required this.mode,
    required this.logSuffix,
    required this.chooseDataset,
  });

  static BenchmarkRunMode performance = BenchmarkRunMode._(
    mode: _performanceModeString,
    logSuffix: _perfLogSuffix,
    chooseDataset: (task) => task.liteDataset,
  );
  static BenchmarkRunMode accuracy = BenchmarkRunMode._(
    mode: _accuracyModeString,
    logSuffix: _accuracyLogSuffix,
    chooseDataset: (task) => task.dataset,
  );

  static BenchmarkRunMode performanceTest = BenchmarkRunMode._(
    mode: _performanceModeString,
    logSuffix: _perfLogSuffix,
    chooseDataset: (task) => task.testDataset,
  );
  static BenchmarkRunMode accuracyTest = BenchmarkRunMode._(
    mode: _accuracyModeString,
    logSuffix: _accuracyLogSuffix,
    chooseDataset: (task) => task.testDataset,
  );
}
