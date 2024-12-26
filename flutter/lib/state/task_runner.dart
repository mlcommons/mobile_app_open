import 'dart:io';
import 'dart:math';

import 'package:async/async.dart';
import 'package:uuid/uuid.dart';
import 'package:worker_manager/worker_manager.dart';

import 'package:mlperfbench/backend/bridge/isolate.dart';
import 'package:mlperfbench/backend/bridge/run_result.dart';
import 'package:mlperfbench/backend/bridge/run_settings.dart';
import 'package:mlperfbench/backend/list.dart';
import 'package:mlperfbench/backend/loadgen_info.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/info.dart';
import 'package:mlperfbench/benchmark/run_info.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';
import 'package:mlperfbench/build_info.dart';
import 'package:mlperfbench/data/extended_result.dart';
import 'package:mlperfbench/data/meta_info.dart';
import 'package:mlperfbench/data/results/benchmark_result.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'package:mlperfbench/resources/export_result_helper.dart';
import 'package:mlperfbench/resources/resource_manager.dart';
import 'package:mlperfbench/store.dart';

class ProgressInfo {
  bool cooldown = false;
  bool accuracy = false;
  BenchmarkInfo? currentBenchmark;
  List<BenchmarkInfo> completedBenchmarks = [];
  List<BenchmarkInfo> activeBenchmarks = [];
  late BenchmarkRunModeEnum runMode;
  int totalStages = 0;
  int currentStage = 0;
  double cooldownDuration = 0;

  double get stageProgress => calculateStageProgress?.call() ?? 0.0;

  double Function()? calculateStageProgress;
}

class TaskRunner {
  final Store store;
  final void Function() notifyListeners;
  final ResourceManager resourceManager;
  final BridgeIsolate backendBridge;
  final BackendInfo backendInfo;

  late ProgressInfo progressInfo;
  bool aborting = false;
  CancelableOperation? _cooldownOperation;

  TaskRunner({
    required this.store,
    required this.notifyListeners,
    required this.resourceManager,
    required this.backendBridge,
    required this.backendInfo,
  });

  BenchmarkRunMode get perfMode =>
      store.selectedBenchmarkRunMode.performanceRunMode;

  BenchmarkRunMode get accuracyMode =>
      store.selectedBenchmarkRunMode.accuracyRunMode;

  List<BenchmarkRunMode> get selectedRunModes =>
      store.selectedBenchmarkRunMode.selectedRunModes;

  Future<void> abortBenchmarks() async {
    aborting = true;
    await _cooldownOperation?.cancel();
    notifyListeners();
  }

  Future<ExtendedResult?> runBenchmarks(
      BenchmarkStore benchmarkStore, String currentLogDir) async {
    progressInfo = ProgressInfo();
    final cooldown = store.cooldown;
    final cooldownDuration = Duration(minutes: store.cooldownDuration);

    final activeBenchmarks =
        benchmarkStore.benchmarks.where((element) => element.isActive);

    final resultHelpers = <ResultHelper>[];
    for (final benchmark in activeBenchmarks) {
      progressInfo.activeBenchmarks.add(benchmark.info);
      final resultHelper = ResultHelper(
          benchmark: benchmark,
          backendInfo: backendInfo,
          performanceMode: perfMode,
          accuracyMode: accuracyMode);
      resultHelpers.add(resultHelper);
    }

    print('Starting in run mode: ${store.selectedBenchmarkRunMode}');
    progressInfo.runMode = store.selectedBenchmarkRunMode;
    progressInfo.totalStages =
        selectedRunModes.length * activeBenchmarks.length;

    progressInfo.currentStage = 0;
    progressInfo.cooldownDuration = cooldownDuration.inSeconds.toDouble();

    var first = true;

    // run all benchmarks in performance mode first
    progressInfo.completedBenchmarks.clear();
    for (final benchmark in activeBenchmarks) {
      if (aborting) break;
      if (!store.selectedBenchmarkRunMode.doPerformanceRun) break;
      // we only do cooldown before performance benchmarks
      if (cooldown && !first && cooldownDuration.inMilliseconds > 0) {
        progressInfo.cooldown = true;
        final timer = Stopwatch()..start();
        progressInfo.calculateStageProgress = () {
          return timer.elapsed.inSeconds / progressInfo.cooldownDuration;
        };
        notifyListeners();
        final delayedFuture = Future.delayed(cooldownDuration);
        _cooldownOperation = CancelableOperation.fromFuture(delayedFuture);
        await _cooldownOperation?.valueOrCancellation();
        progressInfo.cooldown = false;
        timer.stop();
      }
      first = false;
      if (aborting) break;
      final resultHelper =
          resultHelpers.firstWhere((e) => e.benchmark == benchmark);
      await runBenchmark(resultHelper, perfMode, currentLogDir);
    }

    // then in accuracy mode
    progressInfo.completedBenchmarks.clear();
    for (final benchmark in activeBenchmarks) {
      if (aborting) break;
      if (!store.selectedBenchmarkRunMode.doAccuracyRun) break;
      final resultHelper =
          resultHelpers.firstWhere((e) => e.benchmark == benchmark);
      await runBenchmark(resultHelper, accuracyMode, currentLogDir);
    }

    if (aborting) {
      aborting = false;
      return null;
    }

    final exportResults = <BenchmarkExportResult>[];
    for (final resultHelper in resultHelpers) {
      exportResults.add(resultHelper.getBenchmarkExportResult());
    }

    final creationDate = DateTime.now();
    return ExtendedResult(
      meta: ResultMetaInfo(creationDate: creationDate, uuid: const Uuid().v4()),
      environmentInfo: DeviceInfo.instance.envInfo,
      results: exportResults,
      buildInfo: BuildInfoHelper.info,
    );
  }

