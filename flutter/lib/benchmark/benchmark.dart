import 'dart:io';

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
  final pb.ModelConfig modelConfig;
  final bool testMode;

  bool isActive = true;

  final BenchmarkInfo info;

  // this variable holds description of our config file,
  // which may not represent what backend actually used for computations
  final String backendRequestDescription;

  BenchmarkResult? performanceModeResult;
  BenchmarkResult? accuracyModeResult;

  Benchmark(this.benchmarkSetting, this.taskConfig, this.modelConfig,
      this.testMode)
      : info = BenchmarkInfo(modelConfig, taskConfig.name),
        backendRequestDescription =
            '${benchmarkSetting.configuration} | ${benchmarkSetting.acceleratorDesc}';

  String get id => modelConfig.id;

  RunSettings createRunSettings(
      {required BenchmarkRunMode runMode,
      required ResourceManager resourceManager,
      required List<pb.Setting> commonSettings,
      required String backendLibPath,
      required String logDir}) {
    final dataset =
        testMode ? taskConfig.testDataset : runMode.chooseDataset(taskConfig);

    final _fastMode = testMode || FAST_MODE;
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
      dataset_type: taskConfig.dataset.type.value,
      dataset_data_path: resourceManager.get(dataset.path),
      dataset_groundtruth_path: resourceManager.get(dataset.groundtruthSrc),
      dataset_offset: modelConfig.offset,
      scenario: modelConfig.scenario,
      mode: runMode.getBackendModeString(),
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
  final bool testMode;

  BenchmarkList(
      pb.MLPerfConfig mlperfConfig,
      List<pb.BenchmarkSetting> benchmarkSettings,
      this.testMode) {
    for (final task in mlperfConfig.task) {
      for (final model in task.model) {
        final benchmarkSetting = benchmarkSettings
            .singleWhereOrNull((setting) => setting.benchmarkId == model.id);
        if (benchmarkSetting == null) continue;

        benchmarks
            .add(Benchmark(benchmarkSetting, task, model, testMode));
      }
    }
  }

  List<Resource> listResources(
      {bool skipInactive = false, bool includeAccuracy = true}) {
    final result = <Resource>[];

    for (final b in benchmarks) {
      if (skipInactive && !b.isActive) continue;
      if (testMode) {
        final datasetData = Resource(
            path: b.taskConfig.testDataset.path,
            type: ResourceTypeEnum.datasetData);
        final datasetGroundtruth = Resource(
            path: b.taskConfig.testDataset.groundtruthSrc,
            type: ResourceTypeEnum.datasetGroundtruth);
        result.addAll([datasetData, datasetGroundtruth]);
      } else {
        final datasetData = Resource(
            path: b.taskConfig.liteDataset.path,
            type: ResourceTypeEnum.datasetData);
        final datasetGroundtruth = Resource(
            path: b.taskConfig.liteDataset.groundtruthSrc,
            type: ResourceTypeEnum.datasetGroundtruth);
        result.addAll([datasetData, datasetGroundtruth]);

        if (includeAccuracy) {
          final datasetData = Resource(
              path: b.taskConfig.dataset.path,
              type: ResourceTypeEnum.datasetData);
          final datasetGroundtruth = Resource(
              path: b.taskConfig.dataset.groundtruthSrc,
              type: ResourceTypeEnum.datasetGroundtruth);
          result.addAll([datasetData, datasetGroundtruth]);
        }
      }
      final model = Resource(
          path: b.benchmarkSetting.src,
          type: ResourceTypeEnum.model,
          md5Checksum: b.benchmarkSetting.md5Checksum);
      result.add(model);
    }

    final set = <Resource>{};
    result.retainWhere((x) => x.path.isNotEmpty && set.add(x));
    return result.where((element) => element.path.isNotEmpty).toList();
  }
}
