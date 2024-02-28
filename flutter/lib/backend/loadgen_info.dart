import 'dart:convert';
import 'dart:io';

import 'package:json_annotation/json_annotation.dart';

part 'loadgen_info.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class LoadgenInfo {
  final int queryCount;
  final double latencyMean; // Mean latency in seconds
  final double latency90; // 90th percentile in seconds
  final bool isMinDurationMet;
  final bool isMinQueryMet;
  final bool isEarlyStoppingMet;
  final bool isResultValid;

  LoadgenInfo({
    required this.queryCount,
    required this.latencyMean,
    required this.latency90,
    required this.isMinDurationMet,
    required this.isMinQueryMet,
    required this.isEarlyStoppingMet,
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
    // https://github.com/mlcommons/inference/blob/318cb131c0adf3bffcbc3379a502f40891331c54/loadgen/loadgen.cc#L1119
    const latencyKey = 'result_mean_latency_ns';
    // https://github.com/mlcommons/inference/blob/318cb131c0adf3bffcbc3379a502f40891331c54/loadgen/loadgen.cc#L1055
    const queryCountKey = 'result_query_count';
    // https://github.com/mlcommons/inference/blob/318cb131c0adf3bffcbc3379a502f40891331c54/loadgen/loadgen.cc#L1121-L1124
    const latency90Key = 'result_90.00_percentile_latency_ns';
    // https://github.com/mlcommons/inference/blob/318cb131c0adf3bffcbc3379a502f40891331c54/loadgen/loadgen.cc#L1028-L1029
    const validityKey = 'result_validity';
    // https://github.com/mlcommons/inference/blob/318cb131c0adf3bffcbc3379a502f40891331c54/loadgen/loadgen.cc#L1033C23-L1035
    const minDurationMetKey = 'result_min_duration_met';
    const minQueriesMetKey = 'result_min_queries_met';
    const earlyStoppingMetKey = 'early_stopping_met';

    final result = await extractKeys(
      logLines: logLines,
      requiredKeys: {
        latencyKey,
        queryCountKey,
        latency90Key,
        validityKey,
        minDurationMetKey,
        minQueriesMetKey,
        earlyStoppingMetKey,
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
      latency90: (result[latency90Key] as int) / nanosecondsPerSecond,
      isMinDurationMet: result[minDurationMetKey] as bool,
      isMinQueryMet: result[minQueriesMetKey] as bool,
      isEarlyStoppingMet: result[earlyStoppingMetKey] as bool,
      isResultValid: isResultValid,
    );
  }

  factory LoadgenInfo.fromJson(Map<String, dynamic> json) =>
      _$LoadgenInfoFromJson(json);

  Map<String, dynamic> toJson() => _$LoadgenInfoToJson(this);
}