  Future<void> runBenchmark(ResultHelper resultHelper, BenchmarkRunMode mode,
      String currentLogDir) async {
    final benchmark = resultHelper.benchmark;
    progressInfo.currentBenchmark = benchmark.info;
    progressInfo.currentStage++;
    if (mode.loadgenMode == LoadgenModeEnum.performanceOnly) {
      final perfTimer = Stopwatch()..start();
      progressInfo.accuracy = false;
      double taskMinDuration =
          mode.chooseRunConfig(benchmark.taskConfig).minDuration;
      int taskMinQueryCount =
          mode.chooseRunConfig(benchmark.taskConfig).minQueryCount;
      progressInfo.calculateStageProgress = () {
        // UI updates once per second so using 1 second as lower bound should not affect it.
        final minDuration = max(taskMinDuration, 1);
        final timeProgress = perfTimer.elapsedMilliseconds /
            Duration.millisecondsPerSecond /
            minDuration;

        final minQueries = max(taskMinQueryCount, 1);
        final queryCounter = backendBridge.getQueryCounter();
        final queryProgress =
            queryCounter < 0 ? 1.0 : queryCounter / minQueries;
        return min(timeProgress, queryProgress);
      };
      notifyListeners();

      final runHelper = _NativeRunHelper(
        enableArtificialLoad: store.artificialCPULoadEnabled,
        backendBridge: backendBridge,
        benchmark: benchmark,
        runMode: perfMode,
        logParentDir: currentLogDir,
      );
      await runHelper.initRunSettings(
        resourceManager: resourceManager,
        commonSettings: backendInfo.settings.commonSetting,
        backendLibName: backendInfo.libName,
      );
      final performanceRunInfo = await runHelper.run();
      perfTimer.stop();
      performanceRunInfo.loadgenInfo!;

      final performanceResult = performanceRunInfo.result;
      benchmark.performanceModeResult = BenchmarkResult(
        throughput: performanceRunInfo.throughput,
        accuracy: performanceResult.accuracy1,
        accuracy2: performanceResult.accuracy2,
        backendName: performanceResult.backendName,
        acceleratorName: performanceResult.acceleratorName,
        delegateName: benchmark.benchmarkSettings.delegateSelected,
        batchSize: benchmark.selectedDelegate.batchSize,
        loadgenInfo: performanceRunInfo.loadgenInfo!,
      );
      resultHelper.performanceRunInfo = performanceRunInfo;
    } else if (mode.loadgenMode == LoadgenModeEnum.accuracyOnly) {
      progressInfo.accuracy = true;
      progressInfo.calculateStageProgress = () {
        final queryCounter = backendBridge.getQueryCounter();
        final queryProgress = queryCounter < 0
            ? 1.0
            : queryCounter / backendBridge.getDatasetSize();
        return queryProgress;
      };
      notifyListeners();
      final runHelper = _NativeRunHelper(
        enableArtificialLoad: store.artificialCPULoadEnabled,
        backendBridge: backendBridge,
        benchmark: benchmark,
        runMode: accuracyMode,
        logParentDir: currentLogDir,
      );
      await runHelper.initRunSettings(
        resourceManager: resourceManager,
        commonSettings: backendInfo.settings.commonSetting,
        backendLibName: backendInfo.libName,
      );
      final accuracyRunInfo = await runHelper.run();
      resultHelper.accuracyRunInfo = accuracyRunInfo;
      final accuracyResult = accuracyRunInfo.result;
      benchmark.accuracyModeResult = BenchmarkResult(
        // loadgen doesn't calculate latency for accuracy mode benchmarks
        // so throughput is infinity which is not a valid JSON numeric value
        throughput: null,
        accuracy: accuracyResult.accuracy1,
        accuracy2: accuracyResult.accuracy2,
        backendName: accuracyResult.backendName,
        acceleratorName: accuracyResult.acceleratorName,
        delegateName: benchmark.benchmarkSettings.delegateSelected,
        batchSize: benchmark.selectedDelegate.batchSize,
        loadgenInfo: accuracyRunInfo.loadgenInfo,
      );
    } else {
      print('Unknown BenchmarkRunMode: $mode');
      throw 'Unknown BenchmarkRunMode: $mode';
    }
    progressInfo.completedBenchmarks.add(benchmark.info);
    progressInfo.currentBenchmark = null;
  }
}

