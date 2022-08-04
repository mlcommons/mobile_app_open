import 'dart:async';
import 'dart:convert';
import 'dart:io';
import 'dart:math';

import 'package:flutter/cupertino.dart';
import 'package:flutter/foundation.dart' show ChangeNotifier;
import 'package:flutter/material.dart';

import 'package:async/async.dart';
import 'package:collection/collection.dart';
import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/meta_info.dart';
import 'package:mlperfbench_common/data/results/backend_info.dart';
import 'package:mlperfbench_common/data/results/backend_settings.dart';
import 'package:mlperfbench_common/data/results/backend_settings_extra.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';
import 'package:mlperfbench_common/data/results/dataset_info.dart';
import 'package:mlperfbench_common/data/results/dataset_type.dart';
import 'package:mlperfbench_common/data/results/loadgen_scenario.dart';
import 'package:mlperfbench_common/firebase/manager.dart';
import 'package:uuid/uuid.dart';
import 'package:wakelock/wakelock.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/backend/bridge/isolate.dart';
import 'package:mlperfbench/backend/bridge/run_result.dart';
import 'package:mlperfbench/backend/list.dart';
import 'package:mlperfbench/benchmark/info.dart';
import 'package:mlperfbench/benchmark/run_info.dart';
import 'package:mlperfbench/build_info.dart';
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
  waiting,
  running,
  aborting,
  done,
}

class ProgressInfo {
  bool cooldown = false;
  bool accuracy = false;
  BenchmarkInfo? info;
  int totalStages = 0;
  int currentStage = 0;
  int cooldownDurationMs = 0;
  double get stageProgress => calculateStageProgress?.call() ?? 0.0;

  double Function()? calculateStageProgress;
}

class BenchmarkState extends ChangeNotifier {
  final Store _store;
  final BridgeIsolate backendBridge;
  final FirebaseManager? firebaseManager;

  late final ResourceManager resourceManager;
  late final ConfigManager configManager;
  late final BackendInfo backendInfo;

  Object? error;
  StackTrace? stackTrace;

  bool taskConfigFailedToLoad = false;
  String currentLogDir = '';
  // null - downloading/waiting; false - running; true - done
  bool? _doneRunning;

  // Only if [state] == [BenchmarkStateEnum.downloading]
  String get downloadingProgress => resourceManager.progress;

  // Only if [state] == [BenchmarkStateEnum.running]
  ProgressInfo progressInfo = ProgressInfo();
  // Benchmark? currentlyRunning;
  // String runningProgress = '';

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

    final maxSummaryThroughput = pow(
        benchmarks.fold<double>(1, (prev, i) {
          return prev * (i.info.maxThroughput);
        }),
        1.0 / benchmarksCount);

    return summaryThroughput / maxSummaryThroughput;
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

  BenchmarkRunMode get perfMode => _store.testMode
      ? BenchmarkRunMode.performanceTest
      : BenchmarkRunMode.performance;

  BenchmarkRunMode get accuracyMode => _store.testMode
      ? BenchmarkRunMode.accuracyTest
      : BenchmarkRunMode.accuracy;

  List<BenchmarkRunMode> get selectedRunModes {
    final result = <BenchmarkRunMode>[];
    result.add(perfMode);
    if (_store.submissionMode) {
      result.add(accuracyMode);
    }
    return result;
  }

  Future<String> validateExternalResourcesDirectory(
      String errorDescription) async {
    final resources =
        _middle.listResources(modes: selectedRunModes, skipInactive: true);
    final missing = await resourceManager.validateResourcesExist(resources);
    if (missing.isEmpty) return '';

    return errorDescription +
        missing.mapIndexed((i, element) => '\n${i + 1}) $element').join();
  }

  Future<String> validateOfflineMode(String errorDescription) async {
    final resources =
        _middle.listResources(modes: selectedRunModes, skipInactive: true);
    final internetResources = filterInternetResources(resources);
    if (internetResources.isEmpty) return '';

    return errorDescription +
        internetResources
            .mapIndexed((i, element) => '\n${i + 1}) $element')
            .join();
  }

  Future<void> clearCache() async {
    await resourceManager.cacheManager.deleteLoadedResources([], 0);
    notifyListeners();
    try {
      await setTaskConfig(name: _store.chosenConfigurationName);
      deferredLoadResources();
    } catch (e, trace) {
      print("can't load resources: $e");
      print(trace);
      error = e;
      stackTrace = trace;
      taskConfigFailedToLoad = true;
      notifyListeners();
    }
  }

  // Start loading resources in background.
  // Return type 'void' is intended, this function must not be awaited.
  void deferredLoadResources() async {
    try {
      await loadResources();
    } catch (e, trace) {
      print("can't load resources: $e");
      print(trace);
      error = e;
      stackTrace = trace;
      taskConfigFailedToLoad = true;
      notifyListeners();
    }
  }

