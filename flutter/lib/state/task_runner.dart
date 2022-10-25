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
  bool _aborting = false;
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
    _aborting = true;
    await CancelableOperation.fromFuture(_cooldownFuture).cancel();
    notifyListeners();
  }

  Future<ExtendedResult?> runBenchmarks(
      BenchmarkList middle, String currentLogDir) async {
    final cooldown = store.cooldown;
    final cooldownPause = store.testMode || isFastMode
        ? const Duration(seconds: 1)
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

      if (_aborting) break;

      // we only do cooldown before performance benchmarks
      if (cooldown && !first) {
        progressInfo.cooldown = true;
        final timer = Stopwatch()..start();
        progressInfo.calculateStageProgress = () {
          return timer.elapsed.inSeconds / progressInfo.cooldownDuration;
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
      final performanceRunInfo = await runBenchmark(
        benchmark,
        perfMode,
        backendInfo.settings.commonSetting,
        backendInfo.libName,
        currentLogDir,
      );
      perfTimer.stop();
      performanceRunInfo.loadgenInfo!;

      final performanceResult = performanceRunInfo.result;
      benchmark.performanceModeResult = BenchmarkResult(
        throughput: performanceRunInfo.throughput,
        accuracy: performanceResult.accuracy1,
        accuracy2: performanceResult.accuracy2,
        backendName: performanceResult.backendName,
        acceleratorName: performanceResult.acceleratorName,
        batchSize: benchmark.benchmarkSettings.batchSize,
        validity: performanceRunInfo.loadgenInfo!.validity,
      );

      if (_aborting) break;

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
        notifyListeners();
        accuracyRunInfo = await runBenchmark(
          benchmark,
          accuracyMode,
          backendInfo.settings.commonSetting,
          backendInfo.libName,
          currentLogDir,
        );

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

    if (_aborting) {
      _aborting = false;
      return null;
    }
    return ExtendedResult(
      meta: ResultMetaInfo(uuid: const Uuid().v4()),
      environmentInfo: DeviceInfo.instance.envInfo,
      results: exportResults,
      buildInfo: BuildInfoHelper.info,
    );
  }

  void _doSomethingCPUIntensive(double value, TypeSendPort port) {
    var newValue = value;
    while (true) {
      newValue = newValue * 0.999999999999999;
    }
  }

  Future<RunInfo> runBenchmark(
    Benchmark benchmark,
    BenchmarkRunMode runMode,
    List<pb.Setting> commonSettings,
    String backendLibName,
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
      backendLibName: backendLibName,
      logDir: logDir,
      isTestMode: store.testMode,
    );

    if (store.artificialCPULoadEnabled) {
      print('Apply the artificial CPU load for ${benchmark.taskConfig.id}');
      const value = 999999999999999.0;
      final _ = Executor().execute(arg1: value, fun1: _doSomethingCPUIntensive);
    }

    final result = await backendBridge.run(runSettings);
    final elapsed = stopwatch.elapsed;

    if (store.artificialCPULoadEnabled) {
      await Executor().dispose();
    }

    if (!(result.accuracy1?.isInBounds() ?? true) ||
        !(result.accuracy2?.isInBounds() ?? true)) {
      print(
          '${benchmark.id} accuracies: ${result.accuracy1}, ${result.accuracy2}');
      throw '${benchmark.info.taskName}: ${runMode.logSuffix} run: accuracy is invalid (backend may be corrupted)';
    }

    const logFileName = 'mlperf_log_detail.txt';
    final loadgenInfo = await LoadgenInfo.fromFile(
      filepath: '$logDir/$logFileName',
    );

    double throughput;
    if (loadgenInfo == null) {
      throughput = 0.0;
    } else if (benchmark.info.isOffline) {
      throughput = result.numSamples / loadgenInfo.latency90;
    } else {
      throughput = 1.0 / loadgenInfo.latency90;
    }

    print(
        'Run result: id: ${benchmark.id}, $result, throughput: $throughput, elapsed: $elapsed');

    return RunInfo(
      settings: runSettings,
      result: result,
      loadgenInfo: loadgenInfo,
      throughput: throughput,
    );
  }
}
