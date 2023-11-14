import 'package:collection/collection.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/backend/bridge/run_settings.dart';
import 'package:mlperfbench/benchmark/info.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';
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
  final bool validity;

  BenchmarkResult(
      {required this.throughput,
      required this.accuracy,
      required this.accuracy2,
      required this.backendName,
      required this.acceleratorName,
      required this.delegateName,
      required this.batchSize,
      required this.validity});
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

  RunSettings createRunSettings({
    required BenchmarkRunMode runMode,
    required ResourceManager resourceManager,
    required List<pb.CommonSetting> commonSettings,
    required String backendLibName,
    required String logDir,
    required int testMinDuration,
    required int testMinQueryCount,
  }) {
    final dataset = runMode.chooseDataset(taskConfig);

    int minQueryCount;
    double minDuration;
    if (testMinDuration != 0) {
      minQueryCount = testMinQueryCount;
      minDuration = testMinDuration.toDouble();
    } else if (isFastMode) {
      minQueryCount = 8;
      minDuration = 1.0;
    } else {
      minQueryCount = taskConfig.minQueryCount;
      minDuration = taskConfig.minDuration;
    }
    double maxDuration = taskConfig.maxDuration;

    final settings = pb.SettingList(
      setting: commonSettings,
      benchmarkSetting: benchmarkSettings,
    );

    return RunSettings(
      backend_model_path: resourceManager.get(selectedDelegate.modelPath),
      backend_lib_name: backendLibName,
      backend_settings: settings,
      backend_native_lib_path: DeviceInfo.instance.nativeLibraryPath,
      dataset_type: taskConfig.datasets.type.value,
      dataset_data_path: resourceManager.get(dataset.inputPath),
      dataset_groundtruth_path: resourceManager.get(dataset.groundtruthPath),
      dataset_offset: taskConfig.model.offset,
      scenario: taskConfig.scenario,
      mode: runMode.loadgenMode,
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
  final List<Benchmark> benchmarks = <Benchmark>[];

  BenchmarkStore({
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

      for (final delegate in b.benchmarkSettings.delegateChoice) {
        final model = Resource(
          path: delegate.modelPath,
          type: ResourceTypeEnum.model,
          md5Checksum: delegate.modelChecksum,
        );
        result.add(model);
      }
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
