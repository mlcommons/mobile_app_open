import 'dart:convert';

import 'package:collection/collection.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/backend/bridge/run_settings.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;
import 'package:mlperfbench/resources/resource.dart';
import 'package:mlperfbench/resources/resource_manager.dart';
import 'info.dart';
import 'run_mode.dart';

class Benchmark {
  final pb.BenchmarkSetting benchmarkSettings;
  final pb.TaskConfig taskConfig;
  bool isActive;

  final BenchmarkInfo info;

  // this variable holds description of our config file,
  // which may not represent what backend actually used for computations
  final String backendRequestDescription;

  Benchmark({
    required this.benchmarkSettings,
    required this.taskConfig,
    required this.isActive,
  })  : info = BenchmarkInfo(taskConfig),
        backendRequestDescription =
            '${benchmarkSettings.framework} | ${benchmarkSettings.acceleratorDesc}';

  String get id => taskConfig.id;

  RunSettings createRunSettings({
    required BenchmarkRunMode runMode,
    required ResourceManager resourceManager,
    required List<pb.Setting> commonSettings,
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
      mode: runMode.loadgenMode,
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
  }) {
    for (final task in appConfig.task) {
      final backendSettings = backendConfig
          .singleWhereOrNull((setting) => setting.benchmarkId == task.id);
      if (backendSettings == null) {
        print('No matching benchmark settings for task ${task.id}');
        continue;
      }

      benchmarks.add(Benchmark(
        taskConfig: task,
        benchmarkSettings: backendSettings,
        isActive: true,
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

  static String serializeTaskSelection(Map<String, bool> selection) {
    return jsonEncode(selection);
  }

  static Map<String, bool> deserializeTaskSelection(String serialized) {
    if (serialized.isEmpty) {
      return {};
    }
    try {
      final json = jsonDecode(serialized) as Map<String, dynamic>;
      Map<String, bool> result = {};
      for (final e in json.entries) {
        result[e.key] = e.value as bool;
      }
      return result;
    } catch (e, t) {
      print('task selection parse fail: $e');
      print(t);
      return {};
    }
  }

  Map<String, bool> getTaskSelection() {
    Map<String, bool> selection = {};
    for (var item in benchmarks) {
      selection[item.id] = item.isActive;
    }
    return selection;
  }

  void restoreSelection(Map<String, bool> selection) {
    const stateForUnknown = true;

    for (var task in benchmarks) {
      task.isActive = selection[task.id] ?? stateForUnknown;
    }
  }
}
