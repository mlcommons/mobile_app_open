import 'backend_info.dart';
import 'backend_settings.dart';
import 'dataset_info.dart';
import 'loadgen_scenario.dart';

class BenchmarkRunResult {
  static const String _tagThroughput = 'throughput';
  static const String _tagAccuracy = 'accuracy';
  static const String _tagDatasetInfo = 'dataset';
  static const String _tagMeasuredDuration = 'measured_duration_ms';
  static const String _tagMeasuredSamples = 'measured_samples';
  static const String _tagStartDatetime = 'start_datetime';
  static const String _tagValidity = 'loadgen_validity';

  final double? throughput;
  final double? accuracy;
  final DatasetInfo datasetInfo;
  final double measuredDurationMs;
  final int measuredSamples;
  final DateTime startDatetime;
  final bool loadgenValidity;

  BenchmarkRunResult({
    required this.throughput,
    required this.accuracy,
    required this.datasetInfo,
    required this.measuredDurationMs,
    required this.measuredSamples,
    required this.startDatetime,
    required this.loadgenValidity,
  });

  BenchmarkRunResult.fromJson(Map<String, dynamic> json)
      : this(
          throughput: json[_tagThroughput] as double?,
          accuracy: json[_tagAccuracy] as double?,
          datasetInfo: DatasetInfo.fromJson(
              json[_tagDatasetInfo] as Map<String, dynamic>),
          measuredDurationMs: json[_tagMeasuredDuration] as double,
          measuredSamples: json[_tagMeasuredSamples] as int,
          startDatetime: DateTime.parse(json[_tagStartDatetime] as String),
          loadgenValidity: json[_tagValidity] as bool,
        );

  Map<String, dynamic> toJson() => {
        _tagThroughput: throughput,
        _tagAccuracy: accuracy,
        _tagDatasetInfo: datasetInfo,
        _tagMeasuredDuration: measuredDurationMs,
        _tagMeasuredSamples: measuredSamples,
        _tagStartDatetime: startDatetime.toUtc().toIso8601String(),
        _tagValidity: loadgenValidity,
      };
}

class BenchmarkExportResult {
  static const String _tagBenchmarkId = 'benchmark_id';
  static const String _tagBenchmarkName = 'benchmark_name';
  static const String _tagLoadgenScenario = 'loadgen_scenario';
  static const String _tagBackendSettings = 'backend_settings';
  static const String _tagPerformanceRun = 'performance_run';
  static const String _tagAccuracyRun = 'accuracy_run';
  static const String _tagMinDuration = 'min_duration_ms';
  static const String _tagMinSamples = 'min_samples';
  static const String _tagBackendInfo = 'backend_info';

  final String benchmarkId;
  final String benchmarkName;
  final LoadgenScenario loadgenScenario;
  final BackendSettingsInfo backendSettingsInfo;
  final BenchmarkRunResult? performance;
  final BenchmarkRunResult? accuracy;
  final double minDurationMs;
  final int minSamples;
  final BackendReportedInfo backendInfo;

  BenchmarkExportResult(
      {required this.benchmarkId,
      required this.benchmarkName,
      required this.loadgenScenario,
      required this.backendSettingsInfo,
      required this.performance,
      required this.accuracy,
      required this.minDurationMs,
      required this.minSamples,
      required this.backendInfo});

  BenchmarkExportResult.fromJson(Map<String, dynamic> json)
      : this(
            benchmarkId: json[_tagBenchmarkId] as String,
            benchmarkName: json[_tagBenchmarkName] as String,
            loadgenScenario:
                LoadgenScenario.fromJson(json[_tagLoadgenScenario] as String),
            backendSettingsInfo: BackendSettingsInfo.fromJson(
                json[_tagBackendSettings] as Map<String, dynamic>),
            performance: BenchmarkRunResult.fromJson(json[_tagPerformanceRun]),
            accuracy: json[_tagAccuracyRun] == null
                ? null
                : BenchmarkRunResult.fromJson(json[_tagAccuracyRun]),
            minDurationMs: json[_tagMinDuration] as double,
            minSamples: json[_tagMinSamples] as int,
            backendInfo: BackendReportedInfo.fromJson(json[_tagBackendInfo]));

  Map<String, dynamic> toJson() => {
        _tagBenchmarkId: benchmarkId,
        _tagBenchmarkName: benchmarkName,
        _tagLoadgenScenario: loadgenScenario,
        _tagBackendSettings: backendSettingsInfo,
        _tagPerformanceRun: performance,
        _tagAccuracyRun: accuracy,
        _tagMinDuration: minDurationMs,
        _tagMinSamples: minSamples,
        _tagBackendInfo: backendInfo
      };
}

class BenchmarkExportResultList {
  final List<BenchmarkExportResult> list;

  BenchmarkExportResultList(this.list);

  static BenchmarkExportResultList fromJson(List<dynamic> json) {
    final list = <BenchmarkExportResult>[];
    for (var item in json) {
      list.add(BenchmarkExportResult.fromJson(item as Map<String, dynamic>));
    }
    return BenchmarkExportResultList(list);
  }

  List<dynamic> toJson() {
    var result = <dynamic>[];
    for (var item in list) {
      result.add(item);
    }
    return result;
  }

  double calculateAverageThroughput() {
    var throughput = 0.0;
    var count = 0;
    for (var item in list) {
      if (item.performance == null) {
        continue;
      }
      throughput += item.performance!.throughput!;
      count++;
      ;
    }
    return throughput / count;
  }
}
