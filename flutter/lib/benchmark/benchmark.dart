import 'dart:io';

import 'package:collection/collection.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/backend/bridge/run_settings.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;
import 'package:mlperfbench/resources/resource_manager.dart';
import 'info.dart';
import 'run_mode.dart';

class BenchmarkResult {
  final double throughput;
  final String accuracy;
  final String backendName;
  final String acceleratorName;
  final int batchSize;
  final int threadsNumber;

  BenchmarkResult(
      {required this.throughput,
      required String accuracy,
      required this.backendName,
      required this.acceleratorName,
      required this.batchSize,
      required this.threadsNumber})
      : accuracy = _replaceAccuracy(accuracy);

  static String _replaceAccuracy(String accuracy) {
    if (accuracy != 'N/A') {
      const onlyNumbersWithPercentPattern = '[^.%0-9]';
      accuracy = accuracy.replaceAll(RegExp(onlyNumbersWithPercentPattern), '');
    }
    return accuracy;
  }

  String getFormattedAccuracyValue(BenchmarkTypeEnum type) {
    switch (type) {
      // if the benchmark type is unknown, just show the original string
      // so we know that this need to be fixed
      case BenchmarkTypeEnum.unknown:
        return accuracy;
      case BenchmarkTypeEnum.imageClassification:
        return (numericAccuracy * 100).toStringAsFixed(2);
      case BenchmarkTypeEnum.objectDetection:
        return (numericAccuracy * 100).toStringAsFixed(2);
      case BenchmarkTypeEnum.imageSegmentation:
        return (numericAccuracy * 100).toStringAsFixed(2);
      case BenchmarkTypeEnum.languageUnderstanding:
        return numericAccuracy.toStringAsFixed(2);
    }
  }

  double get numericAccuracy {
    if (!accuracy.endsWith('%')) {
      return double.tryParse(accuracy) ?? 0.0;
    }

    return (double.tryParse(accuracy.replaceAll(RegExp('%'), '')) ?? 0.0) / 100;
  }

  static const _tagThroughput = 'throughput';
  static const _tagAccuracy = 'accuracy';
  static const _tagBackendName = 'backend_name';
  static const _tagAcceleratorName = 'accelerator_name';
  static const _tagBatchSize = 'batch_size';
  static const _tagThreadsNumber = 'threads_number';

  static BenchmarkResult? fromJson(Map<String, dynamic>? json) {
    if (json == null) return null;
    return BenchmarkResult(
        throughput: json[_tagThroughput] as double,
        accuracy: json[_tagAccuracy] as String,
        backendName: json[_tagBackendName] as String,
        acceleratorName: json[_tagAcceleratorName] as String,
        batchSize: json[_tagBatchSize] as int,
        threadsNumber: json[_tagThreadsNumber] as int);
  }

  Map<String, dynamic> toJson() => {
        _tagThroughput: throughput,
        _tagAccuracy: accuracy,
        _tagBackendName: backendName,
        _tagAcceleratorName: acceleratorName,
        _tagBatchSize: batchSize,
        _tagThreadsNumber: threadsNumber,
      };
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

class Benchmark {
  final pb.BenchmarkSetting benchmarkSetting;
  final pb.TaskConfig taskConfig;
  final pb.ModelConfig modelConfig;
  final bool testMode;

  final BenchmarkInfo info;
  // this variable holds description of our config file,
  // which may not represent what backend actually used for computations
  final String backendRequestDescription;
  // TODO save config in the Store?
  final BenchmarkConfig config;

  BenchmarkResult? performanceModeResult;
  BenchmarkResult? accuracyModeResult;

  Benchmark(this.benchmarkSetting, this.taskConfig, this.modelConfig,
      this.testMode, BenchmarkConfig? predefinedConfig)
      : info = BenchmarkInfo(modelConfig, taskConfig.name),
        backendRequestDescription =
            '${benchmarkSetting.configuration} | ${benchmarkSetting.acceleratorDesc}',
        config = predefinedConfig ?? _getDefaultConfig(benchmarkSetting);

  String get id => modelConfig.id;

  static BenchmarkConfig _getDefaultConfig(
      pb.BenchmarkSetting benchmarkSetting) {
    // shardsCount is only supported by the TFLite backend.
    // On iPhones changing this value may significantly affect performance.
    // On Android this value does not affect performance as much,
    // (shards=2 is faster than shards=1, but I didn't notice any further improvements).
    // Originally this value was hardcoded to 2 in the backend
    // (before we made it configurable for iOS devices).
    const _defaultShardsCount = 2;
    // when creating run config we multiply batch size and shardsCount
    if (benchmarkSetting.batchSize >= _defaultShardsCount) {
      return BenchmarkConfig(BatchPreset(
          name: 'backend-defined',
          batchSize: benchmarkSetting.batchSize ~/ _defaultShardsCount,
          shardsCount: _defaultShardsCount));
    } else {
      return BenchmarkConfig(BatchPreset(
          name: 'backend-defined',
          batchSize: benchmarkSetting.batchSize,
          shardsCount: 1));
    }
  }

  RunSettings createRunSettings(
      {required BenchmarkRunMode runMode,
      required ResourceManager resourceManager,
      required List<pb.Setting> commonSettings,
      required String backendLibPath,
      required Directory tmpDir}) {
    final dataset =
        testMode ? taskConfig.testDataset : runMode.chooseDataset(taskConfig);

    final _fastMode = testMode || FAST_MODE;
    var minQueryCount = _fastMode ? 8 : taskConfig.minQueryCount;
    var minDuration = _fastMode ? 10 : taskConfig.minDurationMs;

    final settings = pb.SettingList(
      setting: commonSettings,
      benchmarkSetting: benchmarkSetting,
    );

    if (info.isOffline) {
      benchmarkSetting.batchSize = config.batchSize * config.threadsNumber;
      settings.setting.add(pb.Setting(
        id: 'shards_num',
        name: 'Number of threads for inference',
        value: pb.Setting_Value(
          name: config.threadsNumber.toString(),
          value: config.threadsNumber.toString(),
        ),
      ));
    }

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
      output_dir: tmpDir.path,
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
      this.testMode,
      BatchPreset? defaultPreset) {
    for (final task in mlperfConfig.task) {
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

  List<String> listResources(
      {bool skipInactive = false, bool includeAccuracy = true}) {
    final result = <String>[];

    for (final b in benchmarks) {
      if (skipInactive && !b.config.active) continue;
      if (testMode) {
        result.add(b.taskConfig.testDataset.path);
        result.add(b.taskConfig.testDataset.groundtruthSrc);
      } else {
        result.add(b.taskConfig.liteDataset.path);
        result.add(b.taskConfig.liteDataset.groundtruthSrc);

        if (includeAccuracy) {
          result.add(b.taskConfig.dataset.path);
          result.add(b.taskConfig.dataset.groundtruthSrc);
        }
      }
      result.add(b.benchmarkSetting.src);
    }

    final set = <String>{};
    result.retainWhere((x) => x.isNotEmpty && set.add(x));
    return result.where((element) => element.isNotEmpty).toList();
  }
}
