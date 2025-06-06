import 'package:collection/collection.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/backend/bridge/run_settings.dart';
import 'package:mlperfbench/backend/loadgen_info.dart';
import 'package:mlperfbench/benchmark/info.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';
import 'package:mlperfbench/data/results/benchmark_result.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;
import 'package:mlperfbench/resources/resource.dart';
import 'package:mlperfbench/resources/resource_manager.dart';

class BenchmarkResult {
  final Throughput? throughput;
  final Accuracy? accuracy;
  final Accuracy? accuracy2;
  final String backendName;
  final String acceleratorName;
  final String delegateName;
  final int batchSize;
  final LoadgenInfo? loadgenInfo;

  BenchmarkResult({
    required this.throughput,
    required this.accuracy,
    required this.accuracy2,
    required this.backendName,
    required this.acceleratorName,
    required this.delegateName,
    required this.batchSize,
    required this.loadgenInfo,
  });
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
        backendRequestDescription = benchmarkSettings.framework;

  String get id => taskConfig.id;

  pb.DelegateSetting get selectedDelegate {
    final delegate = benchmarkSettings.delegateChoice.firstWhere(
        (e) => e.delegateName == benchmarkSettings.delegateSelected);
    return delegate;
  }

  Future<RunSettings> createRunSettings({
    required BenchmarkRunMode runMode,
    required ResourceManager resourceManager,
    required List<pb.CommonSetting> commonSettings,
    required String backendLibName,
    required String logDir,
  }) async {
    final dataset = runMode.chooseDataset(taskConfig);
    final runConfig = runMode.chooseRunConfig(taskConfig);

    int minQueryCount = runConfig.minQueryCount;
    double minDuration = runConfig.minDuration;
    double maxDuration = runConfig.maxDuration;

    final settings = pb.SettingList(
      setting: commonSettings,
      benchmarkSetting: benchmarkSettings,
    );
    // Convert TaskConfig.CustomConfig to BenchmarkSetting.CustomSetting
    final customConfigs = taskConfig.customConfig
        .map((e) => pb.CustomSetting(id: e.id, value: e.value))
        .toList();
    benchmarkSettings.customSetting.addAll(customConfigs);
    final uris = selectedDelegate.modelFile.map((e) => e.modelPath).toList();
    final modelDirName = selectedDelegate.delegateName.replaceAll(' ', '_');
    final backendModelPath =
        await resourceManager.getModelPath(uris, modelDirName);
    return RunSettings(
      backend_model_path: backendModelPath,
      backend_lib_name: backendLibName,
      backend_settings: settings,
      backend_native_lib_path: DeviceInfo.instance.nativeLibraryPath,
      dataset_type: taskConfig.datasets.type.value,
      dataset_data_path: resourceManager.get(dataset.inputPath),
      dataset_groundtruth_path: resourceManager.get(dataset.groundtruthPath),
      model_offset: taskConfig.model.offset,
      model_num_classes: taskConfig.model.numClasses,
      model_image_width: taskConfig.model.imageWidth,
      model_image_height: taskConfig.model.imageHeight,
      scenario: taskConfig.scenario,
      mode: runMode.loadgenMode.name,
      batch_size: selectedDelegate.batchSize,
      min_query_count: minQueryCount,
      min_duration: minDuration,
      max_duration: maxDuration,
      single_stream_expected_latency_ns:
          benchmarkSettings.singleStreamExpectedLatencyNs,
      output_dir: logDir,
      benchmark_id: id,
    );
  }
}

class BenchmarkStore {
  final List<Benchmark> allBenchmarks = <Benchmark>[];

  List<Benchmark> get activeBenchmarks {
    return allBenchmarks.where((e) => e.isActive).toList();
  }

  BenchmarkStore({
    required pb.MLPerfConfig appConfig,
    required List<pb.BenchmarkSetting> backendConfig,
    required Map<String, bool> taskSelection,
  }) {
    // sort the order of task based on BenchmarkId.allIds
    final List<pb.TaskConfig> sortedTasks = List.from(appConfig.task)
      ..sort((a, b) =>
          BenchmarkId.allIds.indexOf(a.id) - BenchmarkId.allIds.indexOf(b.id));
    for (final task in sortedTasks) {
      final backendSettings = backendConfig
          .singleWhereOrNull((setting) => setting.benchmarkId == task.id);
      if (backendSettings == null) {
        print('No matching benchmark settings for task ${task.id}');
        continue;
      }

      final enabled = taskSelection[task.id] ?? true;
      allBenchmarks.add(Benchmark(
        taskConfig: task,
        benchmarkSettings: backendSettings,
        isActive: enabled,
      ));
    }
  }

  List<Resource> listResources({
    required List<BenchmarkRunMode> modes,
    required List<Benchmark> benchmarks,
  }) {
    final result = <Resource>[];

    for (final b in benchmarks) {
      for (var mode in modes) {
        final dataset = mode.chooseDataset(b.taskConfig);
        final data = Resource(
          type: ResourceTypeEnum.datasetData,
          path: dataset.inputPath,
          md5Checksum: dataset.inputChecksum,
        );
        final groundtruth = Resource(
          type: ResourceTypeEnum.datasetGroundtruth,
          path: dataset.groundtruthPath,
          md5Checksum: dataset.groundtruthChecksum,
        );
        result.addAll([data, groundtruth]);
      }

      for (final delegate in b.benchmarkSettings.delegateChoice) {
        for (final modelFile in delegate.modelFile) {
          final model = Resource(
            path: modelFile.modelPath,
            type: ResourceTypeEnum.model,
            md5Checksum: modelFile.modelChecksum,
          );
          result.add(model);
        }
      }
    }

    final set = <Resource>{};
    result.retainWhere((x) => x.path.isNotEmpty && set.add(x));
    return result.where((element) => element.path.isNotEmpty).toList();
  }

  Map<String, bool> get selection {
    Map<String, bool> result = {};
    for (var item in allBenchmarks) {
      result[item.id] = item.isActive;
    }
    return result;
  }
}
