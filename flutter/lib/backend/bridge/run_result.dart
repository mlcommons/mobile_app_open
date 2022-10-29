import 'package:mlperfbench_common/data/results/benchmark_result.dart';

// Contains raw data obtained from the dart_ffi_run_benchmark method,
class NativeRunResult {
  final Accuracy? accuracy1;
  final Accuracy? accuracy2;
  final int numSamples;
  final double duration;
  final String backendName;
  final String backendVendor;
  final String acceleratorName;
  final DateTime startTime;

  NativeRunResult({
    required this.accuracy1,
    required this.accuracy2,
    required this.numSamples,
    required this.duration,
    required this.backendName,
    required this.backendVendor,
    required this.acceleratorName,
    required this.startTime,
  });

  @override
  String toString() => 'RunResult(accuracy:$accuracy1, accuracy2:$accuracy2)';
}
