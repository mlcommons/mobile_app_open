import 'dart:convert';
import 'dart:io';

class LoadgenInfo {
  // Mean latency in seconds
  final double meanLatency;
  final int queryCount;
  // 90th percentile in seconds
  final double latency90;
  final bool validity;

  LoadgenInfo({
    required this.meanLatency,
    required this.queryCount,
    required this.latency90,
    required this.validity,
  });

  static Future<LoadgenInfo?> fromFile({required String filepath}) {
    final lines = File(filepath)
        .openRead()
        .map(utf8.decode)
        .transform(const LineSplitter());
    return extractLoadgenInfo(logLines: lines);
  }

  static Future<Map<String, dynamic>> extractKeys({
    required Stream<String> logLines,
    required Set<String> requiredKeys,
  }) async {
    Map<String, dynamic> result = {};

    await for (var line in logLines) {
      const prefix = ':::MLLOG ';
      if (!line.startsWith(prefix)) {
        throw 'missing $prefix prefix';
      }
      line = line.substring(prefix.length);
      final json = jsonDecode(line);
      final lineKey = json['key'] as String;
      if (requiredKeys.contains(lineKey)) {
        result[lineKey] = json['value'];
      }
    }

    return result;
  }

  static Future<LoadgenInfo?> extractLoadgenInfo({
    required Stream<String> logLines,
  }) async {
    const latencyKey = 'result_mean_latency_ns';
    const queryCountKey = 'result_query_count';
    const latency90Key = 'result_90.00_percentile_latency_ns';
    const validityKey = 'result_validity';

    final result = await extractKeys(
      logLines: logLines,
      requiredKeys: {
        latencyKey,
        queryCountKey,
        latency90Key,
        validityKey,
      },
    );

    if (result.isEmpty) {
      return null;
    }

    final validity = result[validityKey] as String == 'VALID';

    const nanosecondsPerSecond = 1000 * Duration.microsecondsPerSecond;

    return LoadgenInfo(
      meanLatency: (result[latencyKey] as int) / nanosecondsPerSecond,
      queryCount: result[queryCountKey] as int,
      latency90: (result[latency90Key] as int) / nanosecondsPerSecond,
      validity: validity,
    );
  }
}
