import 'package:mlperfbench/backend/bridge/run_result.dart';
import 'package:mlperfbench/backend/list.dart';
import 'package:mlperfbench/backend/loadgen_info.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/run_info.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';
import 'package:mlperfbench/data/results/backend_info.dart';
import 'package:mlperfbench/data/results/backend_settings.dart';
import 'package:mlperfbench/data/results/backend_settings_extra.dart';
import 'package:mlperfbench/data/results/benchmark_result.dart';
import 'package:mlperfbench/data/results/dataset_info.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;

class ResultHelper {
  final Benchmark benchmark;
  final BackendInfo backendInfo;
  RunInfo? performanceRunInfo;
  RunInfo? accuracyRunInfo;
  BenchmarkRunMode performanceMode;
  BenchmarkRunMode accuracyMode;

  ResultHelper({
    required this.benchmark,
    required this.backendInfo,
    required this.performanceMode,
    required this.accuracyMode,
  });

  BenchmarkExportResult getBenchmarkExportResult() {
    RunInfo runInfo;
    if (performanceRunInfo != null) {
      runInfo = performanceRunInfo!;
    } else if (accuracyRunInfo != null) {
      runInfo = accuracyRunInfo!;
    } else {
      throw 'One of performanceRunInfo or accuracyRunInfo must not be null';
    }
    final commonSettings = runInfo.settings.backend_settings.setting;
    return BenchmarkExportResult(
      benchmarkId: benchmark.id,
      benchmarkName: benchmark.taskConfig.name,
      performanceRun: _makeRunResult(performanceRunInfo, performanceMode),
      accuracyRun: _makeRunResult(accuracyRunInfo, accuracyMode),
      minDuration: benchmark.taskConfig.minDuration,
      minSamples: benchmark.taskConfig.minQueryCount,
      backendInfo: _makeBackendInfo(runInfo.result),
      backendSettings: _makeBackendSettingsInfo(benchmark, commonSettings),
      loadgenScenario: BenchmarkExportResult.parseLoadgenScenario(
          benchmark.taskConfig.scenario),
    );
  }

  BenchmarkRunResult? _makeRunResult(RunInfo? info, BenchmarkRunMode runMode) {
    if (info == null) {
      return null;
    }
    final result = info.result;
    final dataset = runMode.chooseDataset(benchmark.taskConfig);

    return BenchmarkRunResult(
      throughput: info.throughput,
      accuracy: result.accuracy1,
      accuracy2: result.accuracy2,
      dataset: DatasetInfo(
        name: dataset.name,
        type: DatasetInfo.parseDatasetType(
            benchmark.taskConfig.datasets.type.toString()),
        dataPath: dataset.inputPath,
        groundtruthPath: dataset.groundtruthPath,
      ),
      measuredDuration: result.duration,
      measuredSamples: result.numSamples,
      startDatetime: result.startTime,
      loadgenInfo: _makeLoadgenInfo(info.loadgenInfo),
    );
  }

  BenchmarkLoadgenInfo? _makeLoadgenInfo(LoadgenInfo? source) {
    if (source == null) {
      return null;
    }
    return BenchmarkLoadgenInfo(
      validity: source.validity,
      duration: source.meanLatency * source.queryCount,
    );
  }

  BackendReportedInfo _makeBackendInfo(NativeRunResult result) {
    return BackendReportedInfo(
      filename: backendInfo.libName,
      backendName: result.backendName,
      vendorName: result.backendVendor,
      acceleratorName: result.acceleratorName,
    );
  }

  BackendSettingsInfo _makeBackendSettingsInfo(
      Benchmark benchmark, List<pb.CommonSetting> commonSettings) {
    final delegate = benchmark.selectedDelegate;
    final extraSettings = _extraSettingsFromCommon(commonSettings) +
        _extraSettingsFromCustom(benchmark.selectedDelegate.customSetting);
    return BackendSettingsInfo(
      acceleratorCode: delegate.acceleratorName,
      acceleratorDesc: delegate.acceleratorDesc,
      delegate: delegate.delegateName,
      framework: benchmark.benchmarkSettings.framework,
      modelPath: delegate.modelPath,
      batchSize: delegate.batchSize,
      extraSettings: extraSettings,
    );
  }

  List<BackendExtraSetting> _extraSettingsFromCommon(
      List<pb.CommonSetting> commonSettings) {
    final list = <BackendExtraSetting>[];
    for (var item in commonSettings) {
      list.add(BackendExtraSetting(
          id: item.id,
          name: item.name,
          value: item.value.value,
          valueName: item.value.name));
    }
    return list;
  }

  List<BackendExtraSetting> _extraSettingsFromCustom(
      List<pb.CustomSetting> settings) {
    final list = <BackendExtraSetting>[];
    for (var item in settings) {
      list.add(BackendExtraSetting(
        id: item.id,
        name: '',
        value: item.value,
        valueName: '',
      ));
    }
    return list;
  }
}
