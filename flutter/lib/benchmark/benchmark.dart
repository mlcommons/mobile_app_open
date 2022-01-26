import 'dart:io';

import 'package:collection/collection.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/backend/run_settings.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;
import 'package:mlperfbench/resources/resource_manager.dart';
import 'info.dart';

class BenchmarkResult {
  final double score;
  final String accuracy;
  final String backendName;
  final int batchSize;
  final int threadsNumber;

  BenchmarkResult(
      {required this.score,
      required String accuracy,
      required this.backendName,
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

  static const _tagScore = 'score';
  static const _tagAccuracy = 'accuracy';
  static const _tagBackendName = 'backend_name';
  static const _tagbatchSize = 'batch_size';
  static const _tagThreadsNumber = 'threads_number';

  static BenchmarkResult? fromJson(Map<String, dynamic>? json) {
    if (json == null) return null;
    return BenchmarkResult(
        score: json[_tagScore] as double,
        accuracy: json[_tagAccuracy] as String,
        backendName: json[_tagBackendName] as String,
        batchSize: json[_tagbatchSize] as int,
        threadsNumber: json[_tagThreadsNumber] as int);
  }

  Map<String, dynamic> toJson() => {
        _tagScore: score,
        _tagAccuracy: accuracy,
        _tagBackendName: backendName,
        _tagbatchSize: batchSize,
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

enum BenchmarkRunModeEnum { performance, accuracy }

class BenchmarkRunMode {
  final BenchmarkRunModeEnum _mode;

  // Only some of these constants are used in the app
  // Let's keep them for now,
  // they are used by other apps that work with mlperf benchmark
  // they can potentially be used in the future in this app
  static final String backendPerfomanceString = 'PerformanceOnly';
  static final String backendAccuracyString = 'AccuracyOnly';
  static final String backendSubmissionString = 'SubmissionRun';

  static final String resultSubmissionString = 'submission_mode';
  static final String resultAccuracyString = 'accuracy_mode';
  static final String resultPerformanceString = 'performance_mode';
  static final String resultPerformance_liteString = 'performance_lite_mode';
  static final String resultTestString = 'testing';

  BenchmarkRunMode._(this._mode);

  static BenchmarkRunMode performance =
      BenchmarkRunMode._(BenchmarkRunModeEnum.performance);
  static BenchmarkRunMode accuracy =
      BenchmarkRunMode._(BenchmarkRunModeEnum.accuracy);

  String getBackendModeString() {
    switch (_mode) {
      case BenchmarkRunModeEnum.performance:
        return backendPerfomanceString;
      case BenchmarkRunModeEnum.accuracy:
        return backendAccuracyString;
      default:
        throw 'unhandled BenchmarkRunModeEnum';
    }
  }

  String getResultModeString() {
    switch (_mode) {
      case BenchmarkRunModeEnum.performance:
        return resultPerformance_liteString;
      case BenchmarkRunModeEnum.accuracy:
        return resultAccuracyString;
      default:
        throw 'unhandled BenchmarkRunModeEnum';
    }
  }

  pb.DatasetConfig chooseDatase(pb.TaskConfig taskConfig) {
    switch (_mode) {
      case BenchmarkRunModeEnum.performance:
        return taskConfig.liteDataset;
      case BenchmarkRunModeEnum.accuracy:
        return taskConfig.dataset;
      default:
        throw 'unhandled BenchmarkRunModeEnum';
    }
  }
}

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
        testMode ? taskConfig.testDataset : runMode.chooseDatase(taskConfig);

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
      backend_settings: settings.writeToBuffer(),
      backend_native_lib_path: DeviceInfo.nativeLibraryPath,
      dataset_type: taskConfig.dataset.type.value,
      dataset_data_path: resourceManager.get(dataset.path),
      dataset_groundtruth_path: resourceManager.get(dataset.groundtruthSrc),
      dataset_offset: modelConfig.offset,
      scenario: modelConfig.scenario,
      batch: benchmarkSetting.batchSize,
      batch_size: config.batchSize,
      threads_number: config.threadsNumber,
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
