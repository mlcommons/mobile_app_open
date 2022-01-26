import 'benchmark.dart';

class BenchmarkMode {
  static final String backendPerfomance = 'PerformanceOnly';
  static final String backendAccuracy = 'AccuracyOnly';
  static final String backendSubmission = 'SubmissionRun';

  static final String submission = 'submission_mode';
  static final String accuracy = 'accuracy_mode';
  static final String performance = 'performance_mode';
  static final String performance_lite = 'performance_lite_mode';
  static final String test = 'testing';
}

class RunDebugResult {
  final String briefResult;
  final String output;

  RunDebugResult(this.briefResult, this.output);
}

class RunResult {
  final DatasetMode datasetMode;
  final String backendMode;

  final String accuracy;
  final int numSamples;
  final double durationMs;
  final double score;
  final String backendDescription;

  RunResult({
    required this.accuracy,
    required this.numSamples,
    required this.durationMs,
    required this.backendDescription,
    required this.datasetMode,
    required this.backendMode,
    required this.score,
  });

  String get mode {
    if (backendMode == BenchmarkMode.backendSubmission) {
      if (datasetMode == DatasetMode.test) {
        return BenchmarkMode.test;
      } else if (datasetMode == DatasetMode.full) {
        return BenchmarkMode.submission;
      }
    }

    if (backendMode == BenchmarkMode.backendAccuracy &&
        datasetMode == DatasetMode.full) return BenchmarkMode.accuracy;

    if (backendMode == BenchmarkMode.backendPerfomance) {
      if (datasetMode == DatasetMode.full) {
        return BenchmarkMode.performance;
      } else if (datasetMode == DatasetMode.lite) {
        return BenchmarkMode.performance_lite;
      }
    }

    return '';
  }

  @override
  String toString() => 'RunResult(score:$score, accuracy:$accuracy)';
}
