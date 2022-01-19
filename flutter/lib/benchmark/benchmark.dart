import 'dart:io';
import 'dart:math';

import 'package:flutter/cupertino.dart';
import 'package:flutter/foundation.dart' show ChangeNotifier;
import 'package:flutter/material.dart' hide Icons;

import 'package:async/async.dart';
import 'package:collection/collection.dart';
import 'package:device_info/device_info.dart';
import 'package:flutter_svg/svg.dart';
import 'package:ios_utsname_ext/extension.dart';
import 'package:package_info_plus/package_info_plus.dart';
import 'package:path_provider/path_provider.dart';
import 'package:wakelock/wakelock.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/backend/bridge/ffi_config.dart';
import 'package:mlperfbench/backend/bridge/handle.dart';
import 'package:mlperfbench/backend/bridge/isolate.dart';
import 'package:mlperfbench/backend/list.dart';
import 'package:mlperfbench/backend/run_settings.dart';
import 'package:mlperfbench/benchmark/benchmark_result.dart';
import 'package:mlperfbench/icons.dart';
import 'package:mlperfbench/info.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;
import 'package:mlperfbench/resources/config_manager.dart';
import 'package:mlperfbench/resources/resource_manager.dart';
import 'package:mlperfbench/resources/utils.dart';
import 'package:mlperfbench/store.dart';

class Benchmark {
  final pb.BenchmarkSetting benchmarkSetting;
  final pb.TaskConfig taskConfig;
  final pb.ModelConfig modelConfig;

  double? score;
  String? accuracy;
  String backendDescription;

  Benchmark(this.benchmarkSetting, this.taskConfig, this.modelConfig)
      : backendDescription =
            '${benchmarkSetting.configuration} | ${benchmarkSetting.acceleratorDesc}';

  String get id => modelConfig.id;

  String get name => modelConfig.name;

  double get maxScore => MAX_SCORE[id]!;

  /// 'Object Detection', 'Image Classification (offline)', and so on.
  String get taskName => taskConfig.name;

  /// 'IC', 'OD', and so on.
  String get code => modelConfig.id.split('_').first;

  /// 'SingleStream' or 'Offline'.
  String get scenario => modelConfig.scenario;

  BenchmarkTypeEnum get type => _typeFromCode();

  SvgPicture get icon => BENCHMARK_ICONS[scenario]?[code] ?? Icons.logo;

  SvgPicture get iconWhite =>
      BENCHMARK_ICONS_WHITE[scenario]?[code] ?? Icons.logo;

  @override
  String toString() => 'Benchmark:$id';

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

class MiddleInterface {
  final List<Benchmark> benchmarks;

  MiddleInterface._(this.benchmarks);

  static Future<MiddleInterface> create(
      File configFile, BackendInfo backendInfo) async {
    final tasks = getMLPerfConfig(await configFile.readAsString());

    final benchmarks = <Benchmark>[];
    for (final task in tasks.task) {
      for (final model in task.model) {
        final benchmarkSetting = backendInfo.settings.benchmarkSetting
            .singleWhereOrNull((setting) => setting.benchmarkId == model.id);
        if (benchmarkSetting == null) continue;

        benchmarks.add(Benchmark(benchmarkSetting, task, model));
      }
    }

    return MiddleInterface._(benchmarks);
  }

