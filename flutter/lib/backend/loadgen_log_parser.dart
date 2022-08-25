import 'dart:convert';
import 'dart:io';

class LoadgenLogParser {
  static Future<Map<String, dynamic>> parseData({
    required String filepath,
    required Set<String> requiredKeys,
  }) async {
    Map<String, dynamic> result = {};
    await File(filepath)
        .openRead()
        .map(utf8.decode)
        .transform(const LineSplitter())
        .forEach((line) {
      const prefix = ':::MLLOG ';
      if (!line.startsWith(prefix)) {
        return;
      }
      line = line.substring(prefix.length);
      final json = jsonDecode(line);
      final lineKey = json['key'] as String;
      if (requiredKeys.contains(lineKey)) {
        result[lineKey] = json['value'];
      }
    });

    return result;
  }

  static Future<LoadgenInfo?> extractLoadgenInfo({
    required String logFile,
  }) async {
    print('reading logs from $logFile');
    try {
      const latencyKey = 'result_mean_latency_ns';
      const queryCountKey = 'result_query_count';
      const latency90Key = 'result_90.00_percentile_latency_ns';
      const validityKey = 'result_validity';

      final result = await LoadgenLogParser.parseData(
        filepath: logFile,
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
    } catch (e, t) {
      print(e);
      print(t);
      return null;
    }
  }
}

class LoadgenInfo {
  final double meanLatency;
  final int queryCount;
  final double latency90;
  final bool validity;

  LoadgenInfo({
    required this.meanLatency,
    required this.queryCount,
    required this.latency90,
    required this.validity,
  });
}
