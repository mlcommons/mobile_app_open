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
import '../benchmark/benchmark.dart';

enum AppStateEnum {
  downloading,
  resourceError,
  ready,
  running,
  aborting,
}

class AppStateHelper {
  final Store _store;
  final TaskListManager _taskListManager;
  final ResourceManager _resourceManager;
  final ConfigManager _configManager;
  final TaskRunner taskRunner;
  final LastResultManager _lastResultManager;

  AppStateHelper(
    this._store,
    this._taskListManager,
    this._resourceManager,
    this._configManager,
    this.taskRunner,
    this._lastResultManager,
  );

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

  Future<void> _runTasks() async {
    // we want last result to be null in case an exception is thrown
    _lastResultManager.value = null;

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
    } finally {
      if (currentLogDir.isNotEmpty && !_store.keepLogs) {
        await Directory(currentLogDir).delete(recursive: true);
      }

      await Wakelock.disable();
    }
  }
}

class AppState extends ChangeNotifier {
  final AppStateHelper _helper;
  Object? pendingError;

  AppStateEnum _state = AppStateEnum.downloading;
  AppStateEnum get state => _state;
  set state(AppStateEnum value) {
    _state = value;
    notifyListeners();
  }

  // TODO move this out of AppState class
  ValidationHelper get validator {
    return ValidationHelper(
      resourceManager: _helper._resourceManager,
      middle: _helper._taskListManager.taskList,
      selectedRunModes: _helper.taskRunner.selectedRunModes,
    );
  }

  // TODO access task runner through context instead of this getter
  TaskRunner get taskRunner => _helper.taskRunner;

  AppState(this._helper) {
    bindHelper();
  }

  void bindHelper() {
    _helper._resourceManager.setUpdateNotifier(notifyListeners);
    _helper._configManager.setUpdateNotifier(_onConfigChange);
    _helper.taskRunner.setUpdateNotifier(_handleTaskRunnerStateChange);
  }

  void _handleTaskRunnerStateChange() {
    switch (_helper.taskRunner.state) {
      case TaskRunnerState.ready:
        state = AppStateEnum.ready;
        break;
      case TaskRunnerState.running:
        state = AppStateEnum.running;
        break;
      case TaskRunnerState.aborting:
        state = AppStateEnum.aborting;
        break;
      default:
        throw 'unsupported state';
    }
  }

  void clearCache() async {
    await _tryRun(
      () async {
        await _helper._resourceManager.cacheManager
            .deleteLoadedResources([], 0);
        await _helper._configManager
            .setConfig(name: _helper._store.chosenConfigurationName);
      },
      failedState: AppStateEnum.resourceError,
    );
  }

  // Start loading resources in background.
  // Return type 'void' is intended, this function must not be awaited.
  void startLoadingResources() async {
    await _tryRun(
      () async {
        state = AppStateEnum.downloading;
        await _helper._loadResources();
        state = AppStateEnum.ready;
      },
      failedState: AppStateEnum.resourceError,
    );
  }

  static Future<AppState> create({
    required Store store,
    required BridgeIsolate bridgeIsolate,
    required TaskListManager taskListManager,
    required ResourceManager resourceManager,
    required ConfigManager configManager,
    required TaskRunner taskRunner,
    required LastResultManager lastResultManager,
  }) async {
    final appStateHelper = AppStateHelper(
      store,
      taskListManager,
      resourceManager,
      configManager,
      taskRunner,
      lastResultManager,
    );
    final result = AppState(appStateHelper);
    await result._tryRun(
      () async => await result._helper._configManager
          .setConfig(name: store.chosenConfigurationName),
      failedState: AppStateEnum.resourceError,
    );

    return result;
  }

  void _onConfigChange() {
    _helper._store.chosenConfigurationName =
        _helper._configManager.currentConfigName;

    _helper._taskListManager.setAppConfig(_helper._configManager.currentConfig);
    _helper._taskListManager.taskList.restoreSelection(
        BenchmarkList.deserializeTaskSelection(_helper._store.taskSelection));
    startLoadingResources();
  }

  void startBenchmark() async {
    await _tryRun(
      () async {
        if (state != AppStateEnum.ready) {
          throw 'app state != ready';
        }
        state = AppStateEnum.running;
        await _helper._runTasks();
        state = AppStateEnum.ready;
      },
      failedState: AppStateEnum.ready,
    );
  }

  // this function catches all exceptions and saves them
  Future<void> _tryRun(
    Future Function() action, {
    required AppStateEnum failedState,
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
