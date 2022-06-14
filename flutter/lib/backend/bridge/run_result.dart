class RunResult {
  final double accuracyNormalized;
  final String accuracyFormatted;
  final int numSamples;
  final double durationMs;
  final double throughput;
  final String backendName;
  final String backendVendor;
  final String acceleratorName;
  final DateTime startTime;
  late final bool validity;

  RunResult({
    required this.accuracyNormalized,
    required this.accuracyFormatted,
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
      'RunResult(throughput:$throughput, accuracy:$accuracyFormatted)';
}
