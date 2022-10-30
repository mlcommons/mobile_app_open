import 'dart:async';
import 'dart:io';

import 'package:flutter/foundation.dart' show ChangeNotifier;

import 'package:wakelock/wakelock.dart';

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
  final Store store;
  final TaskListManager taskListManager;
  final ResourceManager resourceManager;
  final ConfigManager configManager;
  final TaskRunner taskRunner;
  final LastResultManager lastResultManager;

  AppStateHelper({
    required this.store,
    required this.taskListManager,
    required this.resourceManager,
    required this.configManager,
    required this.taskRunner,
    required this.lastResultManager,
  });

  Future<void> _loadResources() async {
    // TODO refactor this method

    final newAppVersion =
        '${BuildInfoHelper.info.version}+${BuildInfoHelper.info.buildNumber}';
    var needToPurgeCache = store.previousAppVersion != newAppVersion;
    store.previousAppVersion = newAppVersion;

    await Wakelock.enable();
    print('start loading resources');
    await resourceManager.handleResources(
      taskListManager.taskList.listResources(
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
    lastResultManager.value = null;

    // disable screen sleep when benchmarks is running
    await Wakelock.enable();

    final startTime = DateTime.now();
    final logDirName = startTime.toIso8601String().replaceAll(':', '-');
    final currentLogDir = '${resourceManager.resourceDir}/logs/$logDirName';

    try {
      final res = await taskRunner.runBenchmarks(
          taskListManager.taskList, currentLogDir);
      if (res != null) {
        lastResultManager.value = res;
        await resourceManager.resultManager.addResult(res);
      }
      print('Benchmarks finished');
    } finally {
      if (currentLogDir.isNotEmpty && !store.keepLogs) {
        await Directory(currentLogDir).delete(recursive: true);
      }

      await Wakelock.disable();
    }
  }
}

// All methods in AppState should have the following properties:
//   - Public methods: return type `void`, public methods must not be awaited
//   - Public methods: exceptions are never thrown
//   - All methods: notifyListeners() call outside of `state` setter is forbidden
//   - All methods: if notifyListeners() is needed when `state` field doesn't change,
//        the class that contains the data that requires UI update should implement ChangeNotifier instead
//   - All methods: complex logic should be moved to other classes, methods in AppState should only manage `state`
//
// Any methods or getters that don't satisfy these requirements should be subject to refactoring.
//
// Suggested code structure that satisfies this:
// void method() <possibly async> {
// _tryRun(() {
//    <change state field>;
//    <invoke AppStateHelper methods>;
//    <change state field>;
// }, failedState: <something>);
// }
class AppState extends ChangeNotifier {
  final AppStateHelper _helper;
  Object? pendingError;

  AppStateEnum _state = AppStateEnum.downloading;
  AppStateEnum get state => _state;
  set state(AppStateEnum value) {
    _state = value;
    notifyListeners();
  }

  // TODO move this out of the AppState class
  ValidationHelper get validator {
    return ValidationHelper(
      resourceManager: _helper.resourceManager,
      middle: _helper.taskListManager.taskList,
      selectedRunModes: _helper.taskRunner.selectedRunModes,
    );
  }

  // TODO provide task runner through context instead of this getter
  TaskRunner get taskRunner => _helper.taskRunner;

  AppState(this._helper) {
    bindCallbacks();
  }

  static Future<AppState> create({
    required AppStateHelper appStateHelper,
  }) async {
    final result = AppState(appStateHelper);
    await result._tryRun(
      () async => await result._helper.configManager
          .setConfig(name: appStateHelper.store.chosenConfigurationName),
      failedState: AppStateEnum.resourceError,
    );

    return result;
  }

  void bindCallbacks() {
    _helper.resourceManager.setUpdateNotifier(notifyListeners);
    _helper.configManager.setUpdateNotifier(_onConfigChange);
    _helper.taskRunner.setUpdateNotifier(_handleTaskRunnerStateChange);
  }

  void _onConfigChange() {
    _helper.store.chosenConfigurationName =
        _helper.configManager.currentConfigName;

    _helper.taskListManager.setAppConfig(_helper.configManager.currentConfig);
    _helper.taskListManager.taskList.restoreSelection(
        BenchmarkList.deserializeTaskSelection(_helper.store.taskSelection));
    startLoadingResources();
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
        await _helper.resourceManager.cacheManager.deleteLoadedResources([], 0);
        await _helper.configManager
            .setConfig(name: _helper.store.chosenConfigurationName);
      },
      failedState: AppStateEnum.resourceError,
    );
  }

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

  // catches all exceptions and saves them to show in UI
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
