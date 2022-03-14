class RunResult {
  final String accuracy;
  final int numSamples;
  final double durationMs;
  final double throughput;
  final String backendName;
  final String acceleratorName;

  RunResult({
    required this.accuracy,
    required this.numSamples,
    required this.durationMs,
    required this.backendName,
    required this.acceleratorName,
    required this.throughput,
  });

  @override
  String toString() => 'RunResult(throughput:$throughput, accuracy:$accuracy)';
}
