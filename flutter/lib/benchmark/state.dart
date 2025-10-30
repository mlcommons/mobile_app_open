import 'dart:async';
import 'dart:convert';
import 'dart:io';
import 'dart:math';

import 'package:flutter/cupertino.dart';
import 'package:flutter/foundation.dart' show ChangeNotifier;
import 'package:flutter/material.dart';

import 'package:wakelock/wakelock.dart';

import 'package:mlperfbench/backend/bridge/isolate.dart';
import 'package:mlperfbench/backend/list.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/board_decoder.dart';
import 'package:mlperfbench/build_info.dart';
import 'package:mlperfbench/data/extended_result.dart';
import 'package:mlperfbench/resources/config_manager.dart';
import 'package:mlperfbench/resources/resource_manager.dart';
import 'package:mlperfbench/resources/validation_helper.dart';
import 'package:mlperfbench/state/task_runner.dart';
import 'package:mlperfbench/store.dart';

enum BenchmarkStateEnum {
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
  late final TaskRunner taskRunner;
  late final BoardDecoder boardDecoder;

  Object? error;
  StackTrace? stackTrace;

  bool taskConfigFailedToLoad = false;

  // null - downloading/waiting; false - running; true - done
  bool? _doneRunning;

  // Only if [state] == [BenchmarkStateEnum.downloading]
  String get loadingPath => resourceManager.loadingPath;

  double get loadingProgress => resourceManager.loadingProgress;

  ExtendedResult? lastResult;

  num get result {
    final benchmarksCount = allBenchmarks
        .where((benchmark) => benchmark.performanceModeResult != null)
        .length;

    if (benchmarksCount == 0) return 0;

    final summaryThroughput = pow(
        allBenchmarks.fold<double>(1, (prev, i) {
          return prev * (i.performanceModeResult?.throughput?.value ?? 1.0);
        }),
        1.0 / benchmarksCount);

    final maxSummaryThroughput = pow(
        allBenchmarks.fold<double>(1, (prev, i) {
          return prev * (i.info.maxThroughput);
        }),
        1.0 / benchmarksCount);

    return summaryThroughput / maxSummaryThroughput;
  }

  List<Benchmark> get allBenchmarks => _benchmarkStore.allBenchmarks;

  List<Benchmark> get activeBenchmarks => _benchmarkStore.activeBenchmarks;

  late BenchmarkStore _benchmarkStore;

  ValidationHelper get validator {
    return ValidationHelper(
      resourceManager: resourceManager,
      benchmarkStore: _benchmarkStore,
      selectedRunModes: taskRunner.selectedRunModes,
    );
  }

  BenchmarkState._(this._store, this.backendBridge) {
    resourceManager = ResourceManager(notifyListeners, _store);
    backendInfo = BackendInfoHelper().findMatching();
    taskRunner = TaskRunner(
      store: _store,
      notifyListeners: notifyListeners,
      resourceManager: resourceManager,
      backendBridge: backendBridge,
      backendInfo: backendInfo,
    );
  }

  Future<void> clearCache() async {
    await resourceManager.cacheManager.deleteLoadedResources([], 0);
    notifyListeners();
    try {
      await setTaskConfig(name: _store.chosenConfigurationName);
      deferredLoadResources();
    } catch (e, trace) {
      print("Can't load resources: $e");
      print(trace);
      error = e;
      stackTrace = trace;
      taskConfigFailedToLoad = true;
      notifyListeners();
    }
  }

  // Start loading resources in background.
  // Return type 'void' is intended, this function must not be awaited.
  // ignore: avoid_void_async
  void deferredLoadResources() async {
    try {
      await loadResources(downloadMissing: false);
    } catch (e, trace) {
      print("Can't load resources: $e");
      print(trace);
      error = e;
      stackTrace = trace;
      taskConfigFailedToLoad = true;
      notifyListeners();
    }
  }

  Future<void> loadResources(
      {required bool downloadMissing,
      List<Benchmark> benchmarks = const []}) async {
    final newAppVersion =
        '${BuildInfoHelper.info.version}+${BuildInfoHelper.info.buildNumber}';
    var needToPurgeCache = _store.previousAppVersion != newAppVersion;
    _store.previousAppVersion = newAppVersion;

    final selectedBenchmarks = benchmarks.isEmpty ? allBenchmarks : benchmarks;
    await Wakelock.enable();
    final selectedResources = _benchmarkStore.listResources(
      modes: [taskRunner.perfMode, taskRunner.accuracyMode],
      benchmarks: selectedBenchmarks,
    );
    final allResources = _benchmarkStore.listResources(
      modes: [taskRunner.perfMode, taskRunner.accuracyMode],
      benchmarks: allBenchmarks,
    );
    try {
      final selectedBenchmarkIds = selectedBenchmarks
          .map((e) => e.benchmarkSettings.benchmarkId)
          .join(', ');
      print('Start loading resources with downloadMissing=$downloadMissing '
          'for $selectedBenchmarkIds');
      await resourceManager.handleResources(
        resources: selectedResources,
        purgeOldCache: needToPurgeCache,
        downloadMissing: downloadMissing,
      );
      print('Finished loading resources with downloadMissing=$downloadMissing');
      // We still need to load all resources after download selected resources.
      await resourceManager.handleResources(
        resources: allResources,
        purgeOldCache: false,
        downloadMissing: false,
      );
      error = null;
      stackTrace = null;
      taskConfigFailedToLoad = false;
    } catch (e, s) {
      print('Could not load resources due to error: $e');
      error = e;
      stackTrace = s;
    }
    await Wakelock.disable();
  }

