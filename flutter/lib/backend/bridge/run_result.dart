class RunResult {
  final String accuracy;
  final int numSamples;
  final double durationMs;
  final double score;
  final String backendName;

  RunResult({
    required this.accuracy,
    required this.numSamples,
    required this.durationMs,
    required this.backendName,
    required this.score,
  });

  @override
  String toString() => 'RunResult(score:$score, accuracy:$accuracy)';
}