  Future<void> loadResources() async {
    final newAppVersion =
        BuildInfoHelper.info.version + '+' + BuildInfoHelper.info.buildNumber;
    var needToPurgeCache = _store.previousAppVersion != newAppVersion;
    _store.previousAppVersion = newAppVersion;

    await Wakelock.enable();
    print('start loading resources');
    await resourceManager.handleResources(
      _middle.listResources(
        modes: [perfMode, accuracyMode],
        skipInactive: false,
      ),
      needToPurgeCache,
    );
    print('finished loading resources');
    error = null;
    stackTrace = null;
    taskConfigFailedToLoad = false;
    await Wakelock.disable();
  }

  static Future<BenchmarkState> create(
      Store store, FirebaseManager? firebaseManager) async {
    final result =
        BenchmarkState._(store, await BridgeIsolate.create(), firebaseManager);

    await result.resourceManager.initSystemPaths();
    result.configManager = ConfigManager(
        result.resourceManager.applicationDirectory, result.resourceManager);
    try {
      await result.setTaskConfig(name: store.chosenConfigurationName);
      result.deferredLoadResources();
    } catch (e, trace) {
      print("can't load resources: $e");
      print(trace);
      result.error = e;
      result.stackTrace = trace;
      result.taskConfigFailedToLoad = true;
    }
    return result;
  }

  /// Reads config but doesn't update resources that depend on config.
  /// Call loadResources() to update dependent resources.
  ///
  /// Can throw an exception.
  Future<void> setTaskConfig({required String name}) async {
    if (name == '') {
      name = configManager.defaultConfig.name;
    }
    await configManager.loadConfig(name);
    _store.chosenConfigurationName = name;
    error = null;
    stackTrace = null;
    taskConfigFailedToLoad = false;

    _middle = BenchmarkList(
      configManager.decodedConfig,
      backendInfo.settings.benchmarkSetting,
    );
    restoreLastResult();
  }

  BenchmarkStateEnum get state {
    if (!resourceManager.done) return BenchmarkStateEnum.downloading;
    switch (_doneRunning) {
      case null:
        return BenchmarkStateEnum.waiting;
      case false:
        return _aborting
            ? BenchmarkStateEnum.aborting
            : BenchmarkStateEnum.running;
      case true:
        return BenchmarkStateEnum.done;
    }
    throw StateError('unreachable');
  }

  Future<void> runBenchmarks() async {
    assert(resourceManager.done, 'Resource manager is not done.');
    assert(_doneRunning != false, '_doneRunning is false');
    _store.previousExtendedResult = '';
    _doneRunning = false;

    // disable screen sleep when benchmarks is running
    await Wakelock.enable();

    try {
      await _runBenchmarks();
      print('Benchmarks finished');
      _doneRunning = _aborting ? null : true;
    } catch (e) {
      _doneRunning = null;
      rethrow;
    } finally {
      if (currentLogDir.isNotEmpty && !_store.keepLogs) {
        await Directory(currentLogDir).delete(recursive: true);
      }

      _aborting = false;

      notifyListeners();

      await Wakelock.disable();
    }
  }