  static Future<BenchmarkState> create(Store store) async {
    final state = BenchmarkState._(store, await BridgeIsolate.create());
    await state.resourceManager.initSystemPaths();
    state.configManager = ConfigManager(
        state.resourceManager.applicationDirectory, state.resourceManager);
    try {
      await state.setTaskConfig(name: store.chosenConfigurationName);
      state.deferredLoadResources();
    } catch (e, trace) {
      print("Can't load resources: $e");
      print(trace);
      state.error = e;
      state.stackTrace = trace;
      state.taskConfigFailedToLoad = true;
    }

    state.boardDecoder = BoardDecoder();
    await state.boardDecoder.init();

    return state;
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

    Map<String, bool>? taskSelection = {};
    if (_store.taskSelection.isNotEmpty) {
      try {
        final map = jsonDecode(_store.taskSelection) as Map<String, dynamic>;
        for (var kv in map.entries) {
          taskSelection[kv.key] = kv.value as bool;
        }
      } catch (e, t) {
        print('Task selection parse fail: $e');
        print(t);
      }
    }

    _benchmarkStore = BenchmarkStore(
      appConfig: configManager.decodedConfig,
      backendConfig: backendInfo.settings.benchmarkSetting,
      taskSelection: taskSelection,
    );
    restoreLastResult();
  }

  void saveTaskSelection() {
    _store.taskSelection = jsonEncode(_benchmarkStore.selection);
  }

  BenchmarkStateEnum get state {
    switch (_doneRunning) {
      case null:
        return BenchmarkStateEnum.waiting;
      case false:
        return taskRunner.aborting
            ? BenchmarkStateEnum.aborting
            : BenchmarkStateEnum.running;
      case true:
        return BenchmarkStateEnum.done;
    }
  }

  Future<void> resetBenchmarkState() async {
    _doneRunning = null;
    resetCurrentResults();
    notifyListeners();
  }

  Future<void> runBenchmarks() async {
    assert(resourceManager.done, 'Resource manager is not done.');
    assert(_doneRunning != false, '_doneRunning is false');
    _store.previousExtendedResult = '';
    _doneRunning = false;

    // disable screen sleep when benchmarks is running
    await Wakelock.enable();

    final startTime = DateTime.now();
    final logDirName = startTime.toIso8601String().replaceAll(':', '-');
    final currentLogDir =
        '${resourceManager.applicationDirectory}/logs/$logDirName';

    try {
      resetCurrentResults();
      lastResult =
          await taskRunner.runBenchmarks(_benchmarkStore, currentLogDir);

      if (lastResult == null) {
        print('Benchmark aborted');
      } else {
        print('Benchmarks finished');

        _store.previousExtendedResult =
            const JsonEncoder().convert(lastResult!.toJson());
        await resourceManager.resultManager.saveResult(lastResult!);
      }

      _doneRunning = taskRunner.aborting ? null : true;
    } catch (e) {
      _doneRunning = null;
      print('Error: $e');
      rethrow;
    } finally {
      if (currentLogDir.isNotEmpty && !_store.keepLogs) {
        await Directory(currentLogDir).delete(recursive: true);
      }

      taskRunner.aborting = false;

      notifyListeners();

      await Wakelock.disable();
    }
  }

  void resetCurrentResults() {
    for (var b in _benchmarkStore.allBenchmarks) {
      b.accuracyModeResult = null;
      b.performanceModeResult = null;
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
          .restoreResults(lastResult!.results, allBenchmarks);
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

  void benchmarkSetActive(Benchmark benchmark, bool isActive) {
    benchmark.isActive = isActive;
    saveTaskSelection();
    notifyListeners();
  }

  void benchmarkSetDelegate(Benchmark benchmark, String delegate) {
    benchmark.benchmarkSettings.delegateSelected = delegate;
    notifyListeners();
  }

  @override
  void notifyListeners() {
    super.notifyListeners();
  }
}
