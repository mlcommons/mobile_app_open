import 'dart:io';

import 'package:collection/collection.dart';
import 'package:flutter_svg/svg.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/backend/bridge/ffi_config.dart';
import 'package:mlperfbench/backend/run_settings.dart';
import 'package:mlperfbench/benchmark/benchmark_result.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/icons.dart';
import 'package:mlperfbench/info.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;
import 'package:mlperfbench/resources/resource_manager.dart';

class BenchmarkInfo {
  final pb.ModelConfig modelConfig;

  /// 'Object Detection', 'Image Classification (offline)', and so on.
  final String taskName;

  BenchmarkInfo(this.modelConfig, this.taskName);

  String get name => modelConfig.name;

  bool get isOffline => modelConfig.scenario == 'Offline';

  double get maxScore => MAX_SCORE[modelConfig.id]!;

  /// 'IC', 'OD', and so on.
  String get code => modelConfig.id.split('_').first;

  /// 'SingleStream' or 'Offline'.
  String get scenario => modelConfig.scenario;

  BenchmarkTypeEnum get type => _typeFromCode();

  SvgPicture get icon => BENCHMARK_ICONS[scenario]?[code] ?? Icons.logo;

  SvgPicture get iconWhite =>
      BENCHMARK_ICONS_WHITE[scenario]?[code] ?? Icons.logo;

  @override
  String toString() => 'Benchmark:${modelConfig.id}';

  BenchmarkTypeEnum _typeFromCode() {
    switch (code) {
      case 'IC':
        return BenchmarkTypeEnum.imageClassification;
      case 'OD':
        return BenchmarkTypeEnum.objectDetection;
      case 'IS':
        return BenchmarkTypeEnum.imageSegmentation;
      case 'LU':
        return BenchmarkTypeEnum.languageUnderstanding;
      default:
        return BenchmarkTypeEnum.unknown;
    }
  }
}

class BenchmarkResult {
  final double score;
  final String accuracy;
  final String backendName;

  BenchmarkResult(this.score, this.accuracy, this.backendName);
}

class BenchmarkConfig {
  bool active = true;
  BatchPreset? batchPreset;
  int batchSize = 0;
  int threadsNumber = 0;

  BenchmarkConfig(this.batchPreset) {
    if (batchPreset != null) {
      batchSize = batchPreset!.batchSize;
      threadsNumber = batchPreset!.shardsCount;
    }
  }
}

// shardsCount is only supported by the TFLite backend.
// On iPhones changing this value may significantly affect performance.
// On Android this value does not affect performance as much,
// (shards=2 is faster than shards=1, but I didn't notice any further improvements).
// Originally this value for hardcoded to 2 in the backend
// (before we made it configurable for iOS devices).
const _defaultShardsCount = 2;

class Benchmark {
  final pb.BenchmarkSetting benchmarkSetting;
  final pb.TaskConfig taskConfig;
  final pb.ModelConfig modelConfig;
  final bool testMode;

  final BenchmarkInfo info;
  final String backendDescription;
  // TODO save config in the Store?
  final BenchmarkConfig config;

  BenchmarkResult? performance;
  BenchmarkResult? accuracy;

  Benchmark(this.benchmarkSetting, this.taskConfig, this.modelConfig,
      this.testMode, BenchmarkConfig? predefinedConfig)
      : info = BenchmarkInfo(modelConfig, taskConfig.name),
        backendDescription =
            '${benchmarkSetting.configuration} | ${benchmarkSetting.acceleratorDesc}',
        config = predefinedConfig ?? _getDefaultConfig(benchmarkSetting);

  String get id => modelConfig.id;

  static BenchmarkConfig _getDefaultConfig(
      pb.BenchmarkSetting benchmarkSetting) {
    return BenchmarkConfig(BatchPreset(
        name: 'backend-defined',
        batchSize: benchmarkSetting.batchSize,
        shardsCount: _defaultShardsCount));
  }

  BenchmarkJob createPerformanceJob(int threadsNumber) {
    return BenchmarkJob._(
        benchmark: this,
        accuracyMode: false,
        threadsNumber: threadsNumber,
        testMode: testMode);
  }

  BenchmarkJob createAccuracyJob(int threadsNumber) {
    return BenchmarkJob._(
        benchmark: this,
        accuracyMode: true,
        threadsNumber: threadsNumber,
        testMode: testMode);
  }
}

class BenchmarkList {
  final List<Benchmark> benchmarks = <Benchmark>[];

  BenchmarkList(String config, List<pb.BenchmarkSetting> benchmarkSettings,
      bool testMode, BatchPreset? defaultPreset) {
    final tasks = getMLPerfConfig(config);

    for (final task in tasks.task) {
      for (final model in task.model) {
        final benchmarkSetting = benchmarkSettings
            .singleWhereOrNull((setting) => setting.benchmarkId == model.id);
        if (benchmarkSetting == null) continue;

        BenchmarkConfig? config;
        if (defaultPreset != null) {
          config = BenchmarkConfig(defaultPreset);
        }
        benchmarks
            .add(Benchmark(benchmarkSetting, task, model, testMode, config));
      }
    }
  }

