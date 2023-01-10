import 'dart:io';
import 'dart:math';

import 'package:async/async.dart';
import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/meta_info.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';
import 'package:uuid/uuid.dart';
import 'package:worker_manager/worker_manager.dart';

import 'package:mlperfbench/app_constants.dart';
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
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'package:mlperfbench/resources/export_result_helper.dart';
import 'package:mlperfbench/resources/resource_manager.dart';
import 'package:mlperfbench/store.dart';

class ProgressInfo {
  bool cooldown = false;
  bool accuracy = false;
  BenchmarkInfo? info;
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

  ProgressInfo progressInfo = ProgressInfo();
  bool aborting = false;
  Future<void> _cooldownFuture = Future.value();

  TaskRunner({
    required this.store,
    required this.notifyListeners,
    required this.resourceManager,
    required this.backendBridge,
    required this.backendInfo,
  });

  BenchmarkRunMode get perfMode => store.testMode
      ? BenchmarkRunMode.performanceTest
      : BenchmarkRunMode.performance;

  BenchmarkRunMode get accuracyMode => store.testMode
      ? BenchmarkRunMode.accuracyTest
      : BenchmarkRunMode.accuracy;

  List<BenchmarkRunMode> get selectedRunModes {
    final result = <BenchmarkRunMode>[];
    result.add(perfMode);
    if (store.submissionMode) {
      result.add(accuracyMode);
    }
    return result;
  }

  Future<void> abortBenchmarks() async {
    aborting = true;
    await CancelableOperation.fromFuture(_cooldownFuture).cancel();
    notifyListeners();
  }

