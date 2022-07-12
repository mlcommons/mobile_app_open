import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;

enum BenchmarkRunModeEnum { performance, accuracy }

class BenchmarkRunMode {
  final BenchmarkRunModeEnum _mode;

  // Only some of these constants are used in the app
  // Let's keep them for now,
  // they are used by other apps that work with mlperf benchmark
  // they can potentially be used in the future in this app
  static const String backendPerformanceString = 'PerformanceOnly';
  static const String backendAccuracyString = 'AccuracyOnly';
  static const String backendSubmissionString = 'SubmissionRun';

  static const String resultSubmissionString = 'submission_mode';
  static const String resultAccuracyString = 'accuracy_mode';
  static const String resultPerformanceString = 'performance_mode';
  static const String resultPerformanceLiteString = 'performance_lite_mode';
  static const String resultTestString = 'testing';

  BenchmarkRunMode._(this._mode);

  static BenchmarkRunMode performance =
      BenchmarkRunMode._(BenchmarkRunModeEnum.performance);
  static BenchmarkRunMode accuracy =
      BenchmarkRunMode._(BenchmarkRunModeEnum.accuracy);

  String getBackendModeString() {
    switch (_mode) {
      case BenchmarkRunModeEnum.performance:
        return backendPerformanceString;
      case BenchmarkRunModeEnum.accuracy:
        return backendAccuracyString;
      default:
        throw 'unhandled BenchmarkRunModeEnum';
    }
  }

  String getResultModeString() {
    switch (_mode) {
      case BenchmarkRunModeEnum.performance:
        return resultPerformanceLiteString;
      case BenchmarkRunModeEnum.accuracy:
        return resultAccuracyString;
      default:
        throw 'unhandled BenchmarkRunModeEnum';
    }
  }

  pb.DatasetConfig chooseDataset(pb.TaskConfig taskConfig) {
    switch (_mode) {
      case BenchmarkRunModeEnum.performance:
        return taskConfig.liteDataset;
      case BenchmarkRunModeEnum.accuracy:
        return taskConfig.dataset;
      default:
        throw 'unhandled BenchmarkRunModeEnum';
    }
  }
}