  Future<void> _runBenchmarks() async {
    final cooldown = _store.cooldown;
    final cooldownPause = _store.testMode || isFastMode
        ? const Duration(seconds: 1)
        : Duration(minutes: _store.cooldownDuration);

    final activeBenchmarks =
        _middle.benchmarks.where((element) => element.isActive);

    final exportResults = <BenchmarkExportResult>[];
    var first = true;

    progressInfo.totalStages = activeBenchmarks.length;
    if (_store.submissionMode) {
      progressInfo.totalStages += activeBenchmarks.length;
    }
    progressInfo.currentStage = 0;
    progressInfo.cooldownDurationMs = cooldownPause.inMilliseconds;

    resetCurrentResults();

    final startTime = DateTime.now();
    final logDirName = startTime.toIso8601String().replaceAll(':', '-');
    currentLogDir = '${resourceManager.applicationDirectory}/logs/$logDirName';

    for (final benchmark in activeBenchmarks) {
      progressInfo.info = benchmark.info;

      // increment counter for performance benchmark before cooldown
      progressInfo.currentStage++;

      if (_aborting) break;

      // we only do cooldown before performance benchmarks
      if (cooldown && !first) {
        progressInfo.cooldown = true;
        final timer = Stopwatch()..start();
        progressInfo.calculateStageProgress = () {
          return timer.elapsedMilliseconds / progressInfo.cooldownDurationMs;
        };
        notifyListeners();
        await (_cooldownFuture = Future.delayed(cooldownPause));
        progressInfo.cooldown = false;
        timer.stop();
      }
      first = false;
      if (_aborting) break;

      final perfTimer = Stopwatch()..start();
      progressInfo.accuracy = false;
      progressInfo.calculateStageProgress = () {
        final timeProgress =
            perfTimer.elapsedMilliseconds / benchmark.taskConfig.minDurationMs;
        final queryProgress = backendBridge.getQueryCounter() /
            benchmark.taskConfig.minQueryCount;
        return min(timeProgress, queryProgress);
      };
      notifyListeners();
      final performanceRunInfo = await runBenchmark(
        benchmark,
        perfMode,
        backendInfo.settings.commonSetting,
        backendInfo.libPath,
        currentLogDir,
      );
      perfTimer.stop();

      final performanceResult = performanceRunInfo.result;
      benchmark.performanceModeResult = BenchmarkResult(
        throughput: performanceResult.throughput,
        accuracy: performanceResult.accuracyNormalized < 0.0
            ? null
            : Accuracy(
                normalized: performanceResult.accuracyNormalized,
                formatted: performanceResult.accuracyFormatted,
              ),
        accuracy2: performanceResult.accuracyNormalized2 < 0.0
            ? null
            : Accuracy(
                normalized: performanceResult.accuracyNormalized2,
                formatted: performanceResult.accuracyFormatted2,
              ),
        backendName: performanceResult.backendName,
        acceleratorName: performanceResult.acceleratorName,
        batchSize: benchmark.benchmarkSetting.batchSize,
        validity: performanceResult.validity,
      );

      if (_aborting) break;

      RunResult? accuracyResult;

      if (_store.submissionMode) {
        progressInfo.currentStage++;
        progressInfo.accuracy = true;
        progressInfo.calculateStageProgress = () {
          final queryProgress =
              backendBridge.getQueryCounter() / backendBridge.getDatasetSize();
          return queryProgress;
        };
        notifyListeners();
        final accuracyRunInfo = await runBenchmark(
          benchmark,
          accuracyMode,
          backendInfo.settings.commonSetting,
          backendInfo.libPath,
          currentLogDir,
        );

        accuracyResult = accuracyRunInfo.result;
        benchmark.accuracyModeResult = BenchmarkResult(
          // loadgen doesn't calculate latency for accuracy mode benchmarks
          // so throughput is infinity which is not a valid JSON numeric value
          throughput: accuracyResult.throughput.isFinite
              ? accuracyResult.throughput
              : 0.0,
          accuracy: accuracyResult.accuracyNormalized < 0.0
              ? null
              : Accuracy(
                  normalized: accuracyResult.accuracyNormalized,
                  formatted: accuracyResult.accuracyFormatted,
                ),
          accuracy2: accuracyResult.accuracyNormalized2 < 0.0
              ? null
              : Accuracy(
                  normalized: accuracyResult.accuracyNormalized2,
                  formatted: accuracyResult.accuracyFormatted2,
                ),
          backendName: accuracyResult.backendName,
          acceleratorName: accuracyResult.acceleratorName,
          batchSize: benchmark.benchmarkSetting.batchSize,
          validity: accuracyResult.validity,
        );
      }

      exportResults.add(exportResultFromRunInfo(benchmark, performanceResult,
          accuracyResult, performanceRunInfo.settings.backend_settings));
    }

    if (!_aborting) {
      lastResult = ExtendedResult(
        meta: ResultMetaInfo(uuid: const Uuid().v4()),
        envInfo: DeviceInfo.environmentInfo,
        results: BenchmarkExportResultList(exportResults),
        buildInfo: BuildInfoHelper.info,
      );
      _store.previousExtendedResult =
          const JsonEncoder().convert(lastResult!.toJson());
      await resourceManager.resultManager.saveResult(lastResult!);
    }
  }

  void resetCurrentResults() {
    for (var b in _middle.benchmarks) {
      b.accuracyModeResult = null;
      b.performanceModeResult = null;
    }
  }