  Future<ExtendedResult?> runBenchmarks(
      BenchmarkList benchmarkList, String currentLogDir) async {
    final cooldown = store.cooldown;
    late final Duration cooldownDuration;
    if (store.testMode) {
      cooldownDuration = Duration(seconds: store.testCooldown);
    } else if (isFastMode) {
      cooldownDuration = const Duration(seconds: 1);
    } else {
      cooldownDuration = Duration(minutes: store.cooldownDuration);
    }

    final activeBenchmarks =
        benchmarkList.benchmarks.where((element) => element.isActive);

    final resultHelpers = <ResultHelper>[];
    for (final benchmark in activeBenchmarks) {
      final resultHelper = ResultHelper(
          benchmark: benchmark,
          backendInfo: backendInfo,
          performanceMode: perfMode,
          accuracyMode: accuracyMode);
      resultHelpers.add(resultHelper);
    }

    progressInfo.totalStages = activeBenchmarks.length;
    if (store.submissionMode) {
      progressInfo.totalStages += activeBenchmarks.length;
    }
    progressInfo.currentStage = 0;
    progressInfo.cooldownDuration = cooldownDuration.inSeconds.toDouble();

    var first = true;

    // run all benchmarks in performance mode first
    for (final benchmark in activeBenchmarks) {
      if (aborting) break;
      // we only do cooldown before performance benchmarks
      if (cooldown && !first && cooldownDuration.inMilliseconds > 0) {
        progressInfo.cooldown = true;
        final timer = Stopwatch()..start();
        progressInfo.calculateStageProgress = () {
          return timer.elapsed.inSeconds / progressInfo.cooldownDuration;
        };
        notifyListeners();
        await (_cooldownFuture = Future.delayed(cooldownDuration));
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
    for (final benchmark in activeBenchmarks) {
      if (aborting) break;
      if (!store.submissionMode) break;
      final resultHelper =
          resultHelpers.firstWhere((e) => e.benchmark == benchmark);
      await runBenchmark(resultHelper, accuracyMode, currentLogDir);
    }

    final exportResults = <BenchmarkExportResult>[];
    for (final resultHelper in resultHelpers) {
      exportResults.add(resultHelper.getBenchmarkExportResult());
    }

    if (aborting) {
      aborting = false;
      return null;
    }
    return ExtendedResult(
      meta:
          ResultMetaInfo(creationDate: DateTime.now(), uuid: const Uuid().v4()),
      environmentInfo: DeviceInfo.instance.envInfo,
      results: exportResults,
      buildInfo: BuildInfoHelper.info,
    );
  }

  Future<void> runBenchmark(ResultHelper resultHelper, BenchmarkRunMode mode,
      String currentLogDir) async {
    final benchmark = resultHelper.benchmark;
    progressInfo.info = benchmark.info;
    progressInfo.currentStage++;

    if (mode == perfMode) {
      final perfTimer = Stopwatch()..start();
      progressInfo.accuracy = false;
      progressInfo.calculateStageProgress = () {
        // UI updates once per second so using 1 second as lower bound should not affect it.
        final minDuration = max(benchmark.taskConfig.minDuration, 1);
        final timeProgress = perfTimer.elapsedMilliseconds /
            Duration.millisecondsPerSecond /
            minDuration;

        final minQueries = max(benchmark.taskConfig.minQueryCount, 1);
        final queryCounter = backendBridge.getQueryCounter();
        final queryProgress =
            queryCounter < 0 ? 1.0 : queryCounter / minQueries;
        return min(timeProgress, queryProgress);
      };
      notifyListeners();

      final performanceRunInfo = await _NativeRunHelper(
        enableArtificialLoad: store.artificialCPULoadEnabled,
        isTestMode: store.testMode,
        resourceManager: resourceManager,
        backendBridge: backendBridge,
        benchmark: benchmark,
        runMode: perfMode,
        commonSettings: backendInfo.settings.commonSetting,
        backendLibName: backendInfo.libName,
        logParentDir: currentLogDir,
        testMinQueryCount: store.testMinQueryCount,
        testMinDuration: store.testMinDuration,
      ).run();
      perfTimer.stop();
      performanceRunInfo.loadgenInfo!;

      final performanceResult = performanceRunInfo.result;
      benchmark.performanceModeResult = BenchmarkResult(
        throughput: performanceRunInfo.throughput ?? 0.0,
        accuracy: performanceResult.accuracy1,
        accuracy2: performanceResult.accuracy2,
        backendName: performanceResult.backendName,
        acceleratorName: performanceResult.acceleratorName,
        batchSize: benchmark.benchmarkSettings.batchSize,
        validity: performanceRunInfo.loadgenInfo!.validity,
      );
      resultHelper.performanceRunInfo = performanceRunInfo;
    } else if (mode == accuracyMode) {
      progressInfo.accuracy = true;
      progressInfo.calculateStageProgress = () {
        final queryCounter = backendBridge.getQueryCounter();
        final queryProgress = queryCounter < 0
            ? 1.0
            : queryCounter / backendBridge.getDatasetSize();
        return queryProgress;
      };
      notifyListeners();
      final accuracyRunInfo = await _NativeRunHelper(
        enableArtificialLoad: store.artificialCPULoadEnabled,
        isTestMode: store.testMode,
        resourceManager: resourceManager,
        backendBridge: backendBridge,
        benchmark: benchmark,
        runMode: accuracyMode,
        commonSettings: backendInfo.settings.commonSetting,
        backendLibName: backendInfo.libName,
        logParentDir: currentLogDir,
        testMinQueryCount: store.testMinQueryCount,
        testMinDuration: store.testMinDuration,
      ).run();
      resultHelper.accuracyRunInfo = accuracyRunInfo;
      final accuracyResult = accuracyRunInfo.result;
      benchmark.accuracyModeResult = BenchmarkResult(
        // loadgen doesn't calculate latency for accuracy mode benchmarks
        // so throughput is infinity which is not a valid JSON numeric value
        throughput: 0.0,
        accuracy: accuracyResult.accuracy1,
        accuracy2: accuracyResult.accuracy2,
        backendName: accuracyResult.backendName,
        acceleratorName: accuracyResult.acceleratorName,
        batchSize: benchmark.benchmarkSettings.batchSize,
        validity: false,
      );
    } else {
      throw 'Unknown BenchmarkRunMode: $mode';
    }
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
    required bool isTestMode,
    required ResourceManager resourceManager,
    required List<pb.Setting> commonSettings,
    required String backendLibName,
    required String logParentDir,
    required int testMinQueryCount,
    required int testMinDuration,
  }) : logDir = '$logParentDir/${benchmark.id}-${runMode.readable}' {
    runSettings = benchmark.createRunSettings(
      runMode: runMode,
      resourceManager: resourceManager,
      commonSettings: commonSettings,
      backendLibName: backendLibName,
      logDir: logDir,
      testMinQueryCount: testMinQueryCount,
      testMinDuration: testMinDuration,
    );
  }

  Future<RunInfo> run() async {
    await Directory(logDir).create(recursive: true);

    if (enableArtificialLoad) {
      print('Apply the artificial CPU load for ${benchmark.taskConfig.id}');
      const value = 999999999999999.0;

      // execute() returns Cancelable, which should in theory allow you to stop the isolate
      // unfortunately, it doesn't work, artificial load doesn't stop
      final _ = Executor().execute(arg1: value, fun1: _doSomethingCPUIntensive);
    }

    try {
      return await _invokeNativeRun();
    } finally {
      await Executor().dispose();
    }
  }

  Future<RunInfo> _invokeNativeRun() async {
    final logPrefix = '${benchmark.id}: $runMode mode';

    print('$logPrefix: starting...');
    final stopwatch = Stopwatch()..start();
    final nativeResult = await backendBridge.run(runSettings);
    final elapsed = stopwatch.elapsed;
    print('$logPrefix: elapsed: $elapsed');
    print('$logPrefix: result: $nativeResult');

    if (!_checkAccuracy(nativeResult)) {
      throw '$logPrefix: accuracy is invalid (backend may be corrupted)';
    }

    final runInfo = await _makeRunInfo(nativeResult);
    print('$logPrefix: throughput: ${runInfo.throughput}');

    return runInfo;
  }

  Future<RunInfo> _makeRunInfo(NativeRunResult nativeResult) async {
    final loadgenInfo = await extractLoadgenInfo();
    final throughput = _calculateThroughput(nativeResult, loadgenInfo);

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

  // This function needs to be either a static function or a top level function to be accessible as a Flutter entry point.
  static void _doSomethingCPUIntensive(double value, TypeSendPort port) {
    var newValue = value;
    while (true) {
      newValue = newValue * 0.999999999999999;
    }
  }
}
