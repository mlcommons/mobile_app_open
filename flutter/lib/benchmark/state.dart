import 'dart:io';
import 'dart:math';

import 'package:flutter/cupertino.dart';
import 'package:flutter/foundation.dart' show ChangeNotifier;
import 'package:flutter/material.dart' hide Icons;

import 'package:async/async.dart';
import 'package:collection/collection.dart';
import 'package:device_info/device_info.dart';
import 'package:ios_utsname_ext/extension.dart';
import 'package:package_info_plus/package_info_plus.dart';
import 'package:wakelock/wakelock.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/backend/bridge/isolate.dart';
import 'package:mlperfbench/backend/list.dart';
import 'package:mlperfbench/benchmark/benchmark_result.dart';
import 'package:mlperfbench/info.dart';
import 'package:mlperfbench/resources/config_manager.dart';
import 'package:mlperfbench/resources/resource_manager.dart';
import 'package:mlperfbench/resources/utils.dart';
import 'package:mlperfbench/store.dart';
import 'benchmark.dart';

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

  Future<void> _cooldownFuture = Future.value();

  bool _aborting = false;

  late BenchmarkList _middle;

  BenchmarkState._(this._store, this.backendBridge) {
    resourceManager = ResourceManager(notifyListeners);
    backendInfo = BackendInfo.findMatching();
  }

  static double _getSummaryMaxScore() => MAX_SCORE['SUMMARY_MAX_SCORE']!;

  List<String> listSelectedResources() {
    final result = <String>[];
    for (final job in _getBenchmarkJobs()) {
      result.add(job.benchmark.benchmarkSetting.src);
      result.add(job.dataset.path);
      result.add(job.dataset.groundtruthSrc);
    }
    final set = <String>{};
    result.retainWhere((x) => x.isNotEmpty && set.add(x));
    return result;
  }

  Future<String> validateExternalResourcesDirectory(
      String errorDescription) async {
    final resources = listSelectedResources();
    final missing = await resourceManager.validateResourcesExist(resources);
    if (missing.isEmpty) return '';

    return errorDescription +
        missing.mapIndexed((i, element) => '\n${i + 1}) $element').join();
  }

  Future<String> validateOfflineMode(String errorDescription) async {
    final resources = listSelectedResources();
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
    _store.clearBenchmarkList();
    await resetConfig();
    await loadResources();
  }

  Future<void> loadResources() async {
    _middle = BenchmarkList(await File(configManager.configPath).readAsString(),
        backendInfo.settings.benchmarkSetting);
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
    final submissionMode = _store.submissionMode;
    final testMode = _store.testMode;
    final jobs = <BenchmarkJob>[];

    for (final benchmark in _middle.benchmarks) {
      var storedConfig = _store
          .getBenchmarkList()
          .firstWhere((element) => element.id == benchmark.id);
      if (!storedConfig.active) continue;
      benchmark.benchmarkSetting.batchSize = storedConfig.batchSize;
      jobs.add(BenchmarkJob(
        benchmark: benchmark,
        accuracyMode: false,
        threadsNumber: storedConfig.threadsNumber,
        testMode: testMode,
      ));

      if (!submissionMode) continue;

      jobs.add(BenchmarkJob(
        benchmark: benchmark,
        accuracyMode: true,
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

    final cooldown = _store.cooldown;
    final cooldownPause = FAST_MODE
        ? Duration(seconds: 1)
        : Duration(minutes: _store.cooldownPause);
    final jobs = _getBenchmarkJobs();

    var jobsDoneCounter = 0;
    var wasAccuracy = true;
    final results = <RunResult>[];

    for (final job in jobs) {
      if (_aborting) break;

      if (cooldown && !job.accuracyMode && !wasAccuracy) {
        _cooling = true;
        notifyListeners();
        await (_cooldownFuture = Future.delayed(cooldownPause));
        _cooling = false;
        notifyListeners();
      }
      if (_aborting) break;
      wasAccuracy = job.accuracyMode;

      final resultFuture = job.run(resourceManager, backendBridge,
          backendInfo.settings.commonSetting, backendInfo.libPath);
      currentlyRunning = job.benchmark;
      runningProgress = '${(100 * (jobsDoneCounter / jobs.length)).round()}%';
      jobsDoneCounter++;
      notifyListeners();

      final result = await resultFuture;
      results.add(result);

      job.benchmark.backendDescription = result.backendDescription;
      if (job.accuracyMode) {
        job.benchmark.accuracy = result.accuracy;
      } else {
        job.benchmark.score = result.score;
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