  BenchmarkExportResult exportResultFromRunInfo(
      Benchmark benchmark,
      RunResult performance,
      RunResult? accuracy,
      pb.SettingList actualSettings) {
    final performanceDataset = perfMode.chooseDataset(benchmark.taskConfig);
    final accuracyDataset = accuracyMode.chooseDataset(benchmark.taskConfig);
    return BenchmarkExportResult(
        benchmarkId: benchmark.id,
        benchmarkName: benchmark.taskConfig.name,
        performance: BenchmarkRunResult(
          throughput: performance.throughput,
          accuracy: performance.accuracyNormalized < 0.0
              ? null
              : Accuracy(
                  normalized: performance.accuracyNormalized,
                  formatted: performance.accuracyFormatted,
                ),
          accuracy2: performance.accuracyNormalized2 < 0.0
              ? null
              : Accuracy(
                  normalized: performance.accuracyNormalized2,
                  formatted: performance.accuracyFormatted2,
                ),
          datasetInfo: DatasetInfo(
            name: accuracyDataset.name,
            type: DatasetType.fromJson(performanceDataset.type.toString()),
            dataPath: performanceDataset.path,
            groundtruthPath: performanceDataset.groundtruthSrc,
          ),
          measuredDurationMs: performance.durationMs,
          measuredSamples: performance.numSamples,
          startDatetime: performance.startTime,
          loadgenValidity: performance.validity,
        ),
        accuracy: accuracy == null
            ? null
            : BenchmarkRunResult(
                throughput:
                    accuracy.throughput.isFinite ? accuracy.throughput : null,
                accuracy: accuracy.accuracyNormalized < 0.0
                    ? null
                    : Accuracy(
                        normalized: accuracy.accuracyNormalized,
                        formatted: accuracy.accuracyFormatted,
                      ),
                accuracy2: accuracy.accuracyNormalized2 < 0.0
                    ? null
                    : Accuracy(
                        normalized: accuracy.accuracyNormalized2,
                        formatted: accuracy.accuracyFormatted2,
                      ),
                datasetInfo: DatasetInfo(
                  name: accuracyDataset.name,
                  type: DatasetType.fromJson(accuracyDataset.type.toString()),
                  dataPath: accuracyDataset.path,
                  groundtruthPath: accuracyDataset.groundtruthSrc,
                ),
                measuredDurationMs: accuracy.durationMs,
                measuredSamples: accuracy.numSamples,
                startDatetime: accuracy.startTime,
                loadgenValidity: accuracy.validity,
              ),
        minDurationMs: benchmark.taskConfig.minDurationMs.toDouble(),
        minSamples: benchmark.taskConfig.minQueryCount,
        backendInfo: BackendReportedInfo(
          filename: backendInfo.libPath,
          name: performance.backendName,
          vendor: performance.backendVendor,
          accelerator: performance.acceleratorName,
        ),
        backendSettingsInfo: BackendSettingsInfo(
          acceleratorCode: actualSettings.benchmarkSetting.accelerator,
          acceleratorDesc: actualSettings.benchmarkSetting.acceleratorDesc,
          configuration: actualSettings.benchmarkSetting.configuration,
          modelPath: actualSettings.benchmarkSetting.src,
          batchSize: actualSettings.benchmarkSetting.batchSize,
          extraSettings: extraSettingsFromCommon(actualSettings.setting),
        ),
        loadgenScenario:
            LoadgenScenario.fromJson(benchmark.modelConfig.scenario));
  }

  static BackendExtraSettingList extraSettingsFromCommon(
      List<pb.Setting> commonSettings) {
    final list = <BackendExtraSetting>[];
    for (var item in commonSettings) {
      list.add(BackendExtraSetting(
          id: item.id,
          name: item.name,
          value: item.value.value,
          valueName: item.value.name));
    }
    return BackendExtraSettingList(list);
  }

  Future<RunInfo> runBenchmark(
    Benchmark benchmark,
    BenchmarkRunMode runMode,
    List<pb.Setting> commonSettings,
    String backendLibPath,
    String logDir,
  ) async {
    print('Running ${benchmark.id} in ${runMode.mode} mode...');
    final stopwatch = Stopwatch()..start();

    logDir = '$logDir/${benchmark.id}-${runMode.logSuffix}';
    await Directory(logDir).create(recursive: true);

    final runSettings = benchmark.createRunSettings(
      runMode: runMode,
      resourceManager: resourceManager,
      commonSettings: commonSettings,
      backendLibPath: backendLibPath,
      logDir: logDir,
      isTestMode: _store.testMode,
    );
    final result = await backendBridge.run(runSettings);
    final elapsed = stopwatch.elapsed;

    print('Run result: id: ${benchmark.id}, $result, elapsed: $elapsed');

    return RunInfo(settings: runSettings, result: result);
  }

  Future<void> abortBenchmarks() async {
    if (_doneRunning == false) {
      _aborting = true;
      await CancelableOperation.fromFuture(_cooldownFuture).cancel();
      notifyListeners();
    }
  }

  void restoreLastResult() {
    if (_store.previousExtendedResult == '') {
      return;
    }

    try {
      lastResult = ExtendedResult.fromJson(
          jsonDecode(_store.previousExtendedResult) as Map<String, dynamic>);
      resourceManager.resultManager
          .restoreResults(lastResult!.results.list, benchmarks);
      _doneRunning = true;
      return;
    } catch (e, trace) {
      print('unable to restore previous extended result: $e');
      print(trace);
      error = e;
      stackTrace = trace;
      _store.previousExtendedResult = '';
      resetCurrentResults();
      _doneRunning = null;
    }
  }
}
