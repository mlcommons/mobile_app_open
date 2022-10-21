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
  final pb.BenchmarkSetting benchmarkSettings;
  final pb.TaskConfig taskConfig;
  bool isActive;

  final BenchmarkInfo info;

  // this variable holds description of our config file,
  // which may not represent what backend actually used for computations
  final String backendRequestDescription;

  BenchmarkResult? performanceModeResult;
  BenchmarkResult? accuracyModeResult;

  Benchmark({
    required this.benchmarkSettings,
    required this.taskConfig,
    required this.isActive,
  })  : info = BenchmarkInfo(taskConfig),
        backendRequestDescription =
            '${benchmarkSettings.configuration} | ${benchmarkSettings.acceleratorDesc}';

  String get id => taskConfig.id;

  RunSettings createRunSettings({
    required BenchmarkRunMode runMode,
    required ResourceManager resourceManager,
    required List<pb.Setting> commonSettings,
    required String backendLibName,
    required String logDir,
    required bool isTestMode,
  }) {
    final dataset = runMode.chooseDataset(taskConfig);

    final _fastMode = isTestMode || isFastMode;
    var minQueryCount = _fastMode ? 8 : taskConfig.minQueryCount;
    var minDuration = _fastMode ? 1.0 : taskConfig.minDuration;

    final settings = pb.SettingList(
      setting: commonSettings,
      benchmarkSetting: benchmarkSettings,
    );

    return RunSettings(
      backend_model_path: resourceManager.get(benchmarkSettings.modelPath),
      backend_lib_name: backendLibName,
      backend_settings: settings,
      backend_native_lib_path: DeviceInfo.instance.nativeLibraryPath,
      dataset_type: taskConfig.datasets.type.value,
      dataset_data_path: resourceManager.get(dataset.inputPath),
      dataset_groundtruth_path: resourceManager.get(dataset.groundtruthPath),
      dataset_offset: taskConfig.model.offset,
      scenario: taskConfig.scenario,
      mode: runMode.mode,
      min_query_count: minQueryCount,
      min_duration: minDuration,
      single_stream_expected_latency_ns:
          benchmarkSettings.singleStreamExpectedLatencyNs,
      output_dir: logDir,
      benchmark_id: id,
    );
  }
}

class BenchmarkList {
  final List<Benchmark> benchmarks = <Benchmark>[];

  BenchmarkList({
    required pb.MLPerfConfig appConfig,
    required List<pb.BenchmarkSetting> backendConfig,
    required Map<String, bool> taskSelection,
  }) {
    for (final task in appConfig.task) {
      final backendSettings = backendConfig
          .singleWhereOrNull((setting) => setting.benchmarkId == task.id);
      if (backendSettings == null) {
        print('No matching benchmark settings for task ${task.id}');
        continue;
      }

      final enabled = taskSelection[task.id] ?? true;
      benchmarks.add(Benchmark(
        taskConfig: task,
        benchmarkSettings: backendSettings,
        isActive: enabled,
      ));
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
        path: b.benchmarkSettings.modelPath,
        type: ResourceTypeEnum.model,
        md5Checksum: b.benchmarkSettings.modelChecksum,
      );
      result.add(model);
    }

    final set = <Resource>{};
    result.retainWhere((x) => x.path.isNotEmpty && set.add(x));
    return result.where((element) => element.path.isNotEmpty).toList();
  }

  Map<String, bool> get selection {
    Map<String, bool> result = {};
    for (var item in benchmarks) {
      result[item.id] = item.isActive;
    }
    return result;
  }
}
