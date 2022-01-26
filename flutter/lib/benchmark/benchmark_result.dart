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
  final DatasetMode _datasetMode;
  final String _backendMode;

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

  RunResult(
    this.id,
    this.accuracy,
    this.numSamples,
    this.minSamples,
    this.durationMs,
    this.minDuration,
    this.threadsNumber,
    this.batchSize,
    this.backendDescription,
    this._datasetMode,
    this._backendMode,
    double latency,
    String scenario,
  ) : score = 'Offline' == scenario ? latency : 1000 / latency;

  String get mode {
    if (_backendMode == BenchmarkMode.backendSubmission) {
      if (_datasetMode == DatasetMode.test) {
        return BenchmarkMode.test;
      } else if (_datasetMode == DatasetMode.full) {
        return BenchmarkMode.submission;
      }
    }

    if (_backendMode == BenchmarkMode.backendAccuracy &&
        _datasetMode == DatasetMode.full) return BenchmarkMode.accuracy;

    if (_backendMode == BenchmarkMode.backendPerfomance) {
      if (_datasetMode == DatasetMode.full) {
        return BenchmarkMode.performance;
      } else if (_datasetMode == DatasetMode.lite) {
        return BenchmarkMode.performance_lite;
      }
    }

    return '';
  }

  @override
  String toString() => 'RunResult(score:$score, accuracy:$accuracy)';
}
