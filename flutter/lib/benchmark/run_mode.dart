import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;

class BenchmarkRunMode {
  static const _performanceModeString = 'PerformanceOnly';
  static const _accuracyModeString = 'AccuracyOnly';

  final String mode;
  final pb.DatasetConfig Function(pb.TaskConfig taskConfig) chooseDataset;

  BenchmarkRunMode._({
    required this.mode,
    required this.chooseDataset,
  });

  static BenchmarkRunMode performance = BenchmarkRunMode._(
    mode: _performanceModeString,
    chooseDataset: (task) => task.liteDataset,
  );
  static BenchmarkRunMode accuracy = BenchmarkRunMode._(
    mode: _accuracyModeString,
    chooseDataset: (task) => task.dataset,
  );
}
