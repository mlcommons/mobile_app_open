import 'dart:async';
import 'dart:io';

import 'package:flutter/foundation.dart' show ChangeNotifier;

import 'package:wakelock/wakelock.dart';

import 'package:mlperfbench/backend/bridge/isolate.dart';
import 'package:mlperfbench/build_info.dart';
import 'package:mlperfbench/resources/config_manager.dart';
import 'package:mlperfbench/resources/resource_manager.dart';
import 'package:mlperfbench/resources/validation_helper.dart';
import 'package:mlperfbench/state/last_result_manager.dart';
import 'package:mlperfbench/state/task_list_manager.dart';
import 'package:mlperfbench/state/task_runner.dart';
import 'package:mlperfbench/store.dart';
import 'benchmark.dart';

enum BenchmarkStateEnum {
  downloading,
  resourceError,
  ready,
  running,
  aborting,
}

class BenchmarkState extends ChangeNotifier {
  final Store _store;
  final TaskListManager _taskListManager;
  final ResourceManager _resourceManager;
  final ConfigManager _configManager;
  final TaskRunner taskRunner;
  final LastResultManager _lastResultManager;

  Object? pendingError;

  BenchmarkStateEnum _state = BenchmarkStateEnum.downloading;
  BenchmarkStateEnum get state => _state;
  set state(BenchmarkStateEnum value) {
    _state = value;
    notifyListeners();
  }

  ValidationHelper get validator {
    return ValidationHelper(
      resourceManager: _resourceManager,
      middle: _taskListManager.taskList,
      selectedRunModes: taskRunner.selectedRunModes,
    );
  }

  BenchmarkState(
    this._store,
    this._taskListManager,
    this._resourceManager,
    this._configManager,
    this.taskRunner,
    this._lastResultManager,
  ) {
    _resourceManager.setUpdateNotifier(notifyListeners);
    _configManager.setUpdateNotifier(_onConfigChange);
    taskRunner.setUpdateNotifier(_handleTaskRunnerStateChange);
  }

  void _handleTaskRunnerStateChange() {
    switch (taskRunner.state) {
      case TaskRunnerState.ready:
        state = BenchmarkStateEnum.ready;
        break;
      case TaskRunnerState.running:
        state = BenchmarkStateEnum.running;
        break;
      case TaskRunnerState.aborting:
        state = BenchmarkStateEnum.aborting;
        break;
      default:
        throw 'unsupported state';
    }
  }

  void clearCache() async {
    await _tryRun(
      () async {
        await _resourceManager.cacheManager.deleteLoadedResources([], 0);
        await _configManager.setConfig(name: _store.chosenConfigurationName);
      },
      failedState: BenchmarkStateEnum.resourceError,
    );
  }

  // Start loading resources in background.
  // Return type 'void' is intended, this function must not be awaited.
  void startLoadingResources() async {
    await _tryRun(
      () async {
        state = BenchmarkStateEnum.downloading;
        await _loadResources();
        state = BenchmarkStateEnum.ready;
      },
      failedState: BenchmarkStateEnum.resourceError,
    );
  }

  Future<void> _loadResources() async {
    final newAppVersion =
        '${BuildInfoHelper.info.version}+${BuildInfoHelper.info.buildNumber}';
    var needToPurgeCache = _store.previousAppVersion != newAppVersion;
    _store.previousAppVersion = newAppVersion;

    await Wakelock.enable();
    print('start loading resources');
    await _resourceManager.handleResources(
      _taskListManager.taskList.listResources(
        modes: [taskRunner.perfMode, taskRunner.accuracyMode],
        skipInactive: false,
      ),
      needToPurgeCache,
    );
    print('finished loading resources');
    await Wakelock.disable();
  }

  static Future<BenchmarkState> create({
    required Store store,
    required BridgeIsolate bridgeIsolate,
    required TaskListManager taskListManager,
    required ResourceManager resourceManager,
    required ConfigManager configManager,
    required TaskRunner taskRunner,
    required LastResultManager lastResultManager,
  }) async {
    final result = BenchmarkState(
      store,
      taskListManager,
      resourceManager,
      configManager,
      taskRunner,
      lastResultManager,
    );
    await result._tryRun(
      () async => await result._configManager
          .setConfig(name: store.chosenConfigurationName),
      failedState: BenchmarkStateEnum.resourceError,
    );

    return result;
  }

  void _onConfigChange() {
    _store.chosenConfigurationName = _configManager.currentConfigName;

    _taskListManager.setAppConfig(_configManager.currentConfig);
    _taskListManager.taskList.restoreSelection(
        BenchmarkList.deserializeTaskSelection(_store.taskSelection));
    startLoadingResources();
  }

  void startBenchmark() async {
    await _tryRun(
      _runTasks,
      failedState: BenchmarkStateEnum.ready,
    );
  }

  Future<void> _runTasks() async {
    // we want last result to be null in case an exception is thrown
    _lastResultManager.value = null;

    if (state != BenchmarkStateEnum.ready) {
      throw 'app state != ready';
    }
    state = BenchmarkStateEnum.running;

    // disable screen sleep when benchmarks is running
    await Wakelock.enable();

    final startTime = DateTime.now();
    final logDirName = startTime.toIso8601String().replaceAll(':', '-');
    final currentLogDir = '${_resourceManager.resourceDir}/logs/$logDirName';

    try {
      final res = await taskRunner.runBenchmarks(
          _taskListManager.taskList, currentLogDir);
      if (res != null) {
        _lastResultManager.value = res;
        await _resourceManager.resultManager.addResult(res);
      }
      print('Benchmarks finished');

      state = BenchmarkStateEnum.ready;
    } finally {
      if (currentLogDir.isNotEmpty && !_store.keepLogs) {
        await Directory(currentLogDir).delete(recursive: true);
      }

      await Wakelock.disable();
    }
  }

  // this function catches all exceptions and saves them
  Future<void> _tryRun(
    Future Function() action, {
    required BenchmarkStateEnum failedState,
  }) async {
    try {
      await action();
    } catch (e, t) {
      print('failed action: $e');
      print(t);
      pendingError = e;
      state = failedState;
    }
  }
}
