class RunResult {
  final double accuracyNum;
  final String accuracyString;
  final int numSamples;
  final double durationMs;
  final double throughput;
  final String backendName;
  final String backendVendor;
  final String acceleratorName;
  final DateTime startTime;
  late final bool validity;

  RunResult({
    required this.accuracyNum,
    required this.accuracyString,
    required this.numSamples,
    required this.durationMs,
    required this.backendName,
    required this.backendVendor,
    required this.acceleratorName,
    required this.throughput,
    required this.startTime,
  });

  @override
  String toString() =>
      'RunResult(throughput:$throughput, accuracy:$accuracyString)';
}
