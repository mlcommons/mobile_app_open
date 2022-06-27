class RunResult {
  final double throughput;
  final double accuracyNormalized;
  final String accuracyFormatted;
  final double accuracyNormalized2;
  final String accuracyFormatted2;
  final int numSamples;
  final double durationMs;
  final String backendName;
  final String backendVendor;
  final String acceleratorName;
  final DateTime startTime;
  late final bool validity;

  RunResult({
    required this.accuracyNormalized,
    required this.accuracyFormatted,
    required this.accuracyNormalized2,
    required this.accuracyFormatted2,
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