  /// The list of URL or file names to download.
  List<String> data() {
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

enum BenchmarkStateEnum {
  downloading,
  cooldown,
  waiting,
  running,
  aborting,
  done,
}

class BenchmarkState extends ChangeNotifier {
  final Store _store;
  final BridgeIsolate backendBridge;

  late final ResourceManager resourceManager;
  late final ConfigManager configManager;
  late final BackendInfo backendInfo;

  // null - downloading/waiting; false - running; true - done
  bool? _doneRunning;
  bool _cooling = false;

  // Only if [state] == [BenchmarkStateEnum.downloading]
  String get downloadingProgress => resourceManager.progress;

  // Only if [state] == [BenchmarkStateEnum.running]
  Benchmark? currentlyRunning;
  String runningProgress = '';

  num get result {
    final benchmarksCount =
        benchmarks.where((benchmark) => benchmark.score != null).length;

    if (benchmarksCount == 0) return 0;

    final summaryScore = pow(
        benchmarks.fold<double>(1, (prev, i) {
          if (i.score != null) return prev * i.score!;
          return prev;
        }),
        1.0 / benchmarksCount);

    return summaryScore / _getSummaryMaxScore();
  }

  List<Benchmark> get benchmarks => _middle.benchmarks;

  // Settings from store
  bool _submissionMode = false;
  bool _testMode = false;
  bool _cooldown = false;
  int _cooldownPause = 0;
  Future<void> _cooldownFuture = Future.value();

  bool _aborting = false;

  late MiddleInterface _middle;

  BenchmarkState._(this._store, this.backendBridge) {
    resourceManager = ResourceManager(notifyListeners);
    backendInfo = BackendInfo.finMatching();
  }

  static double _getSummaryMaxScore() => MAX_SCORE['SUMMARY_MAX_SCORE']!;

  Future<String> validateExternalResourcesDirectory(
      String errorDescription) async {
    final datasetsError = <String>[];

    for (final job in _getBenchmarkJobs()) {
      final dataset = job.dataset;
      final groundTruthSrc = dataset.groundtruthSrc;

      if (!await resourceManager.isResourceExist(dataset.path) ||
          (!await resourceManager.isResourceExist(groundTruthSrc)) &&
              _store.submissionMode) {
        final error = dataset.type.name;
        if (!datasetsError.contains(error)) {
          datasetsError.add('$error');
        }
      }
    }

    if (datasetsError.isEmpty) return '';

    var index = 0;
    return errorDescription +
        datasetsError.map((element) => '\n${++index}) $element').join();
  }

  Future<String> validateOfflineMode(String errorDescription) async {
    if (!_store.offlineMode) {
      return '';
    }
    final errors = <String>[];
    for (final job in _getBenchmarkJobs()) {
      final modelPath = job.benchmark.benchmarkSetting.src;
      if (isInternetResource(modelPath)) {
        errors.add(modelPath);
      }
      final testDataPath = job.dataset.path;
      if (isInternetResource(testDataPath)) {
        errors.add(testDataPath);
      }
      final groundtruthDataPath = job.dataset.groundtruthSrc;
      if (isInternetResource(groundtruthDataPath)) {
        errors.add(groundtruthDataPath);
      }
    }

    if (errors.isEmpty) return '';

    var index = 0;
    return errorDescription +
        errors.map((element) => '\n${++index}) $element').join();
  }

  Future<void> clearCache() async {
    await resourceManager.cacheManager.deleteLoadedResources([], 0);
    await configManager.deleteDefaultConfig();
    _store.clearBenchmarkList();
    await resetConfig();
    await loadResources();
  }

  Future<void> loadResources() async {
    _middle = await MiddleInterface.create(
        File(configManager.configPath), backendInfo);
    _store.clearBenchmarkList();
    for (final benchmark in _middle.benchmarks) {
      BatchPreset? batchPreset;
      if (benchmark.modelConfig.scenario == 'Offline') {
        var presetList = resourceManager.getBatchPresets();

        if (Platform.isIOS) {
          var iosInfo = await DeviceInfoPlugin().iosInfo;
          final currentDevice = iosInfo.utsname.machine.iOSProductName;
          for (var p in presetList) {
            if (currentDevice.startsWith(p.name)) {
              batchPreset = p;
              break;
            }
          }
        } else if (Platform.isAndroid) {
          // shardsCount is only supported by the TFLite backend.
          // On iPhones changing this value may significantly affect performance.
          // On Android this value does not affect performance as much,
          // (shards=2 is faster than shards=1, but I didn't notice any further improvements).
          // Originally this value for hardcoded to 2 in the backend
          // (before we made it configurable for iOS devices).
          const defaultShardsCount = 2;
          batchPreset = BatchPreset(
              name: 'backend-defined',
              batchSize: benchmark.benchmarkSetting.batchSize,
              shardsCount: defaultShardsCount);
        }
        batchPreset ??= presetList[0];
      }

      _store.addBenchmarkToList(benchmark.id, benchmark.taskName,
          benchmark.backendDescription, batchPreset);
    }
    await reset();

    final packageInfo = await PackageInfo.fromPlatform();
    final newAppVersion = packageInfo.version + '+' + packageInfo.buildNumber;
    var needToPurgeCache = false;
    if (_store.previousAppVersion != newAppVersion) {
      _store.previousAppVersion = newAppVersion;
      needToPurgeCache = true;
    }

    await Wakelock.enable();
    resourceManager.handleResources(_middle.data(), needToPurgeCache);
    await Wakelock.disable();
  }

  Future<bool> _handlePreviousResult() async {
    if (_doneRunning == null) {
      return resourceManager.resultManager
          .restoreResults(_store.previousResult, benchmarks);
    } else {
      await _store.deletePreviousResult();
      await resourceManager.resultManager.delete();
    }

    return false;
  }

  static Future<BenchmarkState> create(Store store) async {
    final result = BenchmarkState._(store, await BridgeIsolate.create());

    await result.resourceManager.initSystemPaths();
    result.configManager = ConfigManager(
        result.resourceManager.applicationDirectory,
        store.chosenConfigurationName,
        result.resourceManager);
    await result.resourceManager.loadBatchPresets();
    await result.resetConfig();
    await result.loadResources();

    final loadFromStore = () {
      result._submissionMode = store.submissionMode;
      result._testMode = store.testMode;
      result._cooldown = store.cooldown;
      result._cooldownPause = store.cooldownPause;
    };
    store.addListener(loadFromStore);
    loadFromStore();
    return result;
  }

  Future<void> resetConfig({BenchmarksConfig? newConfig}) async {
    final config = newConfig ??
        await configManager.currentConfig ??
        configManager.defaultConfig;
    await configManager.setConfig(config);
    _store.chosenConfigurationName = config.name;
  }

  BenchmarkStateEnum get state {
    if (!resourceManager.done) return BenchmarkStateEnum.downloading;
    switch (_doneRunning) {
      case null:
        return BenchmarkStateEnum.waiting;
      case false:
        return _aborting
            ? BenchmarkStateEnum.aborting
            : _cooling
                ? BenchmarkStateEnum.cooldown
                : BenchmarkStateEnum.running;
      case true:
        return BenchmarkStateEnum.done;
    }
    throw StateError('unreachable');
  }

  List<BenchmarkJob> _getBenchmarkJobs() {
    final submissionMode = _submissionMode;
    final testMode = _testMode;
    final jobs = <BenchmarkJob>[];

    for (final benchmark in _middle.benchmarks) {
      var storedConfig = _store
          .getBenchmarkList()
          .firstWhere((element) => element.id == benchmark.id);
      if (!storedConfig.active) continue;
      benchmark.benchmarkSetting.batchSize = storedConfig.batchSize;
      jobs.add(BenchmarkJob(
        benchmark: benchmark,
        accuracy: false,
        threadsNumber: storedConfig.threadsNumber,
        testMode: testMode,
      ));

      if (!submissionMode) continue;

      jobs.add(BenchmarkJob(
        benchmark: benchmark,
        accuracy: true,
        threadsNumber: storedConfig.threadsNumber,
        testMode: testMode,
      ));
    }
    return jobs;
  }

  void runBenchmarks() async {
    await reset();

    assert(resourceManager.done, 'Resource manager is not done.');
    assert(_doneRunning == null, '_doneRunning is not null');
    _doneRunning = false;

    // disable screen sleep when benchmarks is running
    await Wakelock.enable();

    final cooldown = _cooldown;
    final cooldownPause = _cooldownPause;
    final jobs = _getBenchmarkJobs();

    var n = 0;
    var wasAccuracy = true;
    final results = <RunResult?>[];

    for (final job in jobs) {
      if (_aborting) break;

      if (cooldown && !job.accuracy && !wasAccuracy) {
        _cooling = true;
        notifyListeners();
        await (_cooldownFuture = Future.delayed(FAST_MODE
            ? Duration(seconds: 1)
            : Duration(minutes: cooldownPause)));
        _cooling = false;
        notifyListeners();
      }
      if (_aborting) break;
      wasAccuracy = job.accuracy;

      final resultFuture = job._run(resourceManager, backendBridge,
          backendInfo.settings.commonSetting, backendInfo.libPath);
      currentlyRunning = job.benchmark;
      runningProgress = '${(100 * (n++ / jobs.length)).round()}%';
      notifyListeners();

      final result = await resultFuture;
      results.add(result);

      job.benchmark.backendDescription = result?.backendDescription ?? 'N/A';
      if (job.accuracy) {
        job.benchmark.accuracy = result?.accuracy;
      } else {
        job.benchmark.score = result?.score;
      }
    }

    if (!_aborting) {
      await resourceManager.resultManager.writeResults(results);
      _store.previousResult =
          resourceManager.resultManager.serializeBriefResults(results);
    }

    currentlyRunning = null;
    _doneRunning = _aborting ? null : true;
    _aborting = false;
    notifyListeners();

    await Wakelock.disable();
  }

  Future<void> abortBenchmarks() async {
    if (_doneRunning == false) {
      _aborting = true;
      await CancelableOperation.fromFuture(_cooldownFuture).cancel();
      notifyListeners();
    }
  }

  Future<void> reset() async {
    final isPreviousResultUsed = await _handlePreviousResult();

    if (isPreviousResultUsed) {
      _doneRunning = true;
    } else {
      _middle.benchmarks
          .forEach((benchmark) => benchmark.accuracy = benchmark.score = null);
      _doneRunning = null;
    }

    _aborting = false;
  }
}

enum DatasetMode { lite, full, test }

class BenchmarkJob {
  final Benchmark benchmark;
  late final pb.DatasetConfig dataset;
  final bool accuracy;
  late final bool fast;
  late final DatasetMode _datasetMode;
  final int threadsNumber;

  BenchmarkJob({
    required this.benchmark,
    required this.accuracy,
    required this.threadsNumber,
    required bool testMode,
  }) {
    if (testMode) {
      _datasetMode = accuracy ? DatasetMode.full : DatasetMode.lite;
      dataset = accuracy
          ? benchmark.taskConfig.dataset
          : benchmark.taskConfig.liteDataset;
    } else {
      _datasetMode = DatasetMode.test;
      dataset = benchmark.taskConfig.testDataset;
    }
    fast = testMode || FAST_MODE;
  }

  Future<RunResult?> _run(
      ResourceManager resourceManager,
      BridgeIsolate backend,
      List<pb.Setting> commonSettings,
      String backendLibPath) async {
    final tmpDir = await getTemporaryDirectory();

    print(
        'Running $benchmark in ${accuracy ? 'accuracy' : 'performance'} mode...');
    final stopwatch = Stopwatch()..start();

    var minQueryCount = fast ? 8 : benchmark.taskConfig.minQueryCount;
    var minDuration = fast ? 10 : benchmark.taskConfig.minDurationMs;

    final settings = pb.SettingList(
      setting: commonSettings,
      benchmarkSetting: benchmark.benchmarkSetting,
    );

    final batchSizeValue = benchmark.benchmarkSetting.batchSize;

    if (benchmark.modelConfig.scenario == 'Offline') {
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

    var backendNativeLibPath = '';
    if (Platform.isAndroid) {
      backendNativeLibPath = await getNativeLibraryPath();
    }
    final result = await backend.run(RunSettings(
      backend_model_path: resourceManager.get(benchmark.benchmarkSetting.src),
      backend_lib_path: backendLibPath,
      backend_settings: settings.writeToBuffer(),
      backend_native_lib_path: backendNativeLibPath,
      dataset_type: benchmark.taskConfig.dataset.type.value,
      dataset_data_path: resourceManager.get(dataset.path),
      dataset_groundtruth_path: resourceManager.get(dataset.groundtruthSrc),
      dataset_offset: benchmark.modelConfig.offset,
      scenario: benchmark.modelConfig.scenario,
      batch: benchmark.benchmarkSetting.batchSize,
      batch_size: batchSizeValue,
      threads_number: threadsNumber,
      mode: accuracy
          ? BenchmarkMode.backendAccuracy
          : BenchmarkMode.backendPerfomance,
      min_query_count: minQueryCount,
      min_duration: minDuration,
      output_dir: tmpDir.path,
      benchmark_id: benchmark.id,
      dataset_mode: _datasetMode,
    ));
    final elapsed = stopwatch.elapsed;

    print('Benchmark result: $result, elapsed: $elapsed');
    return result;
  }
}
