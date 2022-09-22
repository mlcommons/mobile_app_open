import 'package:json_annotation/json_annotation.dart';

import 'backend_info.dart';
import 'backend_settings.dart';
import 'dataset_info.dart';

part 'benchmark_result.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class Accuracy {
  final double normalized;
  final String formatted;

  Accuracy({
    required this.normalized,
    required this.formatted,
  });

  factory Accuracy.fromJson(Map<String, dynamic> json) =>
      _$AccuracyFromJson(json);

  Map<String, dynamic> toJson() => _$AccuracyToJson(this);

  bool isInBounds({double min = 0.0, double max = 1.0}) {
    if (!normalized.isFinite) return false;
    return normalized >= min && normalized <= max;
  }
}

@JsonSerializable(fieldRename: FieldRename.snake)
class BenchmarkLoadgenInfo {
  final bool validity;
  final double duration;

  BenchmarkLoadgenInfo({
    required this.validity,
    required this.duration,
  });

  factory BenchmarkLoadgenInfo.fromJson(Map<String, dynamic> json) =>
      _$BenchmarkLoadgenInfoFromJson(json);

  Map<String, dynamic> toJson() => _$BenchmarkLoadgenInfoToJson(this);
}

@JsonSerializable(fieldRename: FieldRename.snake)
class BenchmarkRunResult {
  final double? throughput;
  final Accuracy? accuracy;
  final Accuracy? accuracy2;
  final DatasetInfo dataset;
  final double measuredDuration;
  final int measuredSamples;
  final DateTime startDatetime;
  final BenchmarkLoadgenInfo? loadgenInfo;

  BenchmarkRunResult({
    required this.throughput,
    required this.accuracy,
    required this.accuracy2,
    required this.dataset,
    required this.measuredDuration,
    required this.measuredSamples,
    required this.startDatetime,
    required this.loadgenInfo,
  });

  factory BenchmarkRunResult.fromJson(Map<String, dynamic> json) =>
      _$BenchmarkRunResultFromJson(json);

  Map<String, dynamic> toJson() => _$BenchmarkRunResultToJson(this);
}

enum LoadgenScenarioEnum {
  @JsonValue('SingleStream')
  singleStream,
  @JsonValue('Offline')
  offline,
}

extension LoadgenScenarioExtension on LoadgenScenarioEnum {
  String get humanName {
    return _$LoadgenScenarioEnumEnumMap[this]!;
  }
}

@JsonSerializable(fieldRename: FieldRename.snake)
class BenchmarkExportResult {
  final String benchmarkId;
  final String benchmarkName;
  final LoadgenScenarioEnum loadgenScenario;
  final BackendSettingsInfo backendSettings;
  final BenchmarkRunResult? performanceRun;
  final BenchmarkRunResult? accuracyRun;
  final double minDuration;
  final int minSamples;
  final BackendReportedInfo backendInfo;

  BenchmarkExportResult({
    required this.benchmarkId,
    required this.benchmarkName,
    required this.loadgenScenario,
    required this.backendSettings,
    required this.performanceRun,
    required this.accuracyRun,
    required this.minDuration,
    required this.minSamples,
    required this.backendInfo,
  });

  factory BenchmarkExportResult.fromJson(Map<String, dynamic> json) =>
      _$BenchmarkExportResultFromJson(json);

  Map<String, dynamic> toJson() => _$BenchmarkExportResultToJson(this);

  static LoadgenScenarioEnum parseLoadgenScenario(String value) {
    return _$LoadgenScenarioEnumEnumMap.entries
        .firstWhere((element) => element.value == value)
        .key;
  }
}
