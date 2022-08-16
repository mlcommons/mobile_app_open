import 'package:collection/collection.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/backend/bridge/run_settings.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;
import 'package:mlperfbench/resources/resource.dart';
import 'package:mlperfbench/resources/resource_manager.dart';
import 'info.dart';
import 'run_mode.dart';

class BenchmarkResult {
  final double throughput;
  final Accuracy? accuracy;
  final Accuracy? accuracy2;
  final String backendName;
  final String acceleratorName;
  final int batchSize;
  final bool validity;

  BenchmarkResult(
      {required this.throughput,
      required this.accuracy,
      required this.accuracy2,
      required this.backendName,
      required this.acceleratorName,
      required this.batchSize,
      required this.validity});

  static const _tagThroughput = 'throughput';
  static const _tagAccuracy = 'accuracy';
  static const _tagAccuracy2 = 'accuracy2';
  static const _tagBackendName = 'backend_name';
  static const _tagAcceleratorName = 'accelerator_name';
  static const _tagBatchSize = 'batch_size';
  static const _tagValidity = 'validity';

  static BenchmarkResult? fromJson(Map<String, dynamic>? json) {
    if (json == null) return null;
    return BenchmarkResult(
      throughput: json[_tagThroughput] as double,
      accuracy: Accuracy.fromJson(json[_tagAccuracy] as Map<String, dynamic>),
      accuracy2: Accuracy.fromJson(json[_tagAccuracy2] as Map<String, dynamic>),
      backendName: json[_tagBackendName] as String,
      acceleratorName: json[_tagAcceleratorName] as String,
      batchSize: json[_tagBatchSize] as int,
      validity: json[_tagValidity] as bool,
    );
  }

  Map<String, dynamic> toJson() => {
        _tagThroughput: throughput,
        _tagAccuracy: accuracy,
        _tagAccuracy2: accuracy2,
        _tagBackendName: backendName,
        _tagAcceleratorName: acceleratorName,
        _tagBatchSize: batchSize,
        _tagValidity: validity,
      };
}

class Benchmark {
  final pb.BenchmarkSetting benchmarkSetting;
  final pb.TaskConfig taskConfig;

  bool isActive = true;

  final BenchmarkInfo info;

  // this variable holds description of our config file,
  // which may not represent what backend actually used for computations
  final String backendRequestDescription;

  BenchmarkResult? performanceModeResult;
  BenchmarkResult? accuracyModeResult;

  Benchmark(
    this.benchmarkSetting,
    this.taskConfig,
  )   : info = BenchmarkInfo(taskConfig),
        backendRequestDescription =
            '${benchmarkSetting.configuration} | ${benchmarkSetting.acceleratorDesc}';

  String get id => taskConfig.id;

  RunSettings createRunSettings({
    required BenchmarkRunMode runMode,
    required ResourceManager resourceManager,
    required List<pb.Setting> commonSettings,
    required String backendLibPath,
    required String logDir,
    required bool isTestMode,
  }) {
    final dataset = runMode.chooseDataset(taskConfig);

    final _fastMode = isTestMode || isFastMode;
    var minQueryCount = _fastMode ? 8 : taskConfig.minQueryCount;
    var minDuration = _fastMode ? 10 : taskConfig.minDurationMs;

    final settings = pb.SettingList(
      setting: commonSettings,
      benchmarkSetting: benchmarkSetting,
    );

    return RunSettings(
      backend_model_path: resourceManager.get(benchmarkSetting.src),
      backend_lib_path: backendLibPath,
      backend_settings: settings,
      backend_native_lib_path: DeviceInfo.nativeLibraryPath,
      dataset_type: taskConfig.datasets.type.value,
      dataset_data_path: resourceManager.get(dataset.inputPath),
      dataset_groundtruth_path: resourceManager.get(dataset.groundtruthPath),
      dataset_offset: taskConfig.model.offset,
      scenario: taskConfig.scenario,
      mode: runMode.mode,
      min_query_count: minQueryCount,
      min_duration: minDuration,
      single_stream_expected_latency_ns:
          benchmarkSetting.singleStreamExpectedLatencyNs,
      output_dir: logDir,
      benchmark_id: id,
    );
  }
}

class BenchmarkList {
  final List<Benchmark> benchmarks = <Benchmark>[];

  BenchmarkList(
    pb.MLPerfConfig mlperfConfig,
    List<pb.BenchmarkSetting> benchmarkSettings,
  ) {
    for (final task in mlperfConfig.task) {
      final benchmarkSetting = benchmarkSettings
          .singleWhereOrNull((setting) => setting.benchmarkId == task.id);
      if (benchmarkSetting == null) continue;

      benchmarks.add(Benchmark(benchmarkSetting, task));
    }
  }

  List<Resource> listResources({
    required List<BenchmarkRunMode> modes,
    bool skipInactive = false,
  }) {
    final result = <Resource>[];

    for (final b in benchmarks) {
      if (skipInactive && !b.isActive) continue;

      for (var mode in modes) {
        final dataset = mode.chooseDataset(b.taskConfig);
        final data = Resource(
          path: dataset.inputPath,
          type: ResourceTypeEnum.datasetData,
        );
        final groundtruth = Resource(
          path: dataset.groundtruthPath,
          type: ResourceTypeEnum.datasetGroundtruth,
        );
        result.addAll([data, groundtruth]);
      }

      final model = Resource(
        path: b.benchmarkSetting.src,
        type: ResourceTypeEnum.model,
        md5Checksum: b.benchmarkSetting.md5Checksum,
      );
      result.add(model);
    }

    final set = <Resource>{};
    result.retainWhere((x) => x.path.isNotEmpty && set.add(x));
    return result.where((element) => element.path.isNotEmpty).toList();
  }
}
