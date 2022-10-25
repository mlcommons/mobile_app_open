import 'dart:async';
import 'dart:convert';
import 'dart:io';
import 'dart:math';

import 'package:flutter/cupertino.dart';
import 'package:flutter/foundation.dart' show ChangeNotifier;
import 'package:flutter/material.dart';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/firebase/manager.dart';
import 'package:wakelock/wakelock.dart';

import 'package:mlperfbench/backend/bridge/isolate.dart';
import 'package:mlperfbench/backend/list.dart';
import 'package:mlperfbench/build_info.dart';
import 'package:mlperfbench/resources/config_manager.dart';
import 'package:mlperfbench/resources/resource_manager.dart';
import 'package:mlperfbench/resources/validation_helper.dart';
import 'package:mlperfbench/state/task_runner.dart';
import 'package:mlperfbench/store.dart';
import 'benchmark.dart';

enum BenchmarkStateEnum {
  downloading,
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
  late final TaskRunner taskRunner;

  Object? error;
  StackTrace? stackTrace;

  bool taskConfigFailedToLoad = false;
  // null - downloading/waiting; false - running; true - done
  bool? _doneRunning;

  // Only if [state] == [BenchmarkStateEnum.downloading]
  String get downloadingProgress => resourceManager.progress;

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

  bool _aborting = false;

  late BenchmarkList _middle;

  ValidationHelper get validator {
    return ValidationHelper(
      resourceManager: resourceManager,
      middle: _middle,
      selectedRunModes: taskRunner.selectedRunModes,
    );
  }

  BenchmarkState._(this._store, this.backendBridge, this.firebaseManager) {
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

  Future<void> uploadLastResult() async {
    await firebaseManager!.restHelper.upload(lastResult!);
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
        '${BuildInfoHelper.info.version}+${BuildInfoHelper.info.buildNumber}';
    var needToPurgeCache = _store.previousAppVersion != newAppVersion;
    _store.previousAppVersion = newAppVersion;

    await Wakelock.enable();
    print('start loading resources');
    await resourceManager.handleResources(
      _middle.listResources(
        modes: [taskRunner.perfMode, taskRunner.accuracyMode],
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

    Map<String, bool>? taskSelection = {};
    if (_store.taskSelection.isNotEmpty) {
      try {
        final map = jsonDecode(_store.taskSelection) as Map<String, dynamic>;
        for (var kv in map.entries) {
          taskSelection[kv.key] = kv.value as bool;
        }
      } catch (e, t) {
        print('task selection parse fail: $e');
        print(t);
      }
    }

    _middle = BenchmarkList(
      appConfig: configManager.decodedConfig,
      backendConfig: backendInfo.settings.benchmarkSetting,
      taskSelection: taskSelection,
    );
    restoreLastResult();
  }

  Future<void> saveTaskSelection() async {
    _store.taskSelection = jsonEncode(_middle.selection);
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

    final startTime = DateTime.now();
    final logDirName = startTime.toIso8601String().replaceAll(':', '-');
    final currentLogDir =
        '${resourceManager.applicationDirectory}/logs/$logDirName';

    try {
      resetCurrentResults();
      lastResult = await taskRunner.runBenchmarks(_middle, currentLogDir);

      print('Benchmarks finished');

      _store.previousExtendedResult =
          const JsonEncoder().convert(lastResult!.toJson());
      await resourceManager.resultManager.saveResult(lastResult!);

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

  void resetCurrentResults() {
    for (var b in _middle.benchmarks) {
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
          .restoreResults(lastResult!.results, benchmarks);
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
