import 'backend_settings.dart';
import 'dataset_info.dart';
import 'loadgen_scenario.dart';

class BenchmarkRunResult {
  static const String _tagScore = 'score';
  static const String _tagDatasetInfo = 'dataset';
  static const String _tagMeasuredDuration = 'measured_duration_ms';
  static const String _tagMeasuredSamples = 'measured_samples';
  static const String _tagStartDatetime = 'start_datetime';

  final double score;
  final DatasetInfo datasetInfo;
  final double measuredDurationMs;
  final int measuredSamples;
  final String startDatetime;

  BenchmarkRunResult({
    required this.score, 
    required this.datasetInfo,
      required this.measuredDurationMs,
      required this.measuredSamples,
      required this.startDatetime,
  });

  BenchmarkRunResult.fromJson(Map<String, dynamic> json)
  : this(
     score: json[_tagScore] as double,
     datasetInfo: DatasetInfo.fromJson(
                json[_tagDatasetInfo] as Map<String, dynamic>),
            measuredDurationMs: json[_tagMeasuredDuration] as double,
            measuredSamples: json[_tagMeasuredSamples] as int,
            startDatetime: json[_tagStartDatetime] as String,
  );

  Map<String, dynamic> toJson() => {
    _tagScore: score,
    _tagDatasetInfo: datasetInfo,
        _tagMeasuredDuration: measuredDurationMs,
        _tagMeasuredSamples: measuredSamples,
        _tagStartDatetime: startDatetime,
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
  static const String _tagBackendAcceleratorName = 'backend_accelerator_name';

  final String benchmarkId;
  final String benchmarkName;
  final LoadgenScenario loadgenScenario;
  final BackendSettingsInfo backendSettingsInfo;
  final BenchmarkRunResult? performance;
  final BenchmarkRunResult? accuracy;
  final double minDurationMs;
  final int minSamples;
  final String backendAcceleratorName;

  BenchmarkExportResult(
      {required this.benchmarkId,
      required this.benchmarkName,
      required this.loadgenScenario,
      required this.backendSettingsInfo,
      required this.performance,
      required this.accuracy,
      required this.minDurationMs,
      required this.minSamples,
      required this.backendAcceleratorName});

  BenchmarkExportResult.fromJson(Map<String, dynamic> json)
      : this(
            benchmarkId: json[_tagBenchmarkId] as String,
            benchmarkName: json[_tagBenchmarkName] as String,
            loadgenScenario:
                LoadgenScenario.fromJson(json[_tagLoadgenScenario] as String),
            backendSettingsInfo: BackendSettingsInfo.fromJson(
                json[_tagBackendSettings] as Map<String, dynamic>),
            performance: BenchmarkRunResult.fromJson(json[_tagPerformanceRun]),
            accuracy: BenchmarkRunResult.fromJson(json[_tagAccuracyRun]),
            minDurationMs: json[_tagMinDuration] as double,
            minSamples: json[_tagMinSamples] as int,
            backendAcceleratorName: json[_tagBackendAcceleratorName] as String);

  Map<String, dynamic> toJson() => {
        _tagBenchmarkId: benchmarkId,
        _tagBenchmarkName: benchmarkName,
        _tagLoadgenScenario: loadgenScenario,
        _tagBackendSettings: backendSettingsInfo,
        _tagPerformanceRun: performance,
        _tagAccuracyRun: accuracy,
        _tagMinDuration: minDurationMs,
        _tagMinSamples: minSamples,
        _tagBackendAcceleratorName: backendAcceleratorName
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
      throughput += item.performance!.score;
      count++;;
    }
    return throughput / count;
  }
}
