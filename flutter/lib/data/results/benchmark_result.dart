import 'package:json_annotation/json_annotation.dart';

import 'package:mlperfbench/data/results/backend_info.dart';
import 'package:mlperfbench/data/results/backend_settings.dart';
import 'package:mlperfbench/data/results/dataset_info.dart';

part 'benchmark_result.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class Throughput implements Comparable<Throughput> {
  final double value;

  const Throughput({
    required this.value,
  });

  factory Throughput.fromJson(Map<String, dynamic> json) =>
      _$ThroughputFromJson(json);

  Map<String, dynamic> toJson() => _$ThroughputToJson(this);

  @override
  int compareTo(Throughput other) {
    final diff = value - other.value;
    if (diff > 0.0) {
      return 1;
    } else if (diff < 0.0) {
      return -1;
    } else {
      return 0;
    }
  }

  bool operator >(Throughput other) {
    return compareTo(other) > 0;
  }

  bool operator <(Throughput other) {
    return compareTo(other) < 0;
  }

  @override
  bool operator ==(Object other) {
    return (other is Throughput) && compareTo(other) == 0;
  }

  @override
  int get hashCode => value.hashCode;

  @override
  String toString() => value.toString();

  String toUIString() => value.toStringAsFixed(2);
}

@JsonSerializable(fieldRename: FieldRename.snake)
class Accuracy implements Comparable<Accuracy> {
  final double normalized;
  final String formatted;

  const Accuracy({
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

  @override
  int compareTo(Accuracy other) {
    final diff = normalized - other.normalized;
    if (diff > 0.0) {
      return 1;
    } else if (diff < 0.0) {
      return -1;
    } else {
      return 0;
    }
  }

  bool operator >(Accuracy other) {
    return compareTo(other) > 0;
  }

  bool operator <(Accuracy other) {
    return compareTo(other) < 0;
  }

  @override
  bool operator ==(Object other) {
    return (other is Accuracy) && compareTo(other) == 0;
  }

  @override
  int get hashCode => normalized.hashCode;

  @override
  String toString() => normalized.toString();

  // We want to display 0.12345 as 12.3 in the overview screen.
  String toUIString() => (normalized * 100).toStringAsFixed(1);
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
  final Throughput? throughput;
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
  final BackendReportedInfo backendInfo;
  final BenchmarkRunResult? performanceRun;
  final BenchmarkRunResult? accuracyRun;
  final double minDuration;
  final int minSamples;

  BenchmarkExportResult({
    required this.benchmarkId,
    required this.benchmarkName,
    required this.loadgenScenario,
    required this.backendSettings,
    required this.backendInfo,
    required this.performanceRun,
    required this.accuracyRun,
    required this.minDuration,
    required this.minSamples,
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