class _NativeRunHelper {
  final bool enableArtificialLoad;
  final BridgeIsolate backendBridge;

  final Benchmark benchmark;
  final BenchmarkRunMode runMode;
  final String logDir;
  late final RunSettings runSettings;

  _NativeRunHelper({
    required this.enableArtificialLoad,
    required this.backendBridge,
    required this.benchmark,
    required this.runMode,
    required String logParentDir,
  }) : logDir = '$logParentDir/${benchmark.id}-${runMode.readable}';

  Future<void> initRunSettings({
    required ResourceManager resourceManager,
    required List<pb.CommonSetting> commonSettings,
    required String backendLibName,
  }) async {
    runSettings = await benchmark.createRunSettings(
      runMode: runMode,
      resourceManager: resourceManager,
      commonSettings: commonSettings,
      backendLibName: backendLibName,
      logDir: logDir,
    );
  }

  Future<RunInfo> run() async {
    await Directory(logDir).create(recursive: true);

    if (enableArtificialLoad) {
      print('Apply the artificial CPU load for ${benchmark.taskConfig.id}');
      final _ = workerManager.execute(
        () async {
          const value = 999999999999999.0;
          var newValue = value;
          while (true) {
            newValue = newValue * 0.999999999999999;
          }
        },
        priority: WorkPriority.immediately,
      );
    }

    try {
      return await _invokeNativeRun();
    } finally {
      await workerManager.dispose();
    }
  }

  Future<RunInfo> _invokeNativeRun() async {
    final logPrefix = '[${benchmark.id}: $runMode mode]';

    print('$logPrefix starting...');
    final stopwatch = Stopwatch()..start();
    final nativeResult = await backendBridge.run(runSettings);
    final elapsed = stopwatch.elapsed;
    print('$logPrefix elapsed: $elapsed');
    print('$logPrefix result: $nativeResult');

    if (!_checkAccuracy(nativeResult)) {
      throw '$logPrefix accuracy is invalid (backend may be corrupted)';
    }

    final runInfo = await _makeRunInfo(nativeResult);

    final throughput = runInfo.throughput;
    if (throughput != null) {
      print('$logPrefix throughput: $throughput');
    }
    final loadgenInfo = runInfo.loadgenInfo;
    if (loadgenInfo != null) {
      print('$logPrefix isMinDurationMet: ${loadgenInfo.isMinDurationMet}');
      print('$logPrefix isMinQueryMet: ${loadgenInfo.isMinQueryMet}');
      print('$logPrefix isEarlyStoppingMet: ${loadgenInfo.isEarlyStoppingMet}');
      print('$logPrefix isResultValid: ${loadgenInfo.isResultValid}');
    }

    return runInfo;
  }

  Future<RunInfo> _makeRunInfo(NativeRunResult nativeResult) async {
    final loadgenInfo = await extractLoadgenInfo();
    final throughputValue = _calculateThroughput(nativeResult, loadgenInfo);
    Throughput? throughput;
    if (throughputValue != null) {
      throughput = Throughput(value: throughputValue);
    }
    return RunInfo(
      settings: runSettings,
      result: nativeResult,
      loadgenInfo: loadgenInfo,
      throughput: throughput,
    );
  }

  bool _checkAccuracy(NativeRunResult result) {
    return (result.accuracy1?.isInBounds() ?? true) &&
        (result.accuracy2?.isInBounds() ?? true);
  }

  double? _calculateThroughput(
    NativeRunResult result,
    LoadgenInfo? loadgenInfo,
  ) {
    if (loadgenInfo == null) {
      return null;
    }
    if (benchmark.info.isOffline) {
      return result.numSamples / loadgenInfo.latency90;
    } else {
      return 1.0 / loadgenInfo.latency90;
    }
  }

  Future<LoadgenInfo?> extractLoadgenInfo() async {
    const logFileName = 'mlperf_log_detail.txt';
    return await LoadgenInfo.fromFile(
      filepath: '$logDir/$logFileName',
    );
  }
}
