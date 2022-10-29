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

enum TaskRunnerState {
  ready,
  running,
  aborting,
}

class TaskRunner {
  static const loadgenLogFileName = 'mlperf_log_detail.txt';

  final Store store;
  final ResourceManager resourceManager;
  final BridgeIsolate backendBridge;
  final BackendInfo backendInfo;

  void Function()? notifyListeners;

  ProgressInfo progressInfo = ProgressInfo();
  TaskRunnerState state = TaskRunnerState.ready;
  Future<void> _cooldownFuture = Future.value();

  TaskRunner({
    required this.store,
    required this.resourceManager,
    required this.backendBridge,
    required this.backendInfo,
  });

  void setUpdateNotifier(void Function() callback) {
    notifyListeners = callback;
  }

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
    state = TaskRunnerState.aborting;
    await CancelableOperation.fromFuture(_cooldownFuture).cancel();
    notifyListeners?.call();
  }

  Future<ExtendedResult?> runBenchmarks(
      BenchmarkList middle, String currentLogDir) async {
    // TODO refactor this method

    state = TaskRunnerState.running;

    final cooldown = store.cooldown;
    final cooldownPause = store.testMode || isFastMode
        ? const Duration(seconds: 10)
        : Duration(minutes: store.cooldownDuration);

    final activeBenchmarks =
        middle.benchmarks.where((element) => element.isActive);

    final exportResults = <BenchmarkExportResult>[];
    var first = true;

    progressInfo.totalStages = activeBenchmarks.length;
    if (store.submissionMode) {
      progressInfo.totalStages += activeBenchmarks.length;
    }
    progressInfo.currentStage = 0;
    progressInfo.cooldownDuration = cooldownPause.inSeconds.toDouble();

    for (final benchmark in activeBenchmarks) {
      progressInfo.info = benchmark.info;

      // increment counter for performance benchmark before cooldown
      progressInfo.currentStage++;

      if (state == TaskRunnerState.aborting) break;

      // we only do cooldown before performance benchmarks
      if (cooldown && !first) {
        progressInfo.cooldown = true;
        final timer = Stopwatch()..start();
        progressInfo.calculateStageProgress = () {
          return timer.elapsed.inSeconds / progressInfo.cooldownDuration;
        };
        notifyListeners?.call();
        await (_cooldownFuture = Future.delayed(cooldownPause));
        progressInfo.cooldown = false;
        timer.stop();
      }
      first = false;
      if (state == TaskRunnerState.aborting) break;

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
      notifyListeners?.call();

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
      ).run();
      perfTimer.stop();
      performanceRunInfo.loadgenInfo!;

      if (state == TaskRunnerState.aborting) break;

      RunInfo? accuracyRunInfo;

      if (store.submissionMode) {
        progressInfo.currentStage++;
        progressInfo.accuracy = true;
        progressInfo.calculateStageProgress = () {
          final queryCounter = backendBridge.getQueryCounter();
          final queryProgress = queryCounter < 0
              ? 1.0
              : queryCounter / backendBridge.getDatasetSize();
          return queryProgress;
        };
        notifyListeners?.call();
        accuracyRunInfo = await _NativeRunHelper(
          enableArtificialLoad: store.artificialCPULoadEnabled,
          isTestMode: store.testMode,
          resourceManager: resourceManager,
          backendBridge: backendBridge,
          benchmark: benchmark,
          runMode: accuracyMode,
          commonSettings: backendInfo.settings.commonSetting,
          backendLibName: backendInfo.libName,
          logParentDir: currentLogDir,
        ).run();
      }

      final resultHelper = ResultHelper(
        benchmark: benchmark,
        backendInfo: backendInfo,
      );

      exportResults.add(resultHelper.exportResultFromRunInfo(
        performanceInfo: performanceRunInfo,
        accuracyInfo: accuracyRunInfo,
        perfMode: perfMode,
        accuracyMode: accuracyMode,
      ));
    }

    if (state == TaskRunnerState.aborting) {
      state = TaskRunnerState.ready;
      return null;
    }
    state = TaskRunnerState.ready;
    return ExtendedResult(
      meta: ResultMetaInfo(uuid: const Uuid().v4()),
      environmentInfo: DeviceInfo.instance.envInfo,
      results: exportResults,
      buildInfo: BuildInfoHelper.info,
    );
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
  }) : logDir = '$logParentDir/${benchmark.id}-${runMode.logSuffix}' {
    runSettings = benchmark.createRunSettings(
      runMode: runMode,
      resourceManager: resourceManager,
      commonSettings: commonSettings,
      backendLibName: backendLibName,
      logDir: logDir,
      isTestMode: isTestMode,
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
    return await LoadgenInfo.fromFile(
      filepath: '$logDir/${TaskRunner.loadgenLogFileName}',
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