  /// The list of URL or file names to download.
  List<String> data() {
    // TODO unify with listSelectedResources
    final result = <String>[];

    for (final b in benchmarks) {
      result.add(b.taskConfig.liteDataset.path);
      result.add(b.taskConfig.liteDataset.groundtruthSrc);

      result.add(b.taskConfig.dataset.path);
      result.add(b.taskConfig.dataset.groundtruthSrc);

      result.add(b.taskConfig.testDataset.path);
      result.add(b.taskConfig.testDataset.groundtruthSrc);

      result.add(b.benchmarkSetting.src);
    }

    result.sort();
    return result.where((element) => element.isNotEmpty).toList();
  }
}

enum BenchmarkTypeEnum {
  unknown,
  imageClassification,
  objectDetection,
  imageSegmentation,
  languageUnderstanding,
}

enum DatasetMode { lite, full, test }

class BenchmarkJob {
  final Benchmark benchmark;
  late final pb.DatasetConfig dataset;
  final bool accuracyMode;
  late final bool _fastMode;
  late final DatasetMode _datasetMode;
  final int threadsNumber;

  BenchmarkJob._({
    required this.benchmark,
    required this.accuracyMode,
    required this.threadsNumber,
    required bool testMode,
  }) {
    if (testMode) {
      _datasetMode = DatasetMode.test;
      dataset = benchmark.taskConfig.testDataset;
    } else {
      _datasetMode = accuracyMode ? DatasetMode.full : DatasetMode.lite;
      dataset = accuracyMode
          ? benchmark.taskConfig.dataset
          : benchmark.taskConfig.liteDataset;
    }
    _fastMode = testMode || FAST_MODE;
  }

  RunSettings createRunSettings(
      ResourceManager resourceManager,
      List<pb.Setting> commonSettings,
      String backendLibPath,
      Directory tmpDir) {
    var minQueryCount = _fastMode ? 8 : benchmark.taskConfig.minQueryCount;
    var minDuration = _fastMode ? 10 : benchmark.taskConfig.minDurationMs;

    final settings = pb.SettingList(
      setting: commonSettings,
      benchmarkSetting: benchmark.benchmarkSetting,
    );

    final batchSizeValue = benchmark.benchmarkSetting.batchSize;

    if (benchmark.info.isOffline) {
      var shardsNumSetting = benchmark.benchmarkSetting.customSetting
          .singleWhereOrNull((element) => element.id == 'shards_num');
      if (shardsNumSetting == null) {
        benchmark.benchmarkSetting.customSetting.add(pb.CustomSetting(
            id: 'shards_num', value: threadsNumber.toString()));
      } else {
        shardsNumSetting.value = threadsNumber.toString();
      }
      var batchSizeSetting = benchmark.benchmarkSetting.customSetting
          .singleWhereOrNull((element) => element.id == 'batch_size');
      if (batchSizeSetting == null) {
        benchmark.benchmarkSetting.customSetting.add(pb.CustomSetting(
            id: 'batch_size', value: batchSizeValue.toString()));
      } else {
        batchSizeSetting.value = batchSizeValue.toString();
      }

      benchmark.benchmarkSetting.batchSize *= threadsNumber;
      settings.setting.add(pb.Setting(
        id: 'shards_num',
        name: 'Number of threads for inference',
        value: pb.Setting_Value(
          name: threadsNumber.toString(),
          value: threadsNumber.toString(),
        ),
      ));
    }

    return RunSettings(
      backend_model_path: resourceManager.get(benchmark.benchmarkSetting.src),
      backend_lib_path: backendLibPath,
      backend_settings: settings.writeToBuffer(),
      backend_native_lib_path: DeviceInfo.nativeLibraryPath,
      dataset_type: benchmark.taskConfig.dataset.type.value,
      dataset_data_path: resourceManager.get(dataset.path),
      dataset_groundtruth_path: resourceManager.get(dataset.groundtruthSrc),
      dataset_offset: benchmark.modelConfig.offset,
      scenario: benchmark.modelConfig.scenario,
      batch: benchmark.benchmarkSetting.batchSize,
      batch_size: batchSizeValue,
      threads_number: threadsNumber,
      mode: accuracyMode
          ? BenchmarkMode.backendAccuracy
          : BenchmarkMode.backendPerfomance,
      min_query_count: minQueryCount,
      min_duration: minDuration,
      output_dir: tmpDir.path,
      benchmark_id: benchmark.id,
      dataset_mode: _datasetMode,
    );
  }
}
