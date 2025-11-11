import 'dart:convert';
import 'dart:io';

import 'package:json_annotation/json_annotation.dart';

part 'loadgen_info.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class LoadgenInfo {
  final int queryCount;
  final double latencyMean; // Mean latency for a query in seconds
  final double latency90; // 90th percentile for a query in seconds
  final double latencyFirstTokenMean; // Mean TTFT latency in seconds
  final double latencyFirstToken90; // 90th percentile TTFT in seconds
  final double tokenThroughput; // A.K.A. TPS
  final bool isMinDurationMet;
  final bool isMinQueryMet;
  final bool isEarlyStoppingMet;
  final bool isTokenBased;
  final bool isResultValid;

  LoadgenInfo({
    required this.queryCount,
    required this.latencyMean,
    required this.latency90,
    required this.latencyFirstTokenMean,
    required this.latencyFirstToken90,
    required this.tokenThroughput,
    required this.isMinDurationMet,
    required this.isMinQueryMet,
    required this.isEarlyStoppingMet,
    required this.isTokenBased,
    required this.isResultValid,
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
    // TODO possibly update these links
    // https://github.com/mlcommons/inference/blob/318cb131c0adf3bffcbc3379a502f40891331c54/loadgen/loadgen.cc#L1119
    const latencyKey = 'result_mean_latency_ns';
    const latencyFirstTokenKey = 'result_first_token_mean_latency_ns';
    // https://github.com/mlcommons/inference/blob/318cb131c0adf3bffcbc3379a502f40891331c54/loadgen/loadgen.cc#L1055
    const queryCountKey = 'result_query_count';
    // https://github.com/mlcommons/inference/blob/318cb131c0adf3bffcbc3379a502f40891331c54/loadgen/loadgen.cc#L1121-L1124
    const latency90Key = 'result_90.00_percentile_latency_ns';
    const latency90FirstTokenKey =
        'result_first_token_90.00_percentile_latency_ns';
    // https://github.com/mlcommons/inference/blob/318cb131c0adf3bffcbc3379a502f40891331c54/loadgen/loadgen.cc#L1028-L1029
    const validityKey = 'result_validity';
    // https://github.com/mlcommons/inference/blob/318cb131c0adf3bffcbc3379a502f40891331c54/loadgen/loadgen.cc#L1033C23-L1035
    const minDurationMetKey = 'result_min_duration_met';
    const minQueriesMetKey = 'result_min_queries_met';
    const earlyStoppingMetKey = 'early_stopping_met';
    const useTokenLatenciesKey = 'requested_use_token_latencies';
    const tokenThroughputKey =
        'result_token_throughput_with_loadgen_overhead'; // For some reason result_token_throughput is unreasonably low

    final result = await extractKeys(
      logLines: logLines,
      requiredKeys: {
        latencyKey,
        latencyFirstTokenKey,
        queryCountKey,
        latency90Key,
        latency90FirstTokenKey,
        validityKey,
        minDurationMetKey,
        minQueriesMetKey,
        earlyStoppingMetKey,
        useTokenLatenciesKey,
        tokenThroughputKey,
      },
    );

    if (result.isEmpty) {
      return null;
    }

    final isResultValid = result[validityKey] as String == 'VALID';

    const nanosecondsPerSecond = 1000 * Duration.microsecondsPerSecond;

    return LoadgenInfo(
      queryCount: result[queryCountKey] as int,
      latencyMean: (result[latencyKey] as int) / nanosecondsPerSecond,
      latencyFirstTokenMean:
          (result[latencyFirstTokenKey] as int) / nanosecondsPerSecond,
      latency90: (result[latency90Key] as int) / nanosecondsPerSecond,
      latencyFirstToken90:
          (result[latency90FirstTokenKey] as int) / nanosecondsPerSecond,
      tokenThroughput: result[tokenThroughputKey] as double,
      isMinDurationMet: result[minDurationMetKey] as bool,
      isMinQueryMet: result[minQueriesMetKey] as bool,
      isEarlyStoppingMet: result[earlyStoppingMetKey] as bool,
      isTokenBased: result[useTokenLatenciesKey] as bool,
      isResultValid: isResultValid,
    );
  }

  factory LoadgenInfo.fromJson(Map<String, dynamic> json) =>
      _$LoadgenInfoFromJson(json);

  Map<String, dynamic> toJson() => _$LoadgenInfoToJson(this);
}
