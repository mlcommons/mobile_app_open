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
  final int minSamples;
  final double durationMs;
  final int minDuration;
  final String id;
  final double score;
  final int threadsNumber;
  final int batchSize;
  final String backendDescription;

  RunResult({
    required this.id,
    required this.accuracy,
    required this.numSamples,
    required this.minSamples,
    required this.durationMs,
    required this.minDuration,
    required this.threadsNumber,
    required this.batchSize,
    required this.backendDescription,
    required this.datasetMode,
    required this.backendMode,
    required double latency,
    required String scenario,
  }) : score = 'Offline' == scenario ? latency : 1000 / latency;

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
