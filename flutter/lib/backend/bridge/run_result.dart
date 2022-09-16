import 'package:mlperfbench_common/data/results/benchmark_result.dart';

class RunResult {
  final Accuracy? accuracy1;
  final Accuracy? accuracy2;
  final int numSamples;
  final double durationMs;
  final String backendName;
  final String backendVendor;
  final String acceleratorName;
  final DateTime startTime;

  RunResult({
    required this.accuracy1,
    required this.accuracy2,
    required this.numSamples,
    required this.durationMs,
    required this.backendName,
    required this.backendVendor,
    required this.acceleratorName,
    required this.startTime,
  });

  @override
  String toString() => 'RunResult(accuracy:${accuracy1 ?? 0.0})';
}
