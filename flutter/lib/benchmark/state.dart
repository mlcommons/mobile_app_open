import 'dart:convert';
import 'dart:io';
import 'dart:math';

import 'package:flutter/cupertino.dart';
import 'package:flutter/foundation.dart' show ChangeNotifier;
import 'package:flutter/material.dart' hide Icons;

import 'package:async/async.dart';
import 'package:collection/collection.dart';
import 'package:mlperfbench/backend/bridge/run_result.dart';
import 'package:mlperfbench/build_info.dart';
import 'package:mlperfbench_common/data/environment/selected_backend_info.dart';
import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/results/backend_settings.dart';
import 'package:mlperfbench_common/data/results/backend_settings_extra.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';
import 'package:mlperfbench_common/data/results/dataset_info.dart';
import 'package:mlperfbench_common/data/results/dataset_type.dart';
import 'package:mlperfbench_common/data/results/loadgen_scenario.dart';
import 'package:mlperfbench_common/firebase/manager.dart';
import 'package:package_info_plus/package_info_plus.dart';
import 'package:path_provider/path_provider.dart';
import 'package:uuid/uuid.dart';
import 'package:wakelock/wakelock.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/backend/bridge/ffi_config.dart';
import 'package:mlperfbench/backend/bridge/isolate.dart';
import 'package:mlperfbench/backend/list.dart';
import 'package:mlperfbench/benchmark/info.dart';
import 'package:mlperfbench/benchmark/run_info.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'package:mlperfbench/resources/config_manager.dart';
import 'package:mlperfbench/resources/resource_manager.dart';
import 'package:mlperfbench/resources/utils.dart';
import 'package:mlperfbench/store.dart';
import 'benchmark.dart';
import 'run_mode.dart';

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
  final FirebaseManager? firebaseManager;

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
  ExtendedResult? lastResult;

  num get result {
    final benchmarksCount = benchmarks
        .where((benchmark) => benchmark.performanceModeResult != null)
        .length;

    if (benchmarksCount == 0) return 0;

    final summaryThroughput = pow(
        benchmarks.fold<double>(1, (prev, i) {
          return prev * (i.performanceModeResult?.throughput ?? 1.0);
        }),
        1.0 / benchmarksCount);

    return summaryThroughput / BenchmarkInfo.getSummaryMaxThroughput();
  }

  List<Benchmark> get benchmarks => _middle.benchmarks;

  Future<void> _cooldownFuture = Future.value();

  bool _aborting = false;

  late BenchmarkList _middle;

  BenchmarkState._(this._store, this.backendBridge, this.firebaseManager) {
    resourceManager = ResourceManager(notifyListeners);
    backendInfo = BackendInfo.findMatching();
  }

  Future<void> uploadLastResult() async {
    await firebaseManager!.restHelper.upload(lastResult!);
  }

  Future<String> validateExternalResourcesDirectory(
      String errorDescription) async {
    final resources = _middle.listResources(
        skipInactive: true, includeAccuracy: _store.submissionMode);
    final missing = await resourceManager.validateResourcesExist(resources);
    if (missing.isEmpty) return '';

    return errorDescription +
        missing.mapIndexed((i, element) => '\n${i + 1}) $element').join();
  }

  Future<String> validateOfflineMode(String errorDescription) async {
    final resources = _middle.listResources(
        skipInactive: true, includeAccuracy: _store.submissionMode);
    final internetResources = filterInternetResources(resources);
    if (internetResources.isEmpty) return '';

    return errorDescription +
        internetResources
            .mapIndexed((i, element) => '\n${i + 1}) $element')
            .join();
  }

  Future<void> clearCache() async {
    await resourceManager.cacheManager.deleteLoadedResources([], 0);
    await configManager.deleteDefaultConfig();
    await resetConfig();
    await loadResources();
  }

  Future<void> loadResources() async {
    final mlperfConfig =
        getMLPerfConfig(await File(configManager.configPath).readAsString());
    _middle = BenchmarkList(mlperfConfig, backendInfo.settings.benchmarkSetting,
        _store.testMode, resourceManager.getDefaultBatchPreset());
    await reset();

    final packageInfo = await PackageInfo.fromPlatform();
    final newAppVersion = packageInfo.version + '+' + packageInfo.buildNumber;
    var needToPurgeCache = false;
    if (_store.previousAppVersion != newAppVersion) {
      _store.previousAppVersion = newAppVersion;
      needToPurgeCache = true;
    }

    await Wakelock.enable();
    resourceManager.handleResources(_middle.listResources(), needToPurgeCache);
    await Wakelock.disable();
  }

  Future<bool> _handlePreviousResult() async {
    if (_doneRunning == null) {
      try {
        lastResult = ExtendedResult.fromJson(
            jsonDecode(_store.previousExtendedResult) as Map<String, dynamic>);
      } catch (e, trace) {
        print('unable to restore previous extended result: $e');
        print(trace);
      }
      return resourceManager.resultManager
          .restoreResults(_store.previousResult, benchmarks);
    } else {
      await _store.deletePreviousResult();
      await resourceManager.resultManager.delete();
    }

    return false;
  }

  static Future<BenchmarkState> create(
      Store store, FirebaseManager? firebaseManager) async {
    final result =
        BenchmarkState._(store, await BridgeIsolate.create(), firebaseManager);

    await result.resourceManager.initSystemPaths();
    result.configManager = ConfigManager(
        result.resourceManager.applicationDirectory,
        store.chosenConfigurationName,
        result.resourceManager);
    await result.resourceManager.loadBatchPresets();
    await result.resetConfig();
    await result.loadResources();
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

  void _updateProgress(double value) {
    runningProgress = '${(100 * value).round()}%';
    notifyListeners();
  }

  void runBenchmarks() async {
    await reset();

    assert(resourceManager.done, 'Resource manager is not done.');
    assert(_doneRunning == null, '_doneRunning is not null');
    _doneRunning = false;

    // disable screen sleep when benchmarks is running
    await Wakelock.enable();

    final cooldown = _store.cooldown;
    final cooldownPause = FAST_MODE
        ? Duration(seconds: 1)
        : Duration(minutes: _store.cooldownPause);

    final activeBenchmarks =
        _middle.benchmarks.where((element) => element.config.active);

    var doneCounter = 0.0;
    var doneMultiplier = _store.submissionMode ? 0.5 : 1.0;
    final exportResults = <BenchmarkExportResult>[];
    var first = true;

    for (final benchmark in activeBenchmarks) {
      currentlyRunning = benchmark;
      _updateProgress(doneCounter * doneMultiplier / activeBenchmarks.length);

      if (_aborting) break;

      // we only do cooldown before performance benchmarks
      if (cooldown && !first) {
        _cooling = true;
        notifyListeners();
        await (_cooldownFuture = Future.delayed(cooldownPause));
        _cooling = false;
        notifyListeners();
      }
      first = false;
      if (_aborting) break;

      final performanceResult = await runBenchmark(benchmark, false,
          backendInfo.settings.commonSetting, backendInfo.libPath);
      _updateProgress(doneCounter * doneMultiplier / activeBenchmarks.length);
      doneCounter++;

      benchmark.performanceModeResult = BenchmarkResult(
          throughput: performanceResult.throughput,
          accuracy: performanceResult.accuracy,
          backendName: performanceResult.backendName,
          acceleratorName: performanceResult.acceleratorName,
          batchSize: benchmark.config.batchSize,
          threadsNumber: benchmark.config.threadsNumber);
      
      DeviceInfo.setBackendInfo(SelectedBackendInfo(
        filename: backendInfo.libPath,
        vendor: performanceResult.backendVendor,
        name: performanceResult.backendName));

      if (_aborting) break;

      RunResult? accuracyResult;

      if (_store.submissionMode) {
        accuracyResult = await runBenchmark(benchmark, true,
            backendInfo.settings.commonSetting, backendInfo.libPath);
        _updateProgress(doneCounter * doneMultiplier / activeBenchmarks.length);
        doneCounter++;

        benchmark.accuracyModeResult = BenchmarkResult(
            // loadgen doesn't calculate latency for accuracy mode benchmarks
            // so throughput is infinity which is not a valid JSON numeric value
            throughput: accuracyResult.throughput.isFinite
                ? accuracyResult.throughput
                : 0.0,
            accuracy: accuracyResult.accuracy,
            backendName: accuracyResult.backendName,
            acceleratorName: accuracyResult.acceleratorName,
            batchSize: benchmark.config.batchSize,
            threadsNumber: benchmark.config.threadsNumber);
      }

      exportResults.add(exportResultFromRunInfo(benchmark, performanceResult, accuracyResult));
    }


    if (!_aborting) {
      lastResult = ExtendedResult(
        meta: null,
        // uuid: Uuid().v4(),
        envInfo: DeviceInfo.environmentInfo,
        results: BenchmarkExportResultList(exportResults), 
        buildInfo: BuildInfoHelper.info,
      );
      _store.previousExtendedResult =
          JsonEncoder().convert(lastResult!.toJson());
      await resourceManager.resultManager.writeResults(lastResult!.results);

      _store.previousResult = resourceManager.resultManager
          .serializeBriefResults(activeBenchmarks.toList());
    }

    currentlyRunning = null;
    _doneRunning = _aborting ? null : true;
    _aborting = false;
    notifyListeners();

    await Wakelock.disable();
  }

  static BenchmarkExportResult exportResultFromRunInfo(Benchmark benchmark, RunResult? performance, RunResult? accuracy) {
      return BenchmarkExportResult(
          benchmarkId: benchmark.id,
          benchmarkName: benchmark.taskConfig.name,
          performance: performance == null ? null : BenchmarkRunResult(
            score: performance.throughput,
            datasetInfo: DatasetInfo(
              name: benchmark.taskConfig.liteDataset.name, 
              type: DatasetType.fromJson(benchmark.taskConfig.liteDataset.type.toString()),
              dataPath: benchmark.taskConfig.liteDataset.path,
              groundtruthPath: benchmark.taskConfig.liteDataset.groundtruthSrc,
            ),
            measuredDurationMs: performance.durationMs,
            measuredSamples: performance.numSamples,
            startDatetime: performance.startTime.toIso8601String(),
            ), 
          accuracy: accuracy == null ? null : BenchmarkRunResult(
            score: 0.0, // TODO
            datasetInfo: DatasetInfo(
              name: benchmark.taskConfig.liteDataset.name, 
              type: DatasetType.fromJson(benchmark.taskConfig.liteDataset.type.toString()),
              dataPath: benchmark.taskConfig.liteDataset.path,
              groundtruthPath: benchmark.taskConfig.liteDataset.groundtruthSrc,
            ),
            measuredDurationMs: accuracy.durationMs,
            measuredSamples: accuracy.numSamples,
            startDatetime: accuracy.startTime.toIso8601String(),
            ), 
          minDurationMs: benchmark.taskConfig.minDurationMs.toDouble(),
          minSamples: benchmark.taskConfig.minQueryCount,
          // shardsNum: info.settings.threads_number,
          // batchSize: info.settings.batch_size,
          // mode: info.runMode.getResultModeString(),
          backendAcceleratorName: '', // TODO
          backendSettingsInfo: BackendSettingsInfo(
            acceleratorCode: benchmark.benchmarkSetting.accelerator,
             acceleratorDesc: benchmark.benchmarkSetting.acceleratorDesc,
              configuration: benchmark.benchmarkSetting.configuration,
               modelPath: benchmark.benchmarkSetting.src,
                extraSettings: BackendExtraSettingList([]) // TODO
                ),
          loadgenScenario: LoadgenScenario.fromJson(benchmark.modelConfig.scenario));
  }

  // BackendExtraSettingList transformExtraSettings() {
    
  // }

  Future<RunResult> runBenchmark(Benchmark benchmark, bool accuracyMode,
      List<pb.Setting> commonSettings, String backendLibPath) async {
    final tmpDir = await getTemporaryDirectory();

    final runMode =
        accuracyMode ? BenchmarkRunMode.accuracy : BenchmarkRunMode.performance;

    print(
        'Running ${benchmark.id} in ${runMode.getResultModeString()} mode...');
    final stopwatch = Stopwatch()..start();

    final runSettings = benchmark.createRunSettings(
        runMode: runMode,
        resourceManager: resourceManager,
        commonSettings: commonSettings,
        backendLibPath: backendLibPath,
        tmpDir: tmpDir);
    final result = await backendBridge.run(runSettings);
    final elapsed = stopwatch.elapsed;

    print('Benchmark result: $result, elapsed: $elapsed');
    return result;
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
      _middle.benchmarks.forEach((benchmark) => benchmark.accuracyModeResult =
          benchmark.performanceModeResult = null);
      _doneRunning = null;
    }

    _aborting = false;
  }
}
