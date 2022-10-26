import 'package:mlperfbench_common/data/results/backend_info.dart';
import 'package:mlperfbench_common/data/results/backend_settings.dart';
import 'package:mlperfbench_common/data/results/backend_settings_extra.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';
import 'package:mlperfbench_common/data/results/dataset_info.dart';

import 'package:mlperfbench/backend/bridge/run_result.dart';
import 'package:mlperfbench/backend/list.dart';
import 'package:mlperfbench/backend/loadgen_info.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/run_info.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;

class ResultHelper {
  final Benchmark benchmark;
  final BackendInfo backendInfo;

  ResultHelper({
    required this.benchmark,
    required this.backendInfo,
  });

  BenchmarkExportResult exportResultFromRunInfo({
    required RunInfo performanceInfo,
    required RunInfo? accuracyInfo,
    required BenchmarkRunMode perfMode,
    required BenchmarkRunMode accuracyMode,
  }) {
    return BenchmarkExportResult(
      benchmarkId: benchmark.id,
      benchmarkName: benchmark.taskConfig.name,
      performanceRun: _makeRunResult(performanceInfo, perfMode),
      accuracyRun: _makeRunResult(accuracyInfo, accuracyMode),
      minDuration: benchmark.taskConfig.minDuration,
      minSamples: benchmark.taskConfig.minQueryCount,
      backendInfo: _makeBackendInfo(performanceInfo.result),
      backendSettings:
          _makeBackendSettingsInfo(performanceInfo.settings.backend_settings),
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

    double? throughput = info.throughput;
    if (!throughput.isFinite) {
      throughput = null;
    }

    return BenchmarkRunResult(
      throughput: throughput,
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

  BackendSettingsInfo _makeBackendSettingsInfo(pb.SettingList settings) {
    final taskSettings = settings.benchmarkSetting;

    return BackendSettingsInfo(
      acceleratorCode: taskSettings.accelerator,
      acceleratorDesc: taskSettings.acceleratorDesc,
      framework: taskSettings.framework,
      modelPath: taskSettings.modelPath,
      batchSize: taskSettings.batchSize,
      extraSettings: _extraSettingsFromCommon(settings.setting),
    );
  }

  List<BackendExtraSetting> _extraSettingsFromCommon(
      List<pb.Setting> commonSettings) {
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
}
